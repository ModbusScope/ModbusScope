
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

add_test(tst_errorlog COMMAND tst_errorlog)

target_link_libraries(tst_errorlog  PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
    Qt5::Test
)