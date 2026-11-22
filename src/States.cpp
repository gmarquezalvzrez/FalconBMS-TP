#include "States.h"
#include "BmsSharedMemory.h"

#include <cmath>
#include <cstdio>

const char *const VAL_ON = "On";
const char *const VAL_OFF = "Off";

namespace {

constexpr double RAD2DEG = 57.29577951308232;
constexpr double FTSEC_TO_KT = 0.5924838;

std::string fInt(double v)
{
	return std::to_string(static_cast<long long>(std::llround(v)));
}

std::string fFix(double v, int decimals)
{
	char buf[48];
	std::snprintf(buf, sizeof(buf), "%.*f", decimals, v);
	return buf;
}

std::string onOff(bool b) { return b ? VAL_ON : VAL_OFF; }

std::string heading360(double deg)
{
	long long h = std::llround(std::fmod(deg, 360.0));
	if (h < 0) h += 360;
	if (h >= 360) h -= 360;
	return std::to_string(h);
}

// Convierte una linea del DED/PFL (char[26]) a UTF-8.
// SUPOSICION: BMS usa 0x01 para la flecha arriba/abajo y 0x02 para el simbolo de grado
// en su fuente del DED. El resto de caracteres de control se sustituye por espacios.
std::string dedLine(const char (&line)[26])
{
	std::string out;
	out.reserve(32);
	for (int i = 0; i < 25 && line[i]; ++i) {
		const unsigned char c = static_cast<unsigned char>(line[i]);
		if (c == 0x01)      out += "\xE2\x86\x95";  // flecha arriba/abajo
		else if (c == 0x02) out += "\xC2\xB0";      // simbolo de grado
		else if (c < 0x20 || c > 0x7E) out += ' ';
		else out += static_cast<char>(c);
	}
	// sin espacios al final, para no disparar actualizaciones por cambios invisibles
	while (!out.empty() && out.back() == ' ')
		out.pop_back();
	return out;
}

std::string cmdsMode(int m)
{
	switch (m) {
		case FlightData2::CmdsOFF:  return "OFF";
		case FlightData2::CmdsSTBY: return "STBY";
		case FlightData2::CmdsMAN:  return "MAN";
		case FlightData2::CmdsSEMI: return "SEMI";
		case FlightData2::CmdsAUTO: return "AUTO";
		case FlightData2::CmdsBYP:  return "BYP";
		default: return "";
	}
}

std::string navMode(int m)
{
	switch (m) {
		case FlightData2::ILS_TACAN: return "ILS/TCN";
		case FlightData2::TACAN:     return "TCN";
		case FlightData2::NAV:       return "NAV";
		case FlightData2::ILS_NAV:   return "ILS/NAV";
		default: return "";
	}
}


// Todos los bits de LightBits, LightBits2 y LightBits3 de FlightData.h, uno por estado (On/Off).
// Generado a partir de los enums de FlightData.h; los textos son los comentarios del propio fichero.
struct LightBitDef { int word; unsigned mask; const char *name; const char *comment; };
const LightBitDef LIGHT_BITS[] = {
		{ 1, static_cast<unsigned>(FlightData::MasterCaution), "MasterCaution", "Left eyebrow" },
		{ 1, static_cast<unsigned>(FlightData::TF), "TF", "Left eyebrow" },
		{ 1, static_cast<unsigned>(FlightData::OXY_BROW), "OXY_BROW", "repurposed for eyebrow OXY LOW (was OBS, unused)" },
		{ 1, static_cast<unsigned>(FlightData::EQUIP_HOT), "EQUIP_HOT", "Caution light; repurposed for cooling fault (was: not used)" },
		{ 1, static_cast<unsigned>(FlightData::ONGROUND), "ONGROUND", "True if on ground: this is not a lamp bit!" },
		{ 1, static_cast<unsigned>(FlightData::ENG_FIRE), "ENG_FIRE", "Right eyebrow; upper half of split face lamp" },
		{ 1, static_cast<unsigned>(FlightData::CONFIG), "CONFIG", "Stores config, caution panel" },
		{ 1, static_cast<unsigned>(FlightData::HYD), "HYD", "Right eyebrow; see also OIL (this lamp is not split face)" },
		{ 1, static_cast<unsigned>(FlightData::Flcs_ABCD), "Flcs_ABCD", "TEST panel FLCS channel lamps; repurposed, was OIL (see HYD; that lamp is not split face)" },
		{ 1, static_cast<unsigned>(FlightData::FLCS), "FLCS", "Right eyebrow; was called DUAL which matches block 25, 30/32 and older 40/42" },
		{ 1, static_cast<unsigned>(FlightData::CAN), "CAN", "Right eyebrow" },
		{ 1, static_cast<unsigned>(FlightData::T_L_CFG), "T_L_CFG", "Right eyebrow" },
		{ 1, static_cast<unsigned>(FlightData::AOAAbove), "AOAAbove", "AOA Indexers" },
		{ 1, static_cast<unsigned>(FlightData::AOAOn), "AOAOn", "AOA Indexers" },
		{ 1, static_cast<unsigned>(FlightData::AOABelow), "AOABelow", "AOA Indexers" },
		{ 1, static_cast<unsigned>(FlightData::RefuelRDY), "RefuelRDY", "Refuel/NWS" },
		{ 1, static_cast<unsigned>(FlightData::RefuelAR), "RefuelAR", "Refuel/NWS" },
		{ 1, static_cast<unsigned>(FlightData::RefuelDSC), "RefuelDSC", "Refuel/NWS" },
		{ 1, static_cast<unsigned>(FlightData::FltControlSys), "FltControlSys", "Caution Lights" },
		{ 1, static_cast<unsigned>(FlightData::LEFlaps), "LEFlaps", "Caution Lights" },
		{ 1, static_cast<unsigned>(FlightData::EngineFault), "EngineFault", "Caution Lights" },
		{ 1, static_cast<unsigned>(FlightData::Overheat), "Overheat", "Caution Lights" },
		{ 1, static_cast<unsigned>(FlightData::FuelLow), "FuelLow", "Caution Lights" },
		{ 1, static_cast<unsigned>(FlightData::Avionics), "Avionics", "Caution Lights" },
		{ 1, static_cast<unsigned>(FlightData::RadarAlt), "RadarAlt", "Caution Lights" },
		{ 1, static_cast<unsigned>(FlightData::IFF), "IFF", "Caution Lights" },
		{ 1, static_cast<unsigned>(FlightData::ECM), "ECM", "Caution Lights" },
		{ 1, static_cast<unsigned>(FlightData::Hook), "Hook", "Caution Lights" },
		{ 1, static_cast<unsigned>(FlightData::NWSFail), "NWSFail", "Caution Lights" },
		{ 1, static_cast<unsigned>(FlightData::CabinPress), "CabinPress", "Caution Lights" },
		{ 1, static_cast<unsigned>(FlightData::AutoPilotOn), "AutoPilotOn", "TRUE if is AP on. NB: This is not a lamp bit!" },
		{ 1, static_cast<unsigned>(FlightData::TFR_STBY), "TFR_STBY", "MISC panel; lower half of split face TFR lamp" },
		{ 2, static_cast<unsigned>(FlightData::HandOff), "HandOff", "Threat Warning Prime" },
		{ 2, static_cast<unsigned>(FlightData::Launch), "Launch", "Threat Warning Prime" },
		{ 2, static_cast<unsigned>(FlightData::PriMode), "PriMode", "Threat Warning Prime" },
		{ 2, static_cast<unsigned>(FlightData::Naval), "Naval", "Threat Warning Prime" },
		{ 2, static_cast<unsigned>(FlightData::Unk), "Unk", "Threat Warning Prime" },
		{ 2, static_cast<unsigned>(FlightData::TgtSep), "TgtSep", "Threat Warning Prime" },
		{ 2, static_cast<unsigned>(FlightData::Go), "Go", "On and operating normally" },
		{ 2, static_cast<unsigned>(FlightData::NoGo), "NoGo", "On but malfunction present" },
		{ 2, static_cast<unsigned>(FlightData::Degr), "Degr", "Status message: AUTO DEGR" },
		{ 2, static_cast<unsigned>(FlightData::Rdy), "Rdy", "Status message: DISPENSE RDY" },
		{ 2, static_cast<unsigned>(FlightData::ChaffLo), "ChaffLo", "Bingo chaff quantity reached" },
		{ 2, static_cast<unsigned>(FlightData::FlareLo), "FlareLo", "Bingo flare quantity reached" },
		{ 2, static_cast<unsigned>(FlightData::AuxSrch), "AuxSrch", "Aux Threat Warning" },
		{ 2, static_cast<unsigned>(FlightData::AuxAct), "AuxAct", "Aux Threat Warning" },
		{ 2, static_cast<unsigned>(FlightData::AuxLow), "AuxLow", "Aux Threat Warning" },
		{ 2, static_cast<unsigned>(FlightData::AuxPwr), "AuxPwr", "Aux Threat Warning" },
		{ 2, static_cast<unsigned>(FlightData::EcmPwr), "EcmPwr", "ECM" },
		{ 2, static_cast<unsigned>(FlightData::EcmFail), "EcmFail", "ECM" },
		{ 2, static_cast<unsigned>(FlightData::FwdFuelLow), "FwdFuelLow", "Caution Lights" },
		{ 2, static_cast<unsigned>(FlightData::AftFuelLow), "AftFuelLow", "Caution Lights" },
		{ 2, static_cast<unsigned>(FlightData::EPUOn), "EPUOn", "EPU panel; run light" },
		{ 2, static_cast<unsigned>(FlightData::JFSOn), "JFSOn", "Eng Jet Start panel; run light" },
		{ 2, static_cast<unsigned>(FlightData::SEC), "SEC", "Caution panel" },
		{ 2, static_cast<unsigned>(FlightData::OXY_LOW), "OXY_LOW", "Caution panel" },
		{ 2, static_cast<unsigned>(FlightData::PROBEHEAT), "PROBEHEAT", "Caution panel" },
		{ 2, static_cast<unsigned>(FlightData::SEAT_ARM), "SEAT_ARM", "Caution panel" },
		{ 2, static_cast<unsigned>(FlightData::BUC), "BUC", "Caution panel" },
		{ 2, static_cast<unsigned>(FlightData::FUEL_OIL_HOT), "FUEL_OIL_HOT", "Caution panel" },
		{ 2, static_cast<unsigned>(FlightData::ANTI_SKID), "ANTI_SKID", "Caution panel" },
		{ 2, static_cast<unsigned>(FlightData::TFR_ENGAGED), "TFR_ENGAGED", "MISC panel; upper half of split face TFR lamp" },
		{ 2, static_cast<unsigned>(FlightData::GEARHANDLE), "GEARHANDLE", "Lamp in gear handle lights on fault or gear in motion" },
		{ 2, static_cast<unsigned>(FlightData::ENGINE), "ENGINE", "Lower half of right eyebrow ENG FIRE/ENGINE lamp" },
		{ 3, static_cast<unsigned>(FlightData::FlcsPmg), "FlcsPmg", "Elec panel" },
		{ 3, static_cast<unsigned>(FlightData::MainGen), "MainGen", "Elec panel" },
		{ 3, static_cast<unsigned>(FlightData::StbyGen), "StbyGen", "Elec panel" },
		{ 3, static_cast<unsigned>(FlightData::EpuGen), "EpuGen", "Elec panel" },
		{ 3, static_cast<unsigned>(FlightData::EpuPmg), "EpuPmg", "Elec panel" },
		{ 3, static_cast<unsigned>(FlightData::ToFlcs), "ToFlcs", "Elec panel" },
		{ 3, static_cast<unsigned>(FlightData::FlcsRly), "FlcsRly", "Elec panel" },
		{ 3, static_cast<unsigned>(FlightData::BatFail), "BatFail", "Elec panel" },
		{ 3, static_cast<unsigned>(FlightData::Hydrazine), "Hydrazine", "EPU panel" },
		{ 3, static_cast<unsigned>(FlightData::Air), "Air", "EPU panel" },
		{ 3, static_cast<unsigned>(FlightData::Elec_Fault), "Elec_Fault", "Caution panel" },
		{ 3, static_cast<unsigned>(FlightData::Lef_Fault), "Lef_Fault", "Caution panel" },
		{ 3, static_cast<unsigned>(FlightData::OnGround), "OnGround", "weight-on-wheels" },
		{ 3, static_cast<unsigned>(FlightData::FlcsBitRun), "FlcsBitRun", "FLT CONTROL panel RUN light (used to be Multi-engine fire light)" },
		{ 3, static_cast<unsigned>(FlightData::FlcsBitFail), "FlcsBitFail", "FLT CONTROL panel FAIL light (used to be Lock light Cue; non-F-16)" },
		{ 3, static_cast<unsigned>(FlightData::DbuWarn), "DbuWarn", "Right eyebrow DBU ON cell; was Shoot light cue; non-F16" },
		{ 3, static_cast<unsigned>(FlightData::NoseGearDown), "NoseGearDown", "Landing gear panel; on means down and locked" },
		{ 3, static_cast<unsigned>(FlightData::LeftGearDown), "LeftGearDown", "Landing gear panel; on means down and locked" },
		{ 3, static_cast<unsigned>(FlightData::RightGearDown), "RightGearDown", "Landing gear panel; on means down and locked" },
		{ 3, static_cast<unsigned>(FlightData::ParkBrakeOn), "ParkBrakeOn", "Parking brake engaged; NOTE: not a lamp bit" },
		{ 3, static_cast<unsigned>(FlightData::Power_Off), "Power_Off", "Set if there is no electrical power. NB: not a lamp bit" },
		{ 3, static_cast<unsigned>(FlightData::cadc), "cadc", "Caution panel" },
		{ 3, static_cast<unsigned>(FlightData::SpeedBrake), "SpeedBrake", "True if speed brake is in anything other than stowed position" },
		{ 3, static_cast<unsigned>(FlightData::SysTest), "SysTest", "Threat Warning Prime: SYS TEST" },
		{ 3, static_cast<unsigned>(FlightData::MCAnnounced), "MCAnnounced", "Master Caution will come up (3 s delay, like RL)" },
		{ 3, static_cast<unsigned>(FlightData::MLGWOW), "MLGWOW", "AFM only: WOW switches on main gear triggered" },
		{ 3, static_cast<unsigned>(FlightData::NLGWOW), "NLGWOW", "AFM only: WOW switch on nose gear triggered" },
		{ 3, static_cast<unsigned>(FlightData::ATF_Not_Engaged), "ATF_Not_Engaged", "ATF not engaged" },
		{ 3, static_cast<unsigned>(FlightData::Inlet_Icing), "Inlet_Icing", "Caution panel" },
};

using S = const BmsSnapshot &;
const std::vector<std::string> ONOFF { VAL_ON, VAL_OFF };

#define TXT(ID, GRP, DESC, EXPR) \
	StateDef { ID, GRP, DESC, "", {}, [](S s) -> std::string { (void)s; return EXPR; } }
#define BOOL(ID, GRP, DESC, EXPR) \
	StateDef { ID, GRP, DESC, VAL_OFF, ONOFF, [](S s) -> std::string { (void)s; return onOff(EXPR); } }

#define LB(bit)  ((s.fd.lightBits  & FlightData::bit) != 0)
#define LB2(bit) ((s.fd.lightBits2 & FlightData::bit) != 0)
#define LB3(bit) ((s.fd.lightBits3 & FlightData::bit) != 0)

const char *G_SIM    = "Simulador";
const char *G_FLIGHT = "Vuelo";
const char *G_ENGINE = "Motor";
const char *G_FUEL   = "Combustible";
const char *G_GEAR   = "Tren y frenos";
const char *G_LIGHTS = "Luces de aviso";
const char *G_CM     = "Contramedidas";
const char *G_NAV    = "Radio y navegacion";
const char *G_DED    = "DED y PFL";

std::vector<StateDef> buildDefinitions()
{
	std::vector<StateDef> v {
		// --- Simulador -------------------------------------------------------------------
		BOOL("sim.running", G_SIM, "BMS: en ejecucion", s.bmsRunning()),
		TXT("sim.diag",     G_SIM, "Plugin: diagnostico de la memoria compartida", s.diag),
		BOOL("sim.flying",  G_SIM, "BMS: piloto en un avion (no en la interfaz)", s.flying()),
		BOOL("sim.in3d",    G_SIM, "BMS: en 3D", s.ivOk && s.iv.In3D),
		BOOL("sim.paused",  G_SIM, "BMS: en pausa", s.ivOk && s.iv.IsPaused),
		BOOL("sim.onGround",G_SIM, "BMS: en tierra", LB(ONGROUND)),
		TXT("sim.version",  G_SIM, "BMS: version",
			s.fd2.BMSVersionMajor > 0
				? std::to_string(s.fd2.BMSVersionMajor) + "." + std::to_string(s.fd2.BMSVersionMinor) + "." +
				  std::to_string(s.fd2.BMSVersionMicro) + " build " + std::to_string(s.fd2.BMSBuildNumber)
				: std::string()),
		TXT("sim.time", G_SIM, "BMS: hora en el simulador (HH:MM:SS)", [&s] {
			const int t = s.fd2.currentTime;
			char b[16];
			std::snprintf(b, sizeof(b), "%02d:%02d:%02d", (t / 3600) % 24, (t / 60) % 60, t % 60);
			return std::string(b);
		}()),

		// --- Vuelo -------------------------------------------------------------------------
		TXT("flight.kias",     G_FLIGHT, "Velocidad indicada (kt)", fInt(s.fd.kias)),
		TXT("flight.tas",      G_FLIGHT, "Velocidad verdadera (kt)", fInt(s.fd.vt * FTSEC_TO_KT)),
		TXT("flight.mach",     G_FLIGHT, "Mach", fFix(s.fd.mach, 2)),
		// AAUZ sigue el convenio de z (positivo hacia abajo); comprobado con BMS 4.38.2 (11512 ft baro / 11040 ft real).
		TXT("flight.altitude", G_FLIGHT, "Altitud barometrica (ft)", fInt(-s.fd2.AAUZ)),
		TXT("flight.altitudeTrue", G_FLIGHT, "Altitud real (ft)", fInt(-s.fd.z)),
		// RALT sigue el convenio de z (negativo = por encima del suelo), comprobado con BMS 4.38.2
		TXT("flight.ralt",     G_FLIGHT, "Radioaltimetro (ft, vacio si no es valido)",
			(s.fd2.miscBits & FlightData2::RALT_Valid) ? fInt(std::fabs(s.fd2.RALT)) : std::string()),
		TXT("flight.vvi",      G_FLIGHT, "Velocidad vertical (ft/min)", fInt(-s.fd.zDot * 60.0)),
		TXT("flight.heading",  G_FLIGHT, "Rumbo verdadero (grados)", heading360(s.fd.yaw * RAD2DEG)),
		TXT("flight.hsiHeading", G_FLIGHT, "Rumbo en el HSI (grados)", heading360(s.fd.currentHeading)),
		TXT("flight.pitch",    G_FLIGHT, "Cabeceo (grados)", fFix(s.fd.pitch * RAD2DEG, 1)),
		TXT("flight.roll",     G_FLIGHT, "Alabeo (grados)", fFix(s.fd.roll * RAD2DEG, 1)),
		TXT("flight.aoa",      G_FLIGHT, "Angulo de ataque (grados)", fFix(s.fd.alpha, 1)),
		TXT("flight.g",        G_FLIGHT, "Factor de carga (G)", fFix(s.fd.gs, 1)),
		TXT("flight.gMax",     G_FLIGHT, "Factor de carga maximo (G)", fFix(s.fd2.gsMax, 1)),
		TXT("flight.latitude", G_FLIGHT, "Latitud (grados)", fFix(s.fd2.latitude, 5)),
		TXT("flight.longitude",G_FLIGHT, "Longitud (grados)", fFix(s.fd2.longitude, 5)),

		// --- Motor -------------------------------------------------------------------------
		TXT("engine.rpm",      G_ENGINE, "RPM (%)", fInt(s.fd.rpm)),
		// FTIT viene en centenas de grados C (7.88 = 788 C); comprobado con BMS 4.38.2 a 98% RPM.
		TXT("engine.ftit",     G_ENGINE, "FTIT (grados C)", fInt(s.fd.ftit * 100.0)),
		TXT("engine.nozzle",   G_ENGINE, "Tobera (% abierta)", fInt(s.fd.nozzlePos)),
		TXT("engine.oil",      G_ENGINE, "Presion de aceite (%)", fInt(s.fd.oilPressure)),
		TXT("engine.fuelFlow", G_ENGINE, "Flujo de combustible (lb/h)", fInt(s.fd.fuelFlow)),
		TXT("engine.epuFuel",  G_ENGINE, "Combustible EPU (%)", fInt(s.fd.epuFuel)),
		TXT("engine.hydA",     G_ENGINE, "Presion hidraulica A", fInt(s.fd2.hydPressureA)),
		TXT("engine.hydB",     G_ENGINE, "Presion hidraulica B", fInt(s.fd2.hydPressureB)),

		// --- Combustible ---------------------------------------------------------------------
		TXT("fuel.internal", G_FUEL, "Combustible interno (lb)", fInt(s.fd.internalFuel)),
		TXT("fuel.external", G_FUEL, "Combustible externo (lb)", fInt(s.fd.externalFuel)),
		TXT("fuel.total",    G_FUEL, "Combustible total (lb)", fInt(s.fd.total)),
		// fwd/aft vienen en decimas de libra (19858 + 15404 = 3526 lb con 3516 lb internos, BMS 4.38.2)
		TXT("fuel.fwd",      G_FUEL, "Combustible deposito delantero (lb)", fInt(s.fd.fwd / 10.0)),
		TXT("fuel.aft",      G_FUEL, "Combustible deposito trasero (lb)", fInt(s.fd.aft / 10.0)),
		TXT("fuel.bingo",    G_FUEL, "Bingo (lb)", fInt(s.fd2.bingoFuel)),

		// --- Tren y frenos -------------------------------------------------------------------
		TXT("gear.position",   G_GEAR, "Tren: posicion (%; 0 arriba, 100 abajo)", fInt(s.fd.gearPos * 100.0)),
		BOOL("gear.noseDown",  G_GEAR, "Tren delantero abajo y blocado", LB3(NoseGearDown)),
		BOOL("gear.leftDown",  G_GEAR, "Tren izquierdo abajo y blocado", LB3(LeftGearDown)),
		BOOL("gear.rightDown", G_GEAR, "Tren derecho abajo y blocado", LB3(RightGearDown)),
		BOOL("gear.handleLight", G_GEAR, "Luz de la palanca del tren", LB2(GEARHANDLE)),
		BOOL("gear.parkBrake", G_GEAR, "Freno de aparcamiento", LB3(ParkBrakeOn)),
		TXT("gear.speedBrake", G_GEAR, "Aerofreno (% abierto)", fInt(s.fd.speedBrake * 100.0)),

		// --- Luces de aviso ------------------------------------------------------------------
		BOOL("lights.masterCaution", G_LIGHTS, "MASTER CAUTION", LB(MasterCaution)),
		BOOL("lights.engFire",  G_LIGHTS, "ENG FIRE", LB(ENG_FIRE)),
		BOOL("lights.engine",   G_LIGHTS, "ENGINE", LB2(ENGINE)),
		BOOL("lights.hyd",      G_LIGHTS, "HYD/OIL PRESS", LB(HYD)),
		BOOL("lights.flcs",     G_LIGHTS, "FLCS", LB(FLCS)),
		BOOL("lights.dbu",      G_LIGHTS, "DBU ON", LB3(DbuWarn)),
		BOOL("lights.tlCfg",    G_LIGHTS, "TO/LDG CONFIG", LB(T_L_CFG)),
		BOOL("lights.can",      G_LIGHTS, "CANOPY", LB(CAN)),
		BOOL("lights.oxyLow",   G_LIGHTS, "OXY LOW", LB(OXY_BROW)),
		BOOL("lights.tf",       G_LIGHTS, "TF FAIL", LB(TF)),
		BOOL("lights.fuelLow",  G_LIGHTS, "FUEL LOW (panel de precaucion)", LB(FuelLow)),
		BOOL("lights.autopilot",G_LIGHTS, "Piloto automatico conectado", LB(AutoPilotOn)),
		BOOL("lights.jfsRun",   G_LIGHTS, "JFS RUN", LB2(JFSOn)),
		BOOL("lights.epuRun",   G_LIGHTS, "EPU RUN", LB2(EPUOn)),
		BOOL("lights.mainGen",  G_LIGHTS, "MAIN GEN", LB3(MainGen)),
		BOOL("lights.stbyGen",  G_LIGHTS, "STBY GEN", LB3(StbyGen)),
		BOOL("lights.aoaAbove", G_LIGHTS, "Indexador AOA: alto", LB(AOAAbove)),
		BOOL("lights.aoaOn",    G_LIGHTS, "Indexador AOA: en angulo", LB(AOAOn)),
		BOOL("lights.aoaBelow", G_LIGHTS, "Indexador AOA: bajo", LB(AOABelow)),
		BOOL("lights.refuelRdy",G_LIGHTS, "Reabastecimiento: RDY", LB(RefuelRDY)),
		BOOL("lights.refuelAr", G_LIGHTS, "Reabastecimiento: AR/NWS", LB(RefuelAR)),
		BOOL("lights.refuelDsc",G_LIGHTS, "Reabastecimiento: DISC", LB(RefuelDSC)),

		// --- Contramedidas -------------------------------------------------------------------
		TXT("cm.chaff", G_CM, "Chaff restante", fInt(s.fd.ChaffCount)),
		TXT("cm.flare", G_CM, "Bengalas restantes", fInt(s.fd.FlareCount)),
		TXT("cm.mode",  G_CM, "Modo CMDS", cmdsMode(s.fd2.cmdsMode)),
		BOOL("cm.chaffLow", G_CM, "Chaff bajo (bingo)", LB2(ChaffLo)),
		BOOL("cm.flareLow", G_CM, "Bengalas bajas (bingo)", LB2(FlareLo)),

		// --- Radio y navegacion --------------------------------------------------------------
		TXT("radio.uhfPreset", G_NAV, "UHF: canal", std::to_string(s.fd2.uhf_panel_preset)),
		// Viene en kHz (225000 = 225.000 MHz, BMS 4.38.2)
		TXT("radio.uhfFreq",   G_NAV, "UHF: frecuencia del canal (MHz)", fFix(s.fd2.uhf_panel_frequency / 1000.0, 3)),
		TXT("nav.tacan",       G_NAV, "TACAN (canal y banda, UFC)",
			std::to_string(s.fd.UFCTChan) + (s.fd2.UfcTacanIsX() ? "X" : "Y") + (s.fd2.UfcTacanIsAA() ? " A/A" : "")),
		TXT("nav.mode",        G_NAV, "Modo HSI", navMode(s.fd2.navMode)),
		TXT("nav.distance",    G_NAV, "Distancia a la baliza (nm)", fFix(s.fd.distanceToBeacon, 1)),
		TXT("nav.bearing",     G_NAV, "Marcacion a la baliza (grados)", heading360(s.fd.bearingToBeacon)),
		TXT("nav.course",      G_NAV, "Curso seleccionado (grados)", heading360(s.fd.desiredCourse)),
	};

	// --- LightBits completos ---------------------------------------------------------------------
	for (const LightBitDef &b : LIGHT_BITS) {
		const std::string word = b.word == 1 ? "" : std::to_string(b.word);
		const unsigned mask = b.mask;
		const int w = b.word;
		v.push_back(StateDef { "lightBits" + word + "." + b.name, "LightBits" + word,
			b.name, VAL_OFF, ONOFF,
			[w, mask](S s) {
				const unsigned bits = w == 1 ? s.fd.lightBits : (w == 2 ? s.fd.lightBits2 : s.fd.lightBits3);
				return onOff((bits & mask) != 0);
			} });
	}

	// --- DED y PFL ---------------------------------------------------------------------------
	for (int i = 0; i < 5; ++i) {
		const std::string n = std::to_string(i + 1);
		v.push_back(StateDef { "ded.line" + n, G_DED, "DED linea " + n, "", {},
			[i](S s) { return dedLine(s.fd.DEDLines[i]); } });
	}
	for (int i = 0; i < 5; ++i) {
		const std::string n = std::to_string(i + 1);
		v.push_back(StateDef { "pfl.line" + n, G_DED, "PFL linea " + n, "", {},
			[i](S s) { return dedLine(s.fd.PFLLines[i]); } });
	}
	return v;
}

}  // namespace

const std::vector<StateDef> &stateDefinitions()
{
	static const std::vector<StateDef> defs = buildDefinitions();
	return defs;
}
