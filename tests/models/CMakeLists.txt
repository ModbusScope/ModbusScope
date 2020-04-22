
find_package(Qt5 COMPONENTS
    Test

    REQUIRED
)

include_directories(${Qt5test_INCLUDE_DIRS})

include_directories(
    ${QT_INCLUDES}
)

enable_testing()

add_executable(tst_errorlog ${CMAKE_CURRENT_SOURCE_DIR}/testerrorlog.cpp ${TEST_SRCS})
add_executable(tst_errorlogmodel ${CMAKE_CURRENT_SOURCE_DIR}/testerrorlogmodel.cpp ${TEST_SRCS})
add_executable(tst_graphdata ${CMAKE_CURRENT_SOURCE_DIR}/testgraphdata.cpp ${TEST_SRCS})
add_executable(tst_mbcregistermodel ${CMAKE_CURRENT_SOURCE_DIR}/testmbcregistermodel.cpp ${TEST_SRCS} ${GOOGLE_TEST_SOURCE})

add_test(tst_errorlog COMMAND tst_errorlog)
add_test(tst_errorlogmodel COMMAND tst_errorlogmodel)
add_test(tst_graphdata COMMAND tst_graphdata)
add_test(tst_mbcregistermodel COMMAND tst_mbcregistermodel)

target_link_libraries(tst_errorlog  PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
    Qt5::Test
)

target_link_libraries(tst_errorlogmodel  PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
    Qt5::Test
)

target_link_libraries(tst_graphdata  PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
    Qt5::Test
)

target_link_libraries(tst_mbcregistermodel  PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
    Qt5::Test
    Threads::Threads
)
