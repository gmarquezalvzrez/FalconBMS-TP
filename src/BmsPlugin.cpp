#include "BmsPlugin.h"
#include "States.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>

namespace {
// SUPOSICION: 100 ms (10 Hz) es suficiente para botones e indicadores y no satura Touch Portal.
// Se puede cambiar desde los ajustes del plugin en Touch Portal.
constexpr int DEFAULT_INTERVAL_MS = 100;
constexpr int MIN_INTERVAL_MS = 20;
constexpr int MAX_INTERVAL_MS = 5000;
// Cada cuanto se comprueba si "Falcon BMS.exe" sigue vivo / se intenta abrir la memoria.
constexpr qint64 PROCESS_CHECK_MS = 2000;

// Si Touch Portal corta la conexion sin pedir el cierre, se reintenta durante ~1 minuto.
constexpr int RECONNECT_DELAY_MS = 2000;
constexpr int MAX_RECONNECT_ATTEMPTS = 30;

const char *SETTING_INTERVAL = "Intervalo de actualizacion (ms)";
}

BmsPlugin::BmsPlugin(QObject *parent) :
	QObject(parent),
	m_client(new TPClientQt(PLUGIN_ID, this))
{
	connect(m_client, &TPClientQt::connected, this, &BmsPlugin::onConnected);
	connect(m_client, &TPClientQt::disconnected, this, &BmsPlugin::onDisconnected);
	connect(m_client, &TPClientQt::error, this, &BmsPlugin::onError);
	connect(m_client, &TPClientQt::message, this, &BmsPlugin::onMessage);

	m_pollTimer.setTimerType(Qt::PreciseTimer);
	m_pollTimer.setInterval(DEFAULT_INTERVAL_MS);
	connect(&m_pollTimer, &QTimer::timeout, this, &BmsPlugin::poll);

	m_reconnectTimer.setSingleShot(true);
	m_reconnectTimer.setInterval(RECONNECT_DELAY_MS);
	connect(&m_reconnectTimer, &QTimer::timeout, this, [this]() {
		qInfo() << "Reintentando conectar con Touch Portal, intento" << m_reconnectAttempts;
		m_client->connect();
	});
}

void BmsPlugin::scheduleReconnect()
{
	if (m_closing || m_reconnectTimer.isActive() || m_client->isConnected())
		return;
	if (++m_reconnectAttempts > MAX_RECONNECT_ATTEMPTS) {
		qWarning() << "No se pudo reconectar con Touch Portal; saliendo.";
		QCoreApplication::exit(1);
		return;
	}
	m_reconnectTimer.start();
}

void BmsPlugin::start()
{
	m_client->connect();
}

void BmsPlugin::onConnected(const TPClientQt::TPInfo &info, const QJsonObject &settings)
{
	qInfo() << "Conectado a Touch Portal" << info.tpVersionString << "SDK" << info.sdkVersion;
	m_reconnectAttempts = 0;
	applySettings(settings);
	m_lastSent.clear();  // enviar todos los valores de nuevo tras conectar
	m_lastProcessCheck = 0;
	m_pollTimer.start();
	poll();
}

void BmsPlugin::onDisconnected()
{
	m_pollTimer.stop();
	if (m_closing) {
		qInfo() << "Desconectado de Touch Portal; saliendo.";
		m_shm.closeAll();
		QCoreApplication::exit(0);
		return;
	}
	qWarning() << "Touch Portal cerro la conexion sin pedir el cierre del plugin:" << m_client->errorString();
	scheduleReconnect();
}

void BmsPlugin::onError(QAbstractSocket::SocketError err)
{
	qWarning() << "Error de conexion con Touch Portal:" << err << m_client->errorString();
	if (!m_client->isConnected())
		scheduleReconnect();
}

void BmsPlugin::onMessage(TPClientQt::MessageType type, const QJsonObject &msg)
{
	qInfo().noquote() << "Mensaje de Touch Portal:" << msg.value(QLatin1String("type")).toString(QStringLiteral("(settings)"));
	switch (type) {
		case TPClientQt::MessageType::settings:
			applySettings(msg);
			break;
		case TPClientQt::MessageType::closePlugin:
			qInfo() << "Touch Portal pidio cerrar el plugin.";
			m_closing = true;
			m_pollTimer.stop();
			m_shm.closeAll();
			m_client->disconnect();
			QCoreApplication::quit();  // salir ya, sin esperar a que se cierre el socket
			break;
		default:
			break;  // esta primera version no define acciones
	}
}

