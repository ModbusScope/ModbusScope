add_executable(tst_dummyadapter
    tst_dummyadapter.cpp
)

target_compile_definitions(tst_dummyadapter PRIVATE
    DUMMY_ADAPTER_EXECUTABLE="$<TARGET_FILE:dummymodbusadapter>"
)

target_link_libraries(tst_dummyadapter Qt::Test ${QT_LIB} ${SCOPESOURCE})

add_dependencies(tst_dummyadapter dummymodbusadapter)

add_test(NAME tst_dummyadapter COMMAND tst_dummyadapter)

add_executable(tst_dummydevicelimit
    tst_dummydevicelimit.cpp
)

target_compile_definitions(tst_dummydevicelimit PRIVATE
    DUMMY_STANDALONE_ADAPTER_EXECUTABLE="$<TARGET_FILE:dummyadapter>"
)

target_link_libraries(tst_dummydevicelimit Qt::Test ${QT_LIB} ${SCOPESOURCE})

add_dependencies(tst_dummydevicelimit dummyadapter)

add_test(NAME tst_dummydevicelimit COMMAND tst_dummydevicelimit)

add_executable(tst_dummyadapterstarterror
    tst_dummyadapterstarterror.cpp
)

target_compile_definitions(tst_dummyadapterstarterror PRIVATE
    DUMMY_STANDALONE_ADAPTER_EXECUTABLE="$<TARGET_FILE:dummyadapter>"
)

target_link_libraries(tst_dummyadapterstarterror Qt::Test ${QT_LIB} ${SCOPESOURCE})

add_dependencies(tst_dummyadapterstarterror dummyadapter)

add_test(NAME tst_dummyadapterstarterror COMMAND tst_dummyadapterstarterror)

add_executable(tst_dummyadapterconfigureerror
    tst_dummyadapterconfigureerror.cpp
)

target_compile_definitions(tst_dummyadapterconfigureerror PRIVATE
    DUMMY_STANDALONE_ADAPTER_EXECUTABLE="$<TARGET_FILE:dummyadapter>"
)

target_link_libraries(tst_dummyadapterconfigureerror Qt::Test ${QT_LIB} ${SCOPESOURCE})

add_dependencies(tst_dummyadapterconfigureerror dummyadapter)

add_test(NAME tst_dummyadapterconfigureerror COMMAND tst_dummyadapterconfigureerror)

add_executable(tst_dummyadapterquality
    tst_dummyadapterquality.cpp
)

target_compile_definitions(tst_dummyadapterquality PRIVATE
    DUMMY_STANDALONE_ADAPTER_EXECUTABLE="$<TARGET_FILE:dummyadapter>"
)

target_link_libraries(tst_dummyadapterquality Qt::Test ${QT_LIB} ${SCOPESOURCE})

add_dependencies(tst_dummyadapterquality dummyadapter)

add_test(NAME tst_dummyadapterquality COMMAND tst_dummyadapterquality)

add_executable(tst_legacymodbusconfig
    tst_legacymodbusconfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../importexport/legacy/projectfilexmltestdata.cpp
)

target_compile_definitions(tst_legacymodbusconfig PRIVATE
    MODBUS_ADAPTER_EXECUTABLE="$<TARGET_FILE:modbusadapter>"
)

target_link_libraries(tst_legacymodbusconfig Qt::Test ${QT_LIB} ${SCOPESOURCE})

add_dependencies(tst_legacymodbusconfig modbusadapter)

add_test(NAME tst_legacymodbusconfig COMMAND tst_legacymodbusconfig)

add_executable(tst_iec104adapter
    tst_iec104adapter.cpp
)

# Runs the shipped iec104adapter binary for the build platform (iec104adapter or iec104adapter.exe).
target_compile_definitions(tst_iec104adapter PRIVATE
    IEC104_ADAPTER_EXECUTABLE="$<TARGET_FILE:iec104adapter>"
)

target_link_libraries(tst_iec104adapter Qt::Test ${QT_LIB} ${SCOPESOURCE})

add_dependencies(tst_iec104adapter iec104adapter)

add_test(NAME tst_iec104adapter COMMAND tst_iec104adapter)

add_executable(tst_modbusadapter
    tst_modbusadapter.cpp
)

# Runs the shipped modbusadapter binary for the build platform (modbusadapter or modbusadapter.exe).
target_compile_definitions(tst_modbusadapter PRIVATE
    MODBUS_ADAPTER_EXECUTABLE="$<TARGET_FILE:modbusadapter>"
)

target_link_libraries(tst_modbusadapter Qt::Test ${QT_LIB} ${SCOPESOURCE})

add_dependencies(tst_modbusadapter modbusadapter)

add_test(NAME tst_modbusadapter COMMAND tst_modbusadapter)

if(UNIX AND NOT APPLE)
    # Derive Qt library path so tests can find Qt shared libraries when run
    # directly from the build tree before a full install (RPATH is $ORIGIN).
    get_filename_component(_qt_lib_dir "${Qt6Core_DIR}/../.." ABSOLUTE)
    set_tests_properties(tst_dummyadapter tst_iec104adapter tst_modbusadapter tst_legacymodbusconfig PROPERTIES
        ENVIRONMENT "LD_LIBRARY_PATH=${_qt_lib_dir}:$ENV{LD_LIBRARY_PATH}"
    )
endif()
