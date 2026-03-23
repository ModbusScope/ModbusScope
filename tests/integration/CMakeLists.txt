set(DUMMY_ADAPTER_PATH "${CMAKE_SOURCE_DIR}/adapters/dummymodbusadapter")

add_executable(tst_dummyadapter
    tst_dummyadapter.cpp
)

target_compile_definitions(tst_dummyadapter PRIVATE
    DUMMY_ADAPTER_PATH="${DUMMY_ADAPTER_PATH}"
)

target_link_libraries(tst_dummyadapter Qt::Test ${QT_LIB} ${SCOPESOURCE})

add_test(NAME tst_dummyadapter COMMAND tst_dummyadapter)
