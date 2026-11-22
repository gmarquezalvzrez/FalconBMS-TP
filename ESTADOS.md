# Estados del plugin Falcon BMS

Lista completa de los estados que publica el plugin y los valores que pueden tomar. Generada desde `entry.tp`.

- Los estados **On/Off** solo valen exactamente `On` u `Off` (con esa mayúscula). Son los que sirven para "When plug-in state changes".
- Los estados de **texto** son números o texto; los números usan punto decimal.
- Con BMS cerrado solo cambian los estados del grupo *Simulador*; el resto conserva su último valor.
- En los botones, el valor se inserta como `${value:ID}`, p. ej. `${value:FalconBMS_TP.state.flight.kias}`.


## Simulador

| Nombre en Touch Portal | ID | Valores |
|---|---|---|
| BMS: en ejecucion | `FalconBMS_TP.state.sim.running` | `On` / `Off` |
| Plugin: diagnostico de la memoria compartida | `FalconBMS_TP.state.sim.diag` | Texto. Por zona: `abierta`, `cerrada` o `error N` (2 = BMS no la ha creado, 5 = acceso denegado), y si se ve el proceso `Falcon BMS.exe` |
| BMS: piloto en un avion (no en la interfaz) | `FalconBMS_TP.state.sim.flying` | `On` / `Off` |
| BMS: en 3D | `FalconBMS_TP.state.sim.in3d` | `On` / `Off` |
| BMS: en pausa | `FalconBMS_TP.state.sim.paused` | `On` / `Off` |
| BMS: en tierra | `FalconBMS_TP.state.sim.onGround` | `On` / `Off` |
| BMS: version | `FalconBMS_TP.state.sim.version` | Texto, p. ej. `4.37.6 build 12345`. Vacío si BMS no lo informa |
| BMS: hora en el simulador (HH:MM:SS) | `FalconBMS_TP.state.sim.time` | `HH:MM:SS` (hora del mundo de BMS) |

## Vuelo

| Nombre en Touch Portal | ID | Valores |
|---|---|---|
| Velocidad indicada (kt) | `FalconBMS_TP.state.flight.kias` | Entero, nudos (p. ej. `352`) |
| Velocidad verdadera (kt) | `FalconBMS_TP.state.flight.tas` | Entero, nudos |
| Mach | `FalconBMS_TP.state.flight.mach` | Decimal con 2 cifras (p. ej. `0.87`) |
| Altitud barometrica (ft) | `FalconBMS_TP.state.flight.altitude` | Entero, pies (puede ser negativo) |
| Altitud real (ft) | `FalconBMS_TP.state.flight.altitudeTrue` | Entero, pies |
| Radioaltimetro (ft, vacio si no es valido) | `FalconBMS_TP.state.flight.ralt` | Entero, pies; vacío si el radioaltímetro no es válido |
| Velocidad vertical (ft/min) | `FalconBMS_TP.state.flight.vvi` | Entero, pies/min; positivo subiendo, negativo bajando |
| Rumbo verdadero (grados) | `FalconBMS_TP.state.flight.heading` | Entero 0–359 |
| Rumbo en el HSI (grados) | `FalconBMS_TP.state.flight.hsiHeading` | Entero 0–359 |
| Cabeceo (grados) | `FalconBMS_TP.state.flight.pitch` | Decimal con 1 cifra, grados; positivo morro arriba |
| Alabeo (grados) | `FalconBMS_TP.state.flight.roll` | Decimal con 1 cifra, grados; positivo alabeo a la derecha |
| Angulo de ataque (grados) | `FalconBMS_TP.state.flight.aoa` | Decimal con 1 cifra, grados |
| Factor de carga (G) | `FalconBMS_TP.state.flight.g` | Decimal con 1 cifra (p. ej. `1.0`, `-2.3`) |
| Factor de carga maximo (G) | `FalconBMS_TP.state.flight.gMax` | Decimal con 1 cifra |
| Latitud (grados) | `FalconBMS_TP.state.flight.latitude` | Decimal con 5 cifras, grados (negativo = sur) |
| Longitud (grados) | `FalconBMS_TP.state.flight.longitude` | Decimal con 5 cifras, grados (negativo = oeste) |

