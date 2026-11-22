# Falcon BMS para Touch Portal (C++)

Plugin de Touch Portal que lee la memoria compartida de Falcon BMS **en modo solo lectura** y publica los
valores como estados de Touch Portal. Usa el cliente C++/Qt de Max Paperno
([TouchPortal-CPP-API](https://github.com/mpaperno/TouchPortal-CPP-API), copiado en `third_party/TPClientQt`).

Primera versión: solo estados (88), sin acciones ni eventos.

## Estructura

| Ruta | Contenido |
|---|---|
| `include/bms/` | `FlightData.h` e `IVibeData.h` de BMS, sin cambios |
| `src/BmsSharedMemory.*` | Abre las zonas con `OpenFileMapping(FILE_MAP_READ)` y copia cada una en cada ciclo |
| `src/States.*` | **Tabla única de estados**: id, grupo, nombre y cómo se calcula cada valor |
| `src/BmsPlugin.*` | Conexión con Touch Portal, temporizador de lectura, solo envía los valores que cambian |
| `src/main.cpp` | Arranque; `--entry-tp FICHERO` genera `entry.tp` a partir de la tabla |
| `entry.tp` | Generado desde la tabla de estados. No editar a mano: regenerar |

Zonas que se leen: `FalconSharedMemoryArea` (FlightData), `FalconSharedMemoryArea2` (FlightData2) y
`FalconIntellivibeSharedMemoryArea` (IntellivibeData). OSB, StringData y DrawingData todavía no.

## Cómo funciona

- Cada 2 s comprueba si hay un proceso `Falcon BMS.exe`. Si lo hay, abre las zonas; si desaparece, las cierra
  (si no, Windows las mantendría vivas con los últimos datos).
- Cada 100 ms (ajustable en *Ajustes → Plugins → Falcon BMS → Intervalo de actualizacion*) copia las zonas,
  calcula todos los estados y envía a Touch Portal solo los que han cambiado.
- Con BMS cerrado solo se actualizan los estados del grupo *Simulador* (`BMS: en ejecución = Off`).
- Las zonas antiguas más pequeñas que la estructura actual se leen hasta donde llegan; el resto queda a cero.

## Compilar en Windows

Requisitos:
1. Visual Studio 2022 (carga "Desarrollo para el escritorio con C++").
2. Qt 6 (kit `MSVC 2022 64-bit`). Basta con Qt Core y Qt Network. Ver "Instalar Qt 6" más abajo.
3. CMake 3.16 o superior (viene con Visual Studio).

### Instalar Qt 6

**Opción A: instalador oficial (necesita una cuenta Qt gratuita)**

1. Descarga el *Qt Online Installer* para Windows desde https://www.qt.io/download-qt-installer-oss
2. Ejecútalo e inicia sesión con tu cuenta Qt (o créala desde el propio instalador).
3. Acepta la licencia de código abierto (LGPL) y marca que eres un particular.
4. En "Installation Folder" deja `C:\Qt` y elige **Custom installation**.
5. En la lista de componentes despliega `Qt` → `Qt 6.8.x` (la versión LTS más reciente) y marca solo
   **MSVC 2022 64-bit**. No hace falta nada más (ni Sources, ni Qt Creator, ni módulos adicionales:
   Core y Network vienen en el kit base).
6. Instala. Quedará en `C:\Qt\6.8.x\msvc2022_64` (con `bin\windeployqt.exe` dentro).

**Opción B: sin cuenta, con aqtinstall (necesita Python)**

```bat
pip install aqtinstall
aqt install-qt windows desktop 6.8.3 win64_msvc2022_64 -O C:\Qt
```

Queda en `C:\Qt\6.8.3\msvc2022_64`.

**Comprobar**: `C:\Qt\6.8.3\msvc2022_64\bin\qmake.exe -v` debe mostrar la versión. Esa carpeta
`...\msvc2022_64` es la que se pasa en `CMAKE_PREFIX_PATH` (cambia `6.8.3` por la versión que tengas).

### Compilar desde Visual Studio (Abrir carpeta)

1. *Archivo → Abrir → Carpeta* y elige la carpeta del proyecto.
2. Arriba, en la lista de configuraciones, elige **Windows x64 Release (Qt 6, MSVC)** (viene de `CMakePresets.json`).
3. CMake busca Qt en `C:\Qt\6.x.y\msvc20xx_64` automáticamente. Si lo instalaste en otro sitio, cambia
   `QT_ROOT` en `CMakePresets.json` o define la variable de entorno `QT_ROOT` con esa carpeta.
4. *Compilar → Compilar todo*.

### Compilar desde la consola

Desde una consola "x64 Native Tools Command Prompt for VS 2022":

```bat
cmake -S . -B build -DCMAKE_PREFIX_PATH=C:\Qt\6.8.3\msvc2022_64
cmake --build build --config Release
cmake --build build --config Release --target tpp
```

Al compilar se genera la carpeta **`build\out\Release\FalconBMS-TP\`** con todo lo necesario:
`FalconBMS-TP.exe`, `entry.tp`, las DLL de Qt (con `windeployqt`) y las DLL del runtime de Visual C++.
Para instalarlo a mano, copia esa carpeta entera dentro de `%APPDATA%\TouchPortal\plugins\`.

El último paso (`--target tpp`) crea además `build\package\FalconBMS-TP.tpp`, que es esa misma carpeta
en un zip. Es lo que se pasa a otra persona: se instala en Touch Portal con *Ajustes → Plug-ins → Import plug-in*.

### Añadir o cambiar estados

1. Edita la tabla en `src/States.cpp`.
2. Compila y regenera el fichero: `build\out\Release\FalconBMS-TP\FalconBMS-TP.exe --entry-tp entry.tp`
3. Sube `PLUGIN_VERSION` en `src/States.h` si cambias algo que Touch Portal deba recargar.

## Runtime de Visual C++ (importante)

Touch Portal arranca el plugin con su carpeta `runtime\bin` en el PATH, y ahí hay un `MSVCP140.dll`
antiguo. Si el plugin lo usa, se cae al segundo de arrancar (`CRASH: excepcion 0xC0000005 ... MSVCP140.dll`).
Por eso `msvcp140.dll`, `vcruntime140.dll` y `vcruntime140_1.dll` tienen que estar **en la carpeta del plugin**,
junto al `.exe`: Windows busca primero ahí. La compilación ya las copia junto al `.exe`.

## Diagnóstico

- El plugin escribe `FalconBMS-TP.log` en su carpeta (`%APPDATA%\TouchPortal\plugins\FalconBMS-TP`). Se vacía en cada arranque.
  Registra la conexión con Touch Portal, el estado de cada zona de memoria y cada cambio de las luces (`lightBits`).
- El estado *Plugin: diagnostico de la memoria compartida* muestra lo mismo en Touch Portal.
- Error 2 = BMS no ha creado esa zona (no está en marcha). Error 5 = acceso denegado: suele pasar si BMS se ejecuta
  como administrador y Touch Portal no (o al revés). Ejecuta los dos con el mismo nivel.

## Suposiciones pendientes de comprobar con BMS en marcha

Están marcadas con `SUPOSICIÓN` en el código:

- **DED/PFL**: `0x01` se muestra como `↕` y `0x02` como `°`.
- **Intervalo**: 100 ms por defecto.
- Estados de luces con valores `On`/`Off`; aún no se distingue luz fija de luz parpadeante.