void BmsPlugin::applySettings(const QJsonObject &settings)
{
	const QJsonValue v = settings.value(QString::fromUtf8(SETTING_INTERVAL));
	if (v.isUndefined())
		return;
	bool ok = false;
	int ms = v.toVariant().toString().toInt(&ok);
	if (!ok)
		return;
	ms = qBound(MIN_INTERVAL_MS, ms, MAX_INTERVAL_MS);
	m_pollTimer.setInterval(ms);
	qInfo() << "Intervalo de actualizacion:" << ms << "ms";
}

void BmsPlugin::sendState(const std::string &fullId, const std::string &value)
{
	auto it = m_lastSent.find(fullId);
	if (it != m_lastSent.end() && it->second == value)
		return;
	m_lastSent[fullId] = value;
	m_client->stateUpdate(fullId.c_str(), value.c_str());
	if (fullId == STATE_ID_PREFIX "lights.masterCaution")
		qInfo().noquote() << "Enviado a Touch Portal: MASTER CAUTION =" << QString::fromStdString(value);
}

void BmsPlugin::poll()
{
	if (!m_client->isConnected())
		return;

	const qint64 now = QDateTime::currentMSecsSinceEpoch();
	if (now - m_lastProcessCheck >= PROCESS_CHECK_MS) {
		m_lastProcessCheck = now;
		const bool proc = BmsSharedMemory::isBmsProcessRunning();
		if (proc) {
			m_procSeen = true;
		}
		else if (m_procSeen && m_shm.anyOpen()) {
			// BMS se cerro: soltar las zonas para que Windows las libere y no queden datos viejos
			qInfo() << "Falcon BMS.exe ya no esta en ejecucion; cerrando la memoria compartida.";
			m_shm.closeAll();
			m_procSeen = false;
		}
		// Se intenta abrir siempre, aunque no se encuentre el proceso, por si el ejecutable
		// tiene otro nombre en tu version de BMS.
		m_shm.tryOpen();

		std::string desc = m_shm.describe() + (proc ? " | proceso Falcon BMS.exe: si" : " | proceso Falcon BMS.exe: no");
		if (desc != m_lastDescribe) {
			m_lastDescribe = desc;
			qInfo().noquote() << "Estado:" << QString::fromStdString(desc);
		}
		m_snap.diag = desc;
	}

	{
		std::string diag = std::move(m_snap.diag);
		m_shm.read(m_snap);
		m_snap.diag = std::move(diag);
	}
	const bool running = m_snap.bmsRunning();

	// Registrar en el log los cambios de las luces (en hexadecimal) para poder depurar
	const unsigned lb[3] = { m_snap.fd.lightBits, m_snap.fd.lightBits2, m_snap.fd.lightBits3 };
	if (running && (lb[0] != m_lastLightBits[0] || lb[1] != m_lastLightBits[1] || lb[2] != m_lastLightBits[2])) {
		qInfo().noquote() << QStringLiteral("lightBits=0x%1 lightBits2=0x%2 lightBits3=0x%3 MasterCaution=%4")
		                     .arg(lb[0], 8, 16, QLatin1Char('0')).arg(lb[1], 8, 16, QLatin1Char('0'))
		                     .arg(lb[2], 8, 16, QLatin1Char('0')).arg((lb[0] & FlightData::MasterCaution) ? "On" : "Off");
		std::copy(lb, lb + 3, m_lastLightBits);
	}

	for (const StateDef &def : stateDefinitions()) {
		// Con BMS cerrado solo se actualizan los estados del simulador; el resto conserva su ultimo valor.
		if (!running && def.id.rfind("sim.", 0) != 0)
			continue;
		sendState(def.fullId(), def.value(m_snap));
	}

	// Cada 10 s (cada 1 s durante los primeros 15 s), unos cuantos valores al log
	if (m_firstPoll == 0)
		m_firstPoll = now;
	const qint64 hbEvery = (now - m_firstPoll < 15000) ? 1000 : 10000;
	if (now - m_lastHeartbeat >= hbEvery) {
		m_lastHeartbeat = now;
		qInfo().noquote() << QStringLiteral("Latido: conectado=%1 ").arg(m_client->isConnected() ? "si" : "no")
		                  + QStringLiteral("volando=%1 en3D=%2 pausa=%3 KIAS=%4 alt=%5 RPM=%6 lightBits=0x%7")
		                     .arg(m_snap.flying() ? "si" : "no").arg(m_snap.iv.In3D ? "si" : "no")
		                     .arg(m_snap.iv.IsPaused ? "si" : "no").arg(qRound(m_snap.fd.kias))
		                     .arg(qRound(-m_snap.fd.z)).arg(qRound(m_snap.fd.rpm))
		                     .arg(m_snap.fd.lightBits, 8, 16, QLatin1Char('0'));
	}

	if (running != m_wasRunning) {
		m_wasRunning = running;
		qInfo() << (running ? "Memoria compartida de BMS abierta." : "BMS no esta en ejecucion.");
	}
}