## Motor

| Nombre en Touch Portal | ID | Valores |
|---|---|---|
| RPM (%) | `FalconBMS_TP.state.engine.rpm` | Entero 0–103 (%) |
| FTIT (grados C) | `FalconBMS_TP.state.engine.ftit` | Entero, °C |
| Tobera (% abierta) | `FalconBMS_TP.state.engine.nozzle` | Entero 0–100 (%) |
| Presion de aceite (%) | `FalconBMS_TP.state.engine.oil` | Entero 0–100 (%) |
| Flujo de combustible (lb/h) | `FalconBMS_TP.state.engine.fuelFlow` | Entero, lb/h |
| Combustible EPU (%) | `FalconBMS_TP.state.engine.epuFuel` | Entero 0–100 (%) |
| Presion hidraulica A | `FalconBMS_TP.state.engine.hydA` | Entero, psi |
| Presion hidraulica B | `FalconBMS_TP.state.engine.hydB` | Entero, psi |

## Combustible

| Nombre en Touch Portal | ID | Valores |
|---|---|---|
| Combustible interno (lb) | `FalconBMS_TP.state.fuel.internal` | Entero, libras |
| Combustible externo (lb) | `FalconBMS_TP.state.fuel.external` | Entero, libras |
| Combustible total (lb) | `FalconBMS_TP.state.fuel.total` | Entero, libras |
| Combustible deposito delantero (lb) | `FalconBMS_TP.state.fuel.fwd` | Entero, libras |
| Combustible deposito trasero (lb) | `FalconBMS_TP.state.fuel.aft` | Entero, libras |
| Bingo (lb) | `FalconBMS_TP.state.fuel.bingo` | Entero, libras |

## Tren y frenos

| Nombre en Touch Portal | ID | Valores |
|---|---|---|
| Tren: posicion (%; 0 arriba, 100 abajo) | `FalconBMS_TP.state.gear.position` | Entero 0–100 (0 arriba, 100 abajo) |
| Tren delantero abajo y blocado | `FalconBMS_TP.state.gear.noseDown` | `On` / `Off` |
| Tren izquierdo abajo y blocado | `FalconBMS_TP.state.gear.leftDown` | `On` / `Off` |
| Tren derecho abajo y blocado | `FalconBMS_TP.state.gear.rightDown` | `On` / `Off` |
| Luz de la palanca del tren | `FalconBMS_TP.state.gear.handleLight` | `On` / `Off` |
| Freno de aparcamiento | `FalconBMS_TP.state.gear.parkBrake` | `On` / `Off` |
| Aerofreno (% abierto) | `FalconBMS_TP.state.gear.speedBrake` | Entero 0–100 (%) |

## Luces de aviso

