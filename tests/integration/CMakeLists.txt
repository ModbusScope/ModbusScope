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
    DUMMY_ADAPTER_EXECUTABLE="$<TARGET_FILE:dummymodbusadapter>"
)

target_link_libraries(tst_dummydevicelimit Qt::Test ${QT_LIB} ${SCOPESOURCE})

add_dependencies(tst_dummydevicelimit dummymodbusadapter)

add_test(NAME tst_dummydevicelimit COMMAND tst_dummydevicelimit)

add_executable(tst_dummyadapterstarterror
    tst_dummyadapterstarterror.cpp
)

target_compile_definitions(tst_dummyadapterstarterror PRIVATE
    DUMMY_ADAPTER_EXECUTABLE="$<TARGET_FILE:dummymodbusadapter>"
)

target_link_libraries(tst_dummyadapterstarterror Qt::Test ${QT_LIB} ${SCOPESOURCE})

add_dependencies(tst_dummyadapterstarterror dummymodbusadapter)

add_test(NAME tst_dummyadapterstarterror COMMAND tst_dummyadapterstarterror)

add_executable(tst_dummyadapterconfigureerror
    tst_dummyadapterconfigureerror.cpp
)

target_compile_definitions(tst_dummyadapterconfigureerror PRIVATE
    DUMMY_ADAPTER_EXECUTABLE="$<TARGET_FILE:dummymodbusadapter>"
)

target_link_libraries(tst_dummyadapterconfigureerror Qt::Test ${QT_LIB} ${SCOPESOURCE})

add_dependencies(tst_dummyadapterconfigureerror dummymodbusadapter)

add_test(NAME tst_dummyadapterconfigureerror COMMAND tst_dummyadapterconfigureerror)

add_executable(tst_modbusadapterquality
    tst_modbusadapterquality.cpp
)

target_compile_definitions(tst_modbusadapterquality PRIVATE
    DUMMY_ADAPTER_EXECUTABLE="$<TARGET_FILE:dummymodbusadapter>"
)

target_link_libraries(tst_modbusadapterquality Qt::Test ${QT_LIB} ${SCOPESOURCE})

add_dependencies(tst_modbusadapterquality dummymodbusadapter)

add_test(NAME tst_modbusadapterquality COMMAND tst_modbusadapterquality)

# Every test below starts a dummymodbusadapter, whose built-in TCP server listens on a fixed port,
# so they must not run at the same time (ctest -j).
set(_dummy_modbus_tests
    tst_dummyadapter
    tst_dummydevicelimit
    tst_dummyadapterstarterror
    tst_dummyadapterconfigureerror
    tst_modbusadapterquality
)
set_tests_properties(${_dummy_modbus_tests} PROPERTIES RESOURCE_LOCK dummymodbusadapter_port)

if(UNIX AND NOT APPLE)
    # Derive Qt library path so tests can find Qt shared libraries when run
    # directly from the build tree before a full install (RPATH is $ORIGIN).
    get_filename_component(_qt_lib_dir "${Qt6Core_DIR}/../.." ABSOLUTE)
    set_tests_properties(${_dummy_modbus_tests} PROPERTIES
        ENVIRONMENT "LD_LIBRARY_PATH=${_qt_lib_dir}:$ENV{LD_LIBRARY_PATH}"
    )
endif()
