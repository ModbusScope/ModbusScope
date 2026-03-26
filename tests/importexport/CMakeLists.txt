
add_xtest(tst_datafileparser ${CMAKE_CURRENT_SOURCE_DIR}/csvdata.cpp)
add_xtest_mock(tst_presethandler)
add_xtest(tst_presetparser ${CMAKE_CURRENT_SOURCE_DIR}/presetfiletestdata.cpp)
add_xtest(tst_projectfileparser ${CMAKE_CURRENT_SOURCE_DIR}/projectfiletestdata.cpp)
add_xtest(tst_projectfilejsonparser ${CMAKE_CURRENT_SOURCE_DIR}/projectfilejsontestdata.cpp)
add_xtest(tst_settingsauto ${CMAKE_CURRENT_SOURCE_DIR}/csvdata.cpp)