| Nombre en Touch Portal | ID | Valores |
|---|---|---|
| MASTER CAUTION | `FalconBMS_TP.state.lights.masterCaution` | `On` / `Off` |
| ENG FIRE | `FalconBMS_TP.state.lights.engFire` | `On` / `Off` |
| ENGINE | `FalconBMS_TP.state.lights.engine` | `On` / `Off` |
| HYD/OIL PRESS | `FalconBMS_TP.state.lights.hyd` | `On` / `Off` |
| FLCS | `FalconBMS_TP.state.lights.flcs` | `On` / `Off` |
| DBU ON | `FalconBMS_TP.state.lights.dbu` | `On` / `Off` |
| TO/LDG CONFIG | `FalconBMS_TP.state.lights.tlCfg` | `On` / `Off` |
| CANOPY | `FalconBMS_TP.state.lights.can` | `On` / `Off` |
| OXY LOW | `FalconBMS_TP.state.lights.oxyLow` | `On` / `Off` |
| TF FAIL | `FalconBMS_TP.state.lights.tf` | `On` / `Off` |
| FUEL LOW (panel de precaucion) | `FalconBMS_TP.state.lights.fuelLow` | `On` / `Off` |
| Piloto automatico conectado | `FalconBMS_TP.state.lights.autopilot` | `On` / `Off` |
| JFS RUN | `FalconBMS_TP.state.lights.jfsRun` | `On` / `Off` |
| EPU RUN | `FalconBMS_TP.state.lights.epuRun` | `On` / `Off` |
| MAIN GEN | `FalconBMS_TP.state.lights.mainGen` | `On` / `Off` |
| STBY GEN | `FalconBMS_TP.state.lights.stbyGen` | `On` / `Off` |
| Indexador AOA: alto | `FalconBMS_TP.state.lights.aoaAbove` | `On` / `Off` |
| Indexador AOA: en angulo | `FalconBMS_TP.state.lights.aoaOn` | `On` / `Off` |
| Indexador AOA: bajo | `FalconBMS_TP.state.lights.aoaBelow` | `On` / `Off` |
| Reabastecimiento: RDY | `FalconBMS_TP.state.lights.refuelRdy` | `On` / `Off` |
| Reabastecimiento: AR/NWS | `FalconBMS_TP.state.lights.refuelAr` | `On` / `Off` |
| Reabastecimiento: DISC | `FalconBMS_TP.state.lights.refuelDsc` | `On` / `Off` |

## Contramedidas

| Nombre en Touch Portal | ID | Valores |
|---|---|---|
| Chaff restante | `FalconBMS_TP.state.cm.chaff` | Entero |
| Bengalas restantes | `FalconBMS_TP.state.cm.flare` | Entero |
| Modo CMDS | `FalconBMS_TP.state.cm.mode` | `OFF`, `STBY`, `MAN`, `SEMI`, `AUTO`, `BYP` |
| Chaff bajo (bingo) | `FalconBMS_TP.state.cm.chaffLow` | `On` / `Off` |
| Bengalas bajas (bingo) | `FalconBMS_TP.state.cm.flareLow` | `On` / `Off` |

## Radio y navegacion

| Nombre en Touch Portal | ID | Valores |
|---|---|---|
| UHF: canal | `FalconBMS_TP.state.radio.uhfPreset` | Entero (número de canal) |
| UHF: frecuencia del canal (MHz) | `FalconBMS_TP.state.radio.uhfFreq` | Decimal con 3 cifras, p. ej. `225.000` |
| TACAN (canal y banda, UFC) | `FalconBMS_TP.state.nav.tacan` | Canal + banda, p. ej. `75X`, `106Y`; añade ` A/A` en modo aire-aire |
| Modo HSI | `FalconBMS_TP.state.nav.mode` | `ILS/TCN`, `TCN`, `NAV`, `ILS/NAV` |
| Distancia a la baliza (nm) | `FalconBMS_TP.state.nav.distance` | Decimal con 1 cifra, millas náuticas |
| Marcacion a la baliza (grados) | `FalconBMS_TP.state.nav.bearing` | Entero 0–359 |
| Curso seleccionado (grados) | `FalconBMS_TP.state.nav.course` | Entero 0–359 |

## LightBits

Todos los bits del enum `LightBits` de `FlightData.h` (el nombre es el del enum).

