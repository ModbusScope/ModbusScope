add_xtest(tst_framingreader)
add_xtest(tst_adapterclient)
add_xtest(tst_adapterprocess)

target_compile_definitions(tst_adapterprocess PRIVATE
    DUMMY_ADAPTER_EXECUTABLE="$<TARGET_FILE:dummymodbusadapter>"
)

if(UNIX AND NOT APPLE)
    # Derive Qt library path so tests can find Qt shared libraries when run
    # directly from the build tree before a full install (RPATH is $ORIGIN).
    get_filename_component(_qt_lib_dir "${Qt6Core_DIR}/../.." ABSOLUTE)
    set_tests_properties(tst_adapterprocess PROPERTIES
        ENVIRONMENT "LD_LIBRARY_PATH=${_qt_lib_dir}:$ENV{LD_LIBRARY_PATH}"
    )
endif()
