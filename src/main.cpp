// Falcon BMS para Touch Portal: lee la memoria compartida de BMS (solo lectura)
// y publica los valores como estados de Touch Portal.
//
// Uso:
//   FalconBMS-TP.exe                  -> modo plugin (lo arranca Touch Portal)
//   FalconBMS-TP.exe --entry-tp FILE  -> genera entry.tp a partir de la tabla de estados y sale

#include <QCoreApplication>
#include <QDateTime>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>
#include <QTimer>

#include <cstdio>
#include <cstdlib>
#include <exception>

#include "BmsPlugin.h"
#include "States.h"

static QJsonObject buildEntryTp()
{
	QJsonArray states;
	for (const StateDef &d : stateDefinitions()) {
		QJsonObject st {
			{ "id", QString::fromStdString(d.fullId()) },
			{ "desc", QString::fromStdString(d.desc) },
			{ "default", QString::fromStdString(d.defaultValue) },
			{ "parentGroup", QString::fromStdString(d.group) },
		};
		if (d.choices.empty()) {
			st.insert("type", "text");
		}
		else {
			QJsonArray ch;
			for (const auto &c : d.choices)
				ch.append(QString::fromStdString(c));
			st.insert("type", "choice");
			st.insert("valueChoices", ch);
		}
		states.append(st);
	}

	const QJsonObject category {
		{ "id", PLUGIN_ID ".cat.main" },
		{ "name", PLUGIN_NAME },
		{ "actions", QJsonArray() },
		{ "events", QJsonArray() },
		{ "connectors", QJsonArray() },
		{ "states", states },
	};

	const QJsonObject intervalSetting {
		{ "name", "Intervalo de actualizacion (ms)" },
		{ "type", "number" },
		{ "default", "100" },
		{ "minValue", 20 },
		{ "maxValue", 5000 },
		{ "readOnly", false },
	};

	return QJsonObject {
		{ "sdk", 6 },
		{ "version", PLUGIN_VERSION },
		{ "name", PLUGIN_NAME },
		{ "id", PLUGIN_ID },
		{ "configuration", QJsonObject {
			{ "colorDark", "#1b2a3a" },
			{ "colorLight", "#3d6b99" },
			{ "parentCategory", "games" },
		} },
		{ "plugin_start_cmd_windows", "\"%TP_PLUGIN_FOLDER%FalconBMS-TP\\FalconBMS-TP.exe\"" },
		{ "settings", QJsonArray { intervalSetting } },
		{ "categories", QJsonArray { category } },
	};
}

// Log en FalconBMS-TP.log, junto al .exe (se vacia en cada arranque).
static QFile *g_logFile = nullptr;

static void logHandler(QtMsgType type, const QMessageLogContext &, const QString &msg)
{
	static const char *lvl[] = { "DEBUG", "WARN ", "ERROR", "FATAL", "INFO " };
	const QByteArray line = QDateTime::currentDateTime().toString("HH:mm:ss.zzz").toUtf8() + " [" +
	                        (int(type) >= 0 && int(type) <= 4 ? lvl[type] : "?") + "] " + msg.toUtf8() + "\n";
	if (g_logFile) {
		g_logFile->write(line);
		g_logFile->flush();
	}
	// No se escribe en stderr: Touch Portal lo copia en su log marcado como ERROR.
}

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

// Si el plugin se cae, deja en el log el codigo de la excepcion y la direccion
// (relativa al .exe) para poder localizar el fallo.
static LONG WINAPI crashHandler(EXCEPTION_POINTERS *ep)
{
	if (g_logFile) {
		const auto addr = reinterpret_cast<quintptr>(ep->ExceptionRecord->ExceptionAddress);
		const auto base = reinterpret_cast<quintptr>(GetModuleHandleW(nullptr));
		HMODULE mod = nullptr;
		char modName[MAX_PATH] = "?";
		if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		                       reinterpret_cast<LPCWSTR>(addr), &mod) && mod)
			GetModuleFileNameA(mod, modName, MAX_PATH);
		char buf[700];
		std::snprintf(buf, sizeof(buf), "CRASH: excepcion 0x%08lX en 0x%llX (exe base 0x%llX, offset 0x%llX) modulo %s\n",
		              static_cast<unsigned long>(ep->ExceptionRecord->ExceptionCode),
		              static_cast<unsigned long long>(addr), static_cast<unsigned long long>(base),
		              static_cast<unsigned long long>(mod ? addr - reinterpret_cast<quintptr>(mod) : 0), modName);
		g_logFile->write(buf);
		g_logFile->flush();
	}
	return EXCEPTION_CONTINUE_SEARCH;
}
#endif

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);
	QCoreApplication::setApplicationName("FalconBMS-TP");

	const QStringList args = app.arguments();
	const int idx = args.indexOf("--entry-tp");
	if (idx >= 0) {
		const QByteArray json = QJsonDocument(buildEntryTp()).toJson(QJsonDocument::Indented);
		if (idx + 1 < args.size()) {
			QFile f(args.at(idx + 1));
			if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
				std::fprintf(stderr, "No se puede escribir %s\n", qPrintable(args.at(idx + 1)));
				return 1;
			}
			f.write(json);
		}
		else {
			std::fwrite(json.constData(), 1, size_t(json.size()), stdout);
		}
		return 0;
	}

	g_logFile = new QFile(QCoreApplication::applicationDirPath() + "/FalconBMS-TP.log", &app);
	if (!g_logFile->open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
		delete g_logFile;
		g_logFile = nullptr;
	}
	qInstallMessageHandler(logHandler);
#ifdef _WIN32
	SetUnhandledExceptionFilter(crashHandler);
#endif
	std::set_terminate([] {
		qCritical() << "CRASH: std::terminate (excepcion C++ no capturada)";
		std::abort();
	});
	QObject::connect(&app, &QCoreApplication::aboutToQuit, [] { qInfo() << "Saliendo del bucle de eventos."; });
	qInfo() << "FalconBMS-TP version" << PLUGIN_VERSION << "arrancando";

	BmsPlugin plugin;
	plugin.start();
	// Durante los primeros 20 s, una linea por segundo para saber hasta cuando sigue vivo el proceso
	QTimer alive;
	int aliveCount = 0;
	QObject::connect(&alive, &QTimer::timeout, [&] {
		qInfo() << "Proceso vivo," << ++aliveCount << "s";
		if (aliveCount >= 20)
			alive.stop();
	});
	alive.start(1000);

	const int ret = app.exec();
	qInfo() << "Fin del plugin, codigo" << ret;
	return ret;
}
