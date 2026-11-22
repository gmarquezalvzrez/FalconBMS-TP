#include "BmsSharedMemory.h"

#ifdef _WIN32
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif
	#include <windows.h>
	#include <tlhelp32.h>
#endif

#include <algorithm>
#include <initializer_list>

bool SharedArea::open()
{
	if (m_view)
		return true;
#ifdef _WIN32
	HANDLE h = OpenFileMappingA(FILE_MAP_READ, FALSE, m_name);
	if (!h) {
		m_lastError = GetLastError();
		return false;
	}
	// Se mapea la zona completa (tamano 0) porque versiones antiguas de BMS crean zonas
	// mas pequenas que las estructuras actuales; read() copia solo lo que exista.
	const void *view = MapViewOfFile(h, FILE_MAP_READ, 0, 0, 0);
	if (!view) {
		m_lastError = GetLastError();
		CloseHandle(h);
		return false;
	}
	m_handle = h;
	m_view = view;
	m_lastError = 0;
	return true;
#else
	m_lastError = 2;
	return false;
#endif
}

void SharedArea::close()
{
#ifdef _WIN32
	if (m_view)
		UnmapViewOfFile(m_view);
	if (m_handle)
		CloseHandle(static_cast<HANDLE>(m_handle));
#endif
	m_view = nullptr;
	m_handle = nullptr;
}

bool SharedArea::read(void *dest) const
{
	if (!m_view)
		return false;
	size_t avail = m_size;
#ifdef _WIN32
	MEMORY_BASIC_INFORMATION mbi {};
	if (VirtualQuery(m_view, &mbi, sizeof(mbi)))
		avail = std::min(m_size, static_cast<size_t>(mbi.RegionSize));
#endif
	std::memcpy(dest, m_view, avail);
	if (avail < m_size)
		std::memset(static_cast<char *>(dest) + avail, 0, m_size - avail);
	return true;
}

BmsSharedMemory::BmsSharedMemory() :
	m_fd("FalconSharedMemoryArea", sizeof(FlightData)),
	m_fd2("FalconSharedMemoryArea2", sizeof(FlightData2)),
	m_iv("FalconIntellivibeSharedMemoryArea", sizeof(IntellivibeData))
{}

void BmsSharedMemory::tryOpen()
{
	m_fd.open();
	m_fd2.open();
	m_iv.open();
}

void BmsSharedMemory::closeAll()
{
	m_fd.close();
	m_fd2.close();
	m_iv.close();
}

bool BmsSharedMemory::anyOpen() const
{
	return m_fd.isOpen() || m_fd2.isOpen() || m_iv.isOpen();
}

void BmsSharedMemory::read(BmsSnapshot &out) const
{
	out.fdOk = m_fd.read(&out.fd);
	out.fd2Ok = m_fd2.read(&out.fd2);
	out.ivOk = m_iv.read(&out.iv);
	// Si una zona no esta disponible se deja a cero para no mostrar datos viejos.
	if (!out.fdOk)  out.fd = FlightData {};
	if (!out.fd2Ok) out.fd2 = FlightData2 {};
	if (!out.ivOk)  out.iv = IntellivibeData {};
}

bool BmsSharedMemory::isBmsProcessRunning()
{
#ifdef _WIN32
	HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snap == INVALID_HANDLE_VALUE)
		return true;  // si no se puede comprobar, se asume que si para no cortar la lectura
	PROCESSENTRY32W pe {};
	pe.dwSize = sizeof(pe);
	bool found = false;
	for (BOOL ok = Process32FirstW(snap, &pe); ok && !found; ok = Process32NextW(snap, &pe))
		found = (_wcsicmp(pe.szExeFile, L"Falcon BMS.exe") == 0);
	CloseHandle(snap);
	return found;
#else
	return false;
#endif
}

std::string BmsSharedMemory::describe() const
{
	std::string out;
	for (const SharedArea *a : { &m_fd, &m_fd2, &m_iv }) {
		if (!out.empty())
			out += "; ";
		out += a->name();
		out += a->isOpen() ? std::string(": abierta") : (a->lastError() ? ": error " + std::to_string(a->lastError()) : std::string(": cerrada"));
	}
	return out;
}
