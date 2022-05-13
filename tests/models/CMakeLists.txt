

add_executable(tst_diagnostic ${CMAKE_CURRENT_SOURCE_DIR}/tst_diagnostic.cpp ${TEST_SRCS})
add_executable(tst_diagnosticmodel ${CMAKE_CURRENT_SOURCE_DIR}/tst_diagnosticmodel.cpp ${TEST_SRCS})
add_executable(tst_graphdata ${CMAKE_CURRENT_SOURCE_DIR}/tst_graphdata.cpp ${TEST_SRCS})
add_executable(tst_mbcregistermodel ${CMAKE_CURRENT_SOURCE_DIR}/tst_mbcregistermodel.cpp ${TEST_SRCS} ${GOOGLE_TEST_SOURCE})

add_test(NAME tst_diagnostic COMMAND tst_diagnostic)
add_test(NAME tst_diagnosticmodel COMMAND tst_diagnosticmodel)
add_test(NAME tst_graphdata COMMAND tst_graphdata)
add_test(NAME tst_mbcregistermodel COMMAND tst_mbcregistermodel)

target_link_libraries(tst_diagnostic  PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
    Qt::Test
)

target_link_libraries(tst_diagnosticmodel  PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
    Qt::Test
)

target_link_libraries(tst_graphdata  PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
    Qt::Test
)

target_link_libraries(tst_mbcregistermodel  PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
    Qt::Test
    Threads::Threads
)
