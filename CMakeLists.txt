cmake_minimum_required(VERSION 3.16)

project(FalconBMS-TP
  VERSION 1.0.0
  DESCRIPTION "Plugin de Touch Portal que lee la memoria compartida de Falcon BMS"
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# Localizar Qt. Orden: QT_ROOT (cache o variable de entorno), CMAKE_PREFIX_PATH,
# y si no hay nada, la version mas reciente instalada en C:\Qt\6.x.y\msvc20xx_64.
set(QT_ROOT "$ENV{QT_ROOT}" CACHE PATH "Carpeta del kit de Qt, p. ej. C:/Qt/6.8.3/msvc2022_64")
if (QT_ROOT AND EXISTS "${QT_ROOT}/lib/cmake")
  list(PREPEND CMAKE_PREFIX_PATH "${QT_ROOT}")
elseif (WIN32 AND NOT Qt6_DIR)
  if (QT_ROOT)
    message(WARNING "QT_ROOT=${QT_ROOT} no contiene Qt; buscando en C:/Qt")
  endif()
  file(GLOB _qt_kits LIST_DIRECTORIES true "C:/Qt/6.*/msvc20*_64")
  if (_qt_kits)
    list(SORT _qt_kits COMPARE NATURAL ORDER DESCENDING)
    list(GET _qt_kits 0 _qt_kit)
    message(STATUS "Qt encontrado automaticamente en ${_qt_kit}")
    list(PREPEND CMAKE_PREFIX_PATH "${_qt_kit}")
  endif()
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network)

add_executable(${PROJECT_NAME}
  src/main.cpp
  src/BmsPlugin.h
  src/BmsPlugin.cpp
  src/BmsSharedMemory.h
  src/BmsSharedMemory.cpp
  src/States.h
  src/States.cpp
  third_party/TPClientQt/TPClientQt.h
  third_party/TPClientQt/TPClientQt.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
  include
  src
  third_party/TPClientQt
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
  QT_USE_QSTRINGBUILDER
  QT_NO_KEYWORDS
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  Qt${QT_VERSION_MAJOR}::Core
  Qt${QT_VERSION_MAJOR}::Network
)

if (MSVC)
  # Touch Portal arranca el plugin con su propio runtime\bin (Java) en el PATH, que trae un
  # MSVCP140.dll antiguo. Con VS 2022 17.10+ eso provoca un 0xC0000005 en std::mutex.
  # Esta definicion evita el constructor constexpr de std::mutex en nuestro codigo; ademas
  # el paquete .tpp lleva las DLL del runtime junto al .exe (se cargan antes que las del PATH).
  target_compile_definitions(${PROJECT_NAME} PRIVATE _DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR)
  # Fuentes en UTF-8
  target_compile_options(${PROJECT_NAME} PRIVATE /utf-8 /W3)
endif()

if (WIN32)
  # Sin ventana de consola cuando lo arranca Touch Portal
  set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

# ---------------------------------------------------------------------------------------------
# Despliegue
#  - Al compilar, la carpeta build/out/<Config>/FalconBMS-TP queda lista para copiar a
#    %APPDATA%\TouchPortal\plugins\: .exe + entry.tp + DLL de Qt + DLL del runtime de MSVC.
#  - Target "tpp": crea build/package/FalconBMS-TP.tpp (zip para "Import plug-in" de Touch Portal)
#      cmake --build build --config Release --target tpp
# ---------------------------------------------------------------------------------------------
set(PKG_ROOT "${CMAKE_BINARY_DIR}/package")
set(PKG_DIR  "${PKG_ROOT}/FalconBMS-TP")

# El .exe se genera en build/out/<Config>/FalconBMS-TP/ : esa carpeta entera es el plugin
# (se copia tal cual a %APPDATA%\TouchPortal\plugins\).
set_target_properties(${PROJECT_NAME} PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/out/$<CONFIG>/FalconBMS-TP")

# Siempre se copia entry.tp junto al .exe
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/entry.tp" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/"
  VERBATIM
)

if (MSVC)
  # Runtime de Visual C++ junto al .exe. Imprescindible: Touch Portal pone en el PATH su runtime\bin
  # con un MSVCP140.dll antiguo que hace caer el plugin (0xC0000005). Windows busca antes en la carpeta del .exe.
  set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
  set(CMAKE_INSTALL_UCRT_LIBRARIES FALSE)
  include(InstallRequiredSystemLibraries)
  if (CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} "$<TARGET_FILE_DIR:${PROJECT_NAME}>/"
      COMMENT "Copiando el runtime de Visual C++ junto al .exe"
      VERBATIM
    )
  else()
    message(WARNING "No se encontraron las DLL del runtime de MSVC; copialas a mano junto al .exe")
  endif()
endif()

if (WIN32 AND TARGET Qt${QT_VERSION_MAJOR}::qmake)
  get_target_property(_qmake Qt${QT_VERSION_MAJOR}::qmake IMPORTED_LOCATION)
  get_filename_component(_qtbin "${_qmake}" DIRECTORY)
  find_program(WINDEPLOYQT windeployqt HINTS "${_qtbin}")
  if (WINDEPLOYQT)
    # Copia las DLL de Qt (y sus plugins) junto al .exe
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
      COMMAND "${WINDEPLOYQT}" --no-translations --no-compiler-runtime --no-system-d3d-compiler
              --no-opengl-sw --verbose 0 "$<TARGET_FILE:${PROJECT_NAME}>"
      COMMENT "Copiando las DLL de Qt junto al .exe (windeployqt)"
      VERBATIM
    )
  else()
    message(WARNING "windeployqt no encontrado: la carpeta del .exe no llevara las DLL de Qt")
  endif()
endif()

# El paquete .tpp es la carpeta del .exe (ya desplegada) sin ficheros de depuracion
add_custom_target(tpp
  COMMAND ${CMAKE_COMMAND} -E rm -rf "${PKG_ROOT}"
  COMMAND ${CMAKE_COMMAND} -E copy_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>" "${PKG_DIR}"
  COMMAND ${CMAKE_COMMAND} -DDIR=${PKG_DIR} -P "${CMAKE_SOURCE_DIR}/tools/clean_package.cmake"
  COMMAND ${CMAKE_COMMAND} -E chdir "${PKG_ROOT}" ${CMAKE_COMMAND} -E tar cf FalconBMS-TP.tpp --format=zip FalconBMS-TP
  DEPENDS ${PROJECT_NAME}
  COMMENT "Creando ${PKG_ROOT}/FalconBMS-TP.tpp"
  VERBATIM
)