| Nombre en Touch Portal | ID | Valores |
|---|---|---|
| MasterCaution | `FalconBMS_TP.state.lightBits.MasterCaution` | `On` / `Off` |
| TF | `FalconBMS_TP.state.lightBits.TF` | `On` / `Off` |
| OXY_BROW | `FalconBMS_TP.state.lightBits.OXY_BROW` | `On` / `Off` |
| EQUIP_HOT | `FalconBMS_TP.state.lightBits.EQUIP_HOT` | `On` / `Off` |
| ONGROUND | `FalconBMS_TP.state.lightBits.ONGROUND` | `On` / `Off` |
| ENG_FIRE | `FalconBMS_TP.state.lightBits.ENG_FIRE` | `On` / `Off` |
| CONFIG | `FalconBMS_TP.state.lightBits.CONFIG` | `On` / `Off` |
| HYD | `FalconBMS_TP.state.lightBits.HYD` | `On` / `Off` |
| Flcs_ABCD | `FalconBMS_TP.state.lightBits.Flcs_ABCD` | `On` / `Off` |
| FLCS | `FalconBMS_TP.state.lightBits.FLCS` | `On` / `Off` |
| CAN | `FalconBMS_TP.state.lightBits.CAN` | `On` / `Off` |
| T_L_CFG | `FalconBMS_TP.state.lightBits.T_L_CFG` | `On` / `Off` |
| AOAAbove | `FalconBMS_TP.state.lightBits.AOAAbove` | `On` / `Off` |
| AOAOn | `FalconBMS_TP.state.lightBits.AOAOn` | `On` / `Off` |
| AOABelow | `FalconBMS_TP.state.lightBits.AOABelow` | `On` / `Off` |
| RefuelRDY | `FalconBMS_TP.state.lightBits.RefuelRDY` | `On` / `Off` |
| RefuelAR | `FalconBMS_TP.state.lightBits.RefuelAR` | `On` / `Off` |
| RefuelDSC | `FalconBMS_TP.state.lightBits.RefuelDSC` | `On` / `Off` |
| FltControlSys | `FalconBMS_TP.state.lightBits.FltControlSys` | `On` / `Off` |
| LEFlaps | `FalconBMS_TP.state.lightBits.LEFlaps` | `On` / `Off` |
| EngineFault | `FalconBMS_TP.state.lightBits.EngineFault` | `On` / `Off` |
| Overheat | `FalconBMS_TP.state.lightBits.Overheat` | `On` / `Off` |
| FuelLow | `FalconBMS_TP.state.lightBits.FuelLow` | `On` / `Off` |
| Avionics | `FalconBMS_TP.state.lightBits.Avionics` | `On` / `Off` |
| RadarAlt | `FalconBMS_TP.state.lightBits.RadarAlt` | `On` / `Off` |
| IFF | `FalconBMS_TP.state.lightBits.IFF` | `On` / `Off` |
| ECM | `FalconBMS_TP.state.lightBits.ECM` | `On` / `Off` |
| Hook | `FalconBMS_TP.state.lightBits.Hook` | `On` / `Off` |
| NWSFail | `FalconBMS_TP.state.lightBits.NWSFail` | `On` / `Off` |
| CabinPress | `FalconBMS_TP.state.lightBits.CabinPress` | `On` / `Off` |
| AutoPilotOn | `FalconBMS_TP.state.lightBits.AutoPilotOn` | `On` / `Off` |
| TFR_STBY | `FalconBMS_TP.state.lightBits.TFR_STBY` | `On` / `Off` |

## LightBits2

Todos los bits del enum `LightBits2` de `FlightData.h` (el nombre es el del enum).

