# Define imported executable targets for the adapter binaries.
# The pre-built binaries are committed directly to this directory, one subfolder per adapter.
#
# Layout expected:
#   adapters/Modbus/modbusadapter          (Linux)
#   adapters/Modbus/dummymodbusadapter     (Linux)
#   adapters/Modbus/modbusadapter.exe      (Windows)
#   adapters/Modbus/dummymodbusadapter.exe (Windows)
#   adapters/IEC104/iec104adapter          (Linux)
#   adapters/IEC104/dummyiec104adapter     (Linux)
#   adapters/IEC104/iec104adapter.exe      (Windows)
#   adapters/IEC104/dummyiec104adapter.exe (Windows)
#   adapters/Dummy/dummyadapter            (Linux)
#   adapters/Dummy/dummyadapter.exe        (Windows)

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
# directory and is not user-selectable. Not to be confused with dummyadapter below.
add_executable(dummymodbusadapter IMPORTED GLOBAL)
set_target_properties(dummymodbusadapter PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/Modbus/dummymodbusadapter${_adapter_suffix}"
)

add_executable(iec104adapter IMPORTED GLOBAL)
set_target_properties(iec104adapter PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/IEC104/iec104adapter${_adapter_suffix}"
)

# dummyiec104adapter is a test-harness stub of the IEC104 protocol (used only by
# tests/ProtocolAdapter and tests/integration); it is never copied to the app output
# directory and is not user-selectable. Not to be confused with dummyadapter below.
add_executable(dummyiec104adapter IMPORTED GLOBAL)
set_target_properties(dummyiec104adapter PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/IEC104/dummyiec104adapter${_adapter_suffix}"
)

# dummyadapter is a real, user-selectable protocol adapter (AdapterDiscovery picks it up as
# id "dummy") used to exercise the app without hardware. It is dev/test-only: built into the
# local dev output directory so it is discoverable for manual testing, but never bundled into
# release/deploy artifacts.
add_executable(dummyadapter IMPORTED GLOBAL)
set_target_properties(dummyadapter PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/Dummy/dummyadapter${_adapter_suffix}"
)
