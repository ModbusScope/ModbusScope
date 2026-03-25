# Define imported executable targets for the ModbusAdapter binaries.
# The pre-built binaries are committed directly to this directory.
#
# Layout expected:
#   adapters/modbusadapter          (Linux)
#   adapters/dummymodbusadapter     (Linux)
#   adapters/modbusadapter.exe      (Windows)
#   adapters/dummymodbusadapter.exe (Windows)

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