| Nombre en Touch Portal | ID | Valores |
|---|---|---|
| HandOff | `FalconBMS_TP.state.lightBits2.HandOff` | `On` / `Off` |
| Launch | `FalconBMS_TP.state.lightBits2.Launch` | `On` / `Off` |
| PriMode | `FalconBMS_TP.state.lightBits2.PriMode` | `On` / `Off` |
| Naval | `FalconBMS_TP.state.lightBits2.Naval` | `On` / `Off` |
| Unk | `FalconBMS_TP.state.lightBits2.Unk` | `On` / `Off` |
| TgtSep | `FalconBMS_TP.state.lightBits2.TgtSep` | `On` / `Off` |
| Go | `FalconBMS_TP.state.lightBits2.Go` | `On` / `Off` |
| NoGo | `FalconBMS_TP.state.lightBits2.NoGo` | `On` / `Off` |
| Degr | `FalconBMS_TP.state.lightBits2.Degr` | `On` / `Off` |
| Rdy | `FalconBMS_TP.state.lightBits2.Rdy` | `On` / `Off` |
| ChaffLo | `FalconBMS_TP.state.lightBits2.ChaffLo` | `On` / `Off` |
| FlareLo | `FalconBMS_TP.state.lightBits2.FlareLo` | `On` / `Off` |
| AuxSrch | `FalconBMS_TP.state.lightBits2.AuxSrch` | `On` / `Off` |
| AuxAct | `FalconBMS_TP.state.lightBits2.AuxAct` | `On` / `Off` |
| AuxLow | `FalconBMS_TP.state.lightBits2.AuxLow` | `On` / `Off` |
| AuxPwr | `FalconBMS_TP.state.lightBits2.AuxPwr` | `On` / `Off` |
| EcmPwr | `FalconBMS_TP.state.lightBits2.EcmPwr` | `On` / `Off` |
| EcmFail | `FalconBMS_TP.state.lightBits2.EcmFail` | `On` / `Off` |
| FwdFuelLow | `FalconBMS_TP.state.lightBits2.FwdFuelLow` | `On` / `Off` |
| AftFuelLow | `FalconBMS_TP.state.lightBits2.AftFuelLow` | `On` / `Off` |
| EPUOn | `FalconBMS_TP.state.lightBits2.EPUOn` | `On` / `Off` |
| JFSOn | `FalconBMS_TP.state.lightBits2.JFSOn` | `On` / `Off` |
| SEC | `FalconBMS_TP.state.lightBits2.SEC` | `On` / `Off` |
| OXY_LOW | `FalconBMS_TP.state.lightBits2.OXY_LOW` | `On` / `Off` |
| PROBEHEAT | `FalconBMS_TP.state.lightBits2.PROBEHEAT` | `On` / `Off` |
| SEAT_ARM | `FalconBMS_TP.state.lightBits2.SEAT_ARM` | `On` / `Off` |
| BUC | `FalconBMS_TP.state.lightBits2.BUC` | `On` / `Off` |
| FUEL_OIL_HOT | `FalconBMS_TP.state.lightBits2.FUEL_OIL_HOT` | `On` / `Off` |
| ANTI_SKID | `FalconBMS_TP.state.lightBits2.ANTI_SKID` | `On` / `Off` |
| TFR_ENGAGED | `FalconBMS_TP.state.lightBits2.TFR_ENGAGED` | `On` / `Off` |
| GEARHANDLE | `FalconBMS_TP.state.lightBits2.GEARHANDLE` | `On` / `Off` |
| ENGINE | `FalconBMS_TP.state.lightBits2.ENGINE` | `On` / `Off` |

## LightBits3

Todos los bits del enum `LightBits3` de `FlightData.h` (el nombre es el del enum).

