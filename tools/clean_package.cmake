# Quita de la carpeta del paquete lo que no hace falta para ejecutar el plugin
file(GLOB _junk "${DIR}/*.pdb" "${DIR}/*.ilk" "${DIR}/*.exp" "${DIR}/*.lib" "${DIR}/*.log" "${DIR}/*.manifest")
if (_junk)
  file(REMOVE ${_junk})
endif()
