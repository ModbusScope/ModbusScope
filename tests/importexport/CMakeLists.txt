
SET(TEST_DATA_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/mbctestdata.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/csvdata.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/presetfiletestdata.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/projectfiletestdata.cpp
)

add_executable(tst_datafileparser ${CMAKE_CURRENT_SOURCE_DIR}/tst_datafileparser.cpp ${TEST_DATA_SRCS})
add_executable(tst_mbcfileimporter ${CMAKE_CURRENT_SOURCE_DIR}/tst_mbcfileimporter.cpp ${TEST_DATA_SRCS})
add_executable(tst_mbcregisterfilter ${CMAKE_CURRENT_SOURCE_DIR}/tst_mbcregisterfilter.cpp ${TEST_DATA_SRCS})
add_executable(tst_presethandler ${CMAKE_CURRENT_SOURCE_DIR}/tst_presethandler.cpp ${TEST_DATA_SRCS} ${GOOGLE_TEST_SOURCE})
add_executable(tst_presetparser ${CMAKE_CURRENT_SOURCE_DIR}/tst_presetparser.cpp ${TEST_DATA_SRCS})
add_executable(tst_projectfileparser ${CMAKE_CURRENT_SOURCE_DIR}/tst_projectfileparser.cpp ${TEST_DATA_SRCS})
add_executable(tst_settingsauto ${CMAKE_CURRENT_SOURCE_DIR}/tst_settingsauto.cpp ${TEST_DATA_SRCS})
add_executable(tst_updateregisteroperations ${CMAKE_CURRENT_SOURCE_DIR}/tst_updateregisteroperations.cpp)
add_executable(tst_updateregisternewexpression ${CMAKE_CURRENT_SOURCE_DIR}/tst_updateregisternewexpression.cpp)


add_test(NAME tst_datafileparser COMMAND tst_datafileparser)
add_test(NAME tst_mbcfileimporter COMMAND tst_mbcfileimporter)
add_test(NAME tst_mbcregisterfilter COMMAND tst_mbcregisterfilter)
add_test(NAME tst_presethandler COMMAND tst_presethandler)
add_test(NAME tst_presetparser COMMAND tst_presetparser)
add_test(NAME tst_projectfileparser COMMAND tst_projectfileparser)
add_test(NAME tst_settingsauto COMMAND tst_settingsauto)
add_test(NAME tst_updateregisteroperations COMMAND tst_updateregisteroperations)
add_test(NAME tst_updateregisternewexpression COMMAND tst_updateregisternewexpression)


target_link_libraries(tst_datafileparser PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
    Qt::Test
)

target_link_libraries(tst_mbcfileimporter PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
    Qt::Test
)

target_link_libraries(tst_mbcregisterfilter PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
    Qt::Test
)

target_link_libraries(tst_presethandler PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
    Qt::Test
    Threads::Threads
)

target_link_libraries(tst_presetparser PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
    Qt::Test
)

target_link_libraries(tst_projectfileparser PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
    Qt::Test
)

target_link_libraries(tst_settingsauto PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
    Qt::Test
)

target_link_libraries(tst_updateregisteroperations PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
    Qt::Test
)

target_link_libraries(tst_updateregisternewexpression PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
    Qt::Test
)
