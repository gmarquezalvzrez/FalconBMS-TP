#pragma once

// Tabla unica de estados que el plugin publica en Touch Portal.
// La misma tabla se usa para enviar valores y para generar entry.tp (--entry-tp),
// asi ambos no pueden desincronizarse.

#include <functional>
#include <string>
#include <vector>

struct BmsSnapshot;

#define PLUGIN_ID        "FalconBMS_TP"
#define PLUGIN_NAME      "Falcon BMS"
#define PLUGIN_VERSION   9
#define STATE_ID_PREFIX  PLUGIN_ID ".state."

struct StateDef
{
	std::string id;                    // sin prefijo, p. ej. "flight.kias"
	std::string group;                 // subgrupo en Touch Portal ("parentGroup")
	std::string desc;                  // nombre visible en Touch Portal
	std::string defaultValue;
	std::vector<std::string> choices;  // vacio = estado de texto; si no, tipo "choice"
	std::function<std::string(const BmsSnapshot &)> value;

	std::string fullId() const { return STATE_ID_PREFIX + id; }
};

const std::vector<StateDef> &stateDefinitions();

// Valores usados por los estados de tipo encendido/apagado.
extern const char *const VAL_ON;
extern const char *const VAL_OFF;
