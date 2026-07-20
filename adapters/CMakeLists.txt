# Define imported executable targets for the ModbusAdapter and IEC104Adapter
# binaries. The pre-built binaries are committed directly to this directory.
#
# Layout expected:
#   adapters/modbusadapter          (Linux)
#   adapters/dummymodbusadapter     (Linux)
#   adapters/modbusadapter.exe      (Windows)
#   adapters/dummymodbusadapter.exe (Windows)
#   adapters/iec104adapter          (Linux)
#   adapters/dummyiec104adapter     (Linux)
#   adapters/iec104adapter.exe      (Windows)
#   adapters/dummyiec104adapter.exe (Windows)

if(WIN32)
    set(_adapter_suffix ".exe")
else()
    set(_adapter_suffix "")
endif()

add_executable(modbusadapter IMPORTED GLOBAL)
set_target_properties(modbusadapter PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/modbusadapter${_adapter_suffix}"
)

add_executable(dummymodbusadapter IMPORTED GLOBAL)
set_target_properties(dummymodbusadapter PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/dummymodbusadapter${_adapter_suffix}"
)

add_executable(iec104adapter IMPORTED GLOBAL)
set_target_properties(iec104adapter PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/iec104adapter${_adapter_suffix}"
)

add_executable(dummyiec104adapter IMPORTED GLOBAL)
set_target_properties(dummyiec104adapter PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/dummyiec104adapter${_adapter_suffix}"
)
