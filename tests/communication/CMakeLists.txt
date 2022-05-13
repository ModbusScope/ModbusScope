
SET(TEST_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/../testslave/testslavedata.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../testslave/testslavemodbus.cpp
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../testslave
)

add_executable(tst_modbuspoll ${CMAKE_CURRENT_SOURCE_DIR}/tst_modbuspoll.cpp ${TEST_SRCS})
add_executable(tst_communication ${CMAKE_CURRENT_SOURCE_DIR}/tst_communication.cpp ${TEST_SRCS})
add_executable(tst_graphdatahandler ${CMAKE_CURRENT_SOURCE_DIR}/tst_graphdatahandler.cpp ${TEST_SRCS})
add_executable(tst_modbusconnection ${CMAKE_CURRENT_SOURCE_DIR}/tst_modbusconnection.cpp ${TEST_SRCS})
add_executable(tst_modbusmaster ${CMAKE_CURRENT_SOURCE_DIR}/tst_modbusmaster.cpp ${TEST_SRCS})
add_executable(tst_registervaluehandler ${CMAKE_CURRENT_SOURCE_DIR}/tst_registervaluehandler.cpp ${TEST_SRCS})
add_executable(tst_readregisters ${CMAKE_CURRENT_SOURCE_DIR}/tst_readregisters.cpp ${TEST_SRCS})
add_executable(tst_expressionparser ${CMAKE_CURRENT_SOURCE_DIR}/tst_expressionparser.cpp ${TEST_SRCS})

add_test(NAME tst_modbuspoll COMMAND tst_modbuspoll)
add_test(NAME tst_communication COMMAND tst_communication)
add_test(NAME tst_graphdatahandler COMMAND tst_graphdatahandler)
add_test(NAME tst_modbusconnection COMMAND tst_modbusconnection)
add_test(NAME tst_modbusmaster COMMAND tst_modbusmaster)
add_test(NAME tst_registervaluehandler COMMAND tst_registervaluehandler)
add_test(NAME tst_readregisters COMMAND tst_readregisters)
add_test(NAME tst_expressionparser COMMAND tst_expressionparser)

target_link_libraries(tst_modbuspoll PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
    Qt::Test
)

target_link_libraries(tst_communication PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
    Qt::Test
)

target_link_libraries(tst_graphdatahandler PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
    Qt::Test
)

target_link_libraries(tst_modbusconnection PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
    Qt::Test
)

target_link_libraries(tst_modbusmaster PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
    Qt::Test
)

target_link_libraries(tst_registervaluehandler PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
    Qt::Test
)

target_link_libraries(tst_readregisters PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
    Qt::Test
)

target_link_libraries(tst_expressionparser PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
    Qt::Test
)
