#pragma once

// Acceso de solo lectura a las zonas de memoria compartida de Falcon BMS.
//
// BMS crea las zonas con CreateFileMapping() y nombres fijos (ver FlightData.h / IVibeData.h).
// Aqui solo se abren con OpenFileMapping(FILE_MAP_READ) y MapViewOfFile(FILE_MAP_READ),
// asi que el plugin nunca puede escribir en ellas.
//
// En sistemas que no son Windows la clase compila pero open() siempre falla. Eso permite
// compilar en Linux para generar entry.tp (opcion --entry-tp) o para pruebas.

#include <cstddef>
#include <string>
#include <cstring>   // FlightData.h usa memcpy sin incluir <cstring>

#include "bms/FlightData.h"
#include "bms/IVibeData.h"

class SharedArea
{
	public:
		SharedArea(const char *name, size_t size) : m_name(name), m_size(size) {}
		~SharedArea() { close(); }
		SharedArea(const SharedArea &) = delete;
		SharedArea &operator=(const SharedArea &) = delete;

		bool open();
		void close();
		bool isOpen() const { return m_view != nullptr; }
		const char *name() const { return m_name; }
		// Ultimo codigo de error de Windows al abrir (0 = sin error). 2 = no existe (BMS no la ha creado), 5 = acceso denegado.
		unsigned long lastError() const { return m_lastError; }

		// Copia 'm_size' bytes de la zona compartida en 'dest'. Devuelve false si no esta abierta.
		bool read(void *dest) const;

	private:
		const char *m_name;
		size_t m_size;
		void *m_handle = nullptr;
		const void *m_view = nullptr;
		unsigned long m_lastError = 0;
};

// Copia instantanea de todas las zonas que usa el plugin, tomada en cada ciclo de lectura.
struct BmsSnapshot
{
	bool fdOk = false;    // "FalconSharedMemoryArea"
	bool fd2Ok = false;   // "FalconSharedMemoryArea2"
	bool ivOk = false;    // "FalconIntellivibeSharedMemoryArea"
	FlightData fd {};
	FlightData2 fd2 {};
	IntellivibeData iv {};
	std::string diag;     // texto de diagnostico que rellena el plugin (no viene de BMS)

	bool bmsRunning() const { return fdOk; }
	bool flying() const { return fdOk && (fd.hsiBits & FlightData::Flying); }
};

class BmsSharedMemory
{
	public:
		BmsSharedMemory();

		// Intenta abrir las zonas que aun no esten abiertas. Barato si ya lo estan.
		void tryOpen();
		void closeAll();
		bool anyOpen() const;

		// Lee todas las zonas abiertas en 'out'.
		void read(BmsSnapshot &out) const;

		// true si hay un proceso "Falcon BMS.exe" en marcha. Hace falta porque mientras el plugin
		// mantenga abiertas las zonas, Windows no las destruye al cerrar BMS y seguirian
		// mostrando los ultimos datos.
		static bool isBmsProcessRunning();

		// Texto para el log y para el estado de diagnostico, p. ej.
		// "FalconSharedMemoryArea: abierta; FalconSharedMemoryArea2: error 2; ..."
		std::string describe() const;

	private:
		SharedArea m_fd;
		SharedArea m_fd2;
		SharedArea m_iv;
};
