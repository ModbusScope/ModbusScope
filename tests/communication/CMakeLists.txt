
SET(TEST_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/../testslave/testslavedata.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../testslave/testslavemodbus.cpp
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../testslave
)

add_xtest(tst_modbuspoll ${TEST_SRCS})
add_xtest(tst_communication ${TEST_SRCS})
add_xtest(tst_graphdatahandler ${TEST_SRCS})
add_xtest(tst_modbusconnection ${TEST_SRCS})
add_xtest(tst_modbusmaster ${TEST_SRCS})
add_xtest(tst_registervaluehandler ${TEST_SRCS})
add_xtest(tst_readregisters ${TEST_SRCS})
add_xtest(tst_expressionparser ${TEST_SRCS})