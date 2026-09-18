# Define imported executable targets for the adapter binaries.
# The pre-built binaries are committed directly to this directory, one subfolder per adapter.
#
# Layout expected:
#   adapters/Modbus/modbusadapter          (Linux)
#   adapters/Modbus/dummymodbusadapter     (Linux)
#   adapters/Modbus/modbusadapter.exe      (Windows)
#   adapters/Modbus/dummymodbusadapter.exe (Windows)

if(WIN32)
    set(_adapter_suffix ".exe")
else()
    set(_adapter_suffix "")
endif()

add_executable(modbusadapter IMPORTED GLOBAL)
set_target_properties(modbusadapter PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/Modbus/modbusadapter${_adapter_suffix}"
)

# dummymodbusadapter is a test-harness stub of the Modbus protocol (used only by
# tests/ProtocolAdapter and tests/integration); it is never copied to the app output
# directory and is not user-selectable.
add_executable(dummymodbusadapter IMPORTED GLOBAL)
set_target_properties(dummymodbusadapter PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/Modbus/dummymodbusadapter${_adapter_suffix}"
)
