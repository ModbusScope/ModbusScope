
find_package(Qt5 COMPONENTS
    Test

    REQUIRED
)

include_directories(${Qt5test_INCLUDE_DIRS})

SET(TEST_DATA_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/mbctestdata.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/testdata.cpp
)

include_directories(
    ${QT_INCLUDES}
)

enable_testing()

add_executable(tst_datafileparser ${CMAKE_CURRENT_SOURCE_DIR}/testdatafileparser.cpp ${TEST_DATA_SRCS})
add_executable(tst_mbcfileimporter ${CMAKE_CURRENT_SOURCE_DIR}/testmbcfileimporter.cpp ${TEST_DATA_SRCS})
add_executable(tst_settingsauto ${CMAKE_CURRENT_SOURCE_DIR}/testsettingsauto.cpp ${TEST_DATA_SRCS})

add_test(tst_datafileparser COMMAND tst_datafileparser)
add_test(tst_mbcfileimporter COMMAND tst_mbcfileimporter)
add_test(tst_settingsauto COMMAND tst_settingsauto)


target_link_libraries(tst_datafileparser  PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
    Qt5::Test
)

target_link_libraries(tst_mbcfileimporter  PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
    Qt5::Test
)

target_link_libraries(tst_settingsauto  PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
    Qt5::Test
)


