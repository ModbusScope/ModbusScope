add_executable(tst_dummyadapter
    tst_dummyadapter.cpp
)

target_compile_definitions(tst_dummyadapter PRIVATE
    DUMMY_ADAPTER_EXECUTABLE="$<TARGET_FILE:dummymodbusadapter>"
)

target_link_libraries(tst_dummyadapter Qt::Test ${QT_LIB} ${SCOPESOURCE})

add_test(NAME tst_dummyadapter COMMAND tst_dummyadapter)

if(UNIX AND NOT APPLE)
    # Derive Qt library path so tests can find Qt shared libraries when run
    # directly from the build tree before a full install (RPATH is $ORIGIN).
    get_filename_component(_qt_lib_dir "${Qt6Core_DIR}/../.." ABSOLUTE)
    set_tests_properties(tst_dummyadapter PROPERTIES
        ENVIRONMENT "LD_LIBRARY_PATH=${_qt_lib_dir}:$ENV{LD_LIBRARY_PATH}"
    )
endif()