| Nombre en Touch Portal | ID | Valores |
|---|---|---|
| FlcsPmg | `FalconBMS_TP.state.lightBits3.FlcsPmg` | `On` / `Off` |
| MainGen | `FalconBMS_TP.state.lightBits3.MainGen` | `On` / `Off` |
| StbyGen | `FalconBMS_TP.state.lightBits3.StbyGen` | `On` / `Off` |
| EpuGen | `FalconBMS_TP.state.lightBits3.EpuGen` | `On` / `Off` |
| EpuPmg | `FalconBMS_TP.state.lightBits3.EpuPmg` | `On` / `Off` |
| ToFlcs | `FalconBMS_TP.state.lightBits3.ToFlcs` | `On` / `Off` |
| FlcsRly | `FalconBMS_TP.state.lightBits3.FlcsRly` | `On` / `Off` |
| BatFail | `FalconBMS_TP.state.lightBits3.BatFail` | `On` / `Off` |
| Hydrazine | `FalconBMS_TP.state.lightBits3.Hydrazine` | `On` / `Off` |
| Air | `FalconBMS_TP.state.lightBits3.Air` | `On` / `Off` |
| Elec_Fault | `FalconBMS_TP.state.lightBits3.Elec_Fault` | `On` / `Off` |
| Lef_Fault | `FalconBMS_TP.state.lightBits3.Lef_Fault` | `On` / `Off` |
| OnGround | `FalconBMS_TP.state.lightBits3.OnGround` | `On` / `Off` |
| FlcsBitRun | `FalconBMS_TP.state.lightBits3.FlcsBitRun` | `On` / `Off` |
| FlcsBitFail | `FalconBMS_TP.state.lightBits3.FlcsBitFail` | `On` / `Off` |
| DbuWarn | `FalconBMS_TP.state.lightBits3.DbuWarn` | `On` / `Off` |
| NoseGearDown | `FalconBMS_TP.state.lightBits3.NoseGearDown` | `On` / `Off` |
| LeftGearDown | `FalconBMS_TP.state.lightBits3.LeftGearDown` | `On` / `Off` |
| RightGearDown | `FalconBMS_TP.state.lightBits3.RightGearDown` | `On` / `Off` |
| ParkBrakeOn | `FalconBMS_TP.state.lightBits3.ParkBrakeOn` | `On` / `Off` |
| Power_Off | `FalconBMS_TP.state.lightBits3.Power_Off` | `On` / `Off` |
| cadc | `FalconBMS_TP.state.lightBits3.cadc` | `On` / `Off` |
| SpeedBrake | `FalconBMS_TP.state.lightBits3.SpeedBrake` | `On` / `Off` |
| SysTest | `FalconBMS_TP.state.lightBits3.SysTest` | `On` / `Off` |
| MCAnnounced | `FalconBMS_TP.state.lightBits3.MCAnnounced` | `On` / `Off` |
| MLGWOW | `FalconBMS_TP.state.lightBits3.MLGWOW` | `On` / `Off` |
| NLGWOW | `FalconBMS_TP.state.lightBits3.NLGWOW` | `On` / `Off` |
| ATF_Not_Engaged | `FalconBMS_TP.state.lightBits3.ATF_Not_Engaged` | `On` / `Off` |
| Inlet_Icing | `FalconBMS_TP.state.lightBits3.Inlet_Icing` | `On` / `Off` |

## DED y PFL

| Nombre en Touch Portal | ID | Valores |
|---|---|---|
| DED linea 1 | `FalconBMS_TP.state.ded.line1` | Texto, hasta 25 caracteres (sin espacios al final) |
| DED linea 2 | `FalconBMS_TP.state.ded.line2` | Texto, hasta 25 caracteres (sin espacios al final) |
| DED linea 3 | `FalconBMS_TP.state.ded.line3` | Texto, hasta 25 caracteres (sin espacios al final) |
| DED linea 4 | `FalconBMS_TP.state.ded.line4` | Texto, hasta 25 caracteres (sin espacios al final) |
| DED linea 5 | `FalconBMS_TP.state.ded.line5` | Texto, hasta 25 caracteres (sin espacios al final) |
| PFL linea 1 | `FalconBMS_TP.state.pfl.line1` | Texto, hasta 25 caracteres (sin espacios al final) |
| PFL linea 2 | `FalconBMS_TP.state.pfl.line2` | Texto, hasta 25 caracteres (sin espacios al final) |
| PFL linea 3 | `FalconBMS_TP.state.pfl.line3` | Texto, hasta 25 caracteres (sin espacios al final) |
| PFL linea 4 | `FalconBMS_TP.state.pfl.line4` | Texto, hasta 25 caracteres (sin espacios al final) |
| PFL linea 5 | `FalconBMS_TP.state.pfl.line5` | Texto, hasta 25 caracteres (sin espacios al final) |
