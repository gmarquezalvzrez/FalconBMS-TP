#pragma once

#include <QObject>
#include <QTimer>

#include <string>
#include <unordered_map>

#include "BmsSharedMemory.h"
#include "TPClientQt.h"

class BmsPlugin : public QObject
{
	Q_OBJECT
	public:
		explicit BmsPlugin(QObject *parent = nullptr);
		void start();

	private:
		void onConnected(const TPClientQt::TPInfo &info, const QJsonObject &settings);
		void onDisconnected();
		void onError(QAbstractSocket::SocketError err);
		void onMessage(TPClientQt::MessageType type, const QJsonObject &msg);
		void applySettings(const QJsonObject &settings);
		void poll();
		void scheduleReconnect();
		void sendState(const std::string &fullId, const std::string &value);

		TPClientQt *m_client;
		QTimer m_pollTimer;
		QTimer m_reconnectTimer;
		int m_reconnectAttempts = 0;
		bool m_closing = false;   // Touch Portal pidio cerrar: no reconectar
		BmsSharedMemory m_shm;
		BmsSnapshot m_snap;
		std::unordered_map<std::string, std::string> m_lastSent;  // ultimo valor enviado por estado
		qint64 m_lastProcessCheck = 0;
		bool m_wasRunning = false;
		bool m_procSeen = false;        // se ha visto "Falcon BMS.exe" desde que se abrio la memoria
		std::string m_lastDescribe;
		qint64 m_lastHeartbeat = 0;
		qint64 m_firstPoll = 0;
		unsigned m_lastLightBits[3] = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };
};
