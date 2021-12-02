
find_package(Qt5 COMPONENTS
    Test

    REQUIRED
)

include_directories(${Qt5test_INCLUDE_DIRS})

SET(TEST_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/../testslave/testslavedata.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../testslave/testslavemodbus.cpp
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../testslave
    ${QT_INCLUDES}
)

enable_testing()

#add_executable(tst_communicationmanager ${CMAKE_CURRENT_SOURCE_DIR}/testcommunicationmanager.cpp ${TEST_SRCS})
add_executable(tst_modbusconnection ${CMAKE_CURRENT_SOURCE_DIR}/testmodbusconnection.cpp ${TEST_SRCS})
add_executable(tst_modbusmaster ${CMAKE_CURRENT_SOURCE_DIR}/testmodbusmaster.cpp ${TEST_SRCS})
add_executable(tst_registervaluehandler ${CMAKE_CURRENT_SOURCE_DIR}/testregistervaluehandler.cpp ${TEST_SRCS})
add_executable(tst_readregisters ${CMAKE_CURRENT_SOURCE_DIR}/testreadregisters.cpp ${TEST_SRCS})
add_executable(tst_expressionparser ${CMAKE_CURRENT_SOURCE_DIR}/testexpressionparser.cpp ${TEST_SRCS})

#add_test(NAME tst_communicationmanager COMMAND tst_communicationmanager)
add_test(NAME tst_modbusconnection COMMAND tst_modbusconnection)
add_test(NAME tst_modbusmaster COMMAND tst_modbusmaster)
add_test(NAME tst_registervaluehandler COMMAND tst_registervaluehandler)
add_test(NAME tst_readregisters COMMAND tst_readregisters)
add_test(NAME tst_expressionparser COMMAND tst_expressionparser)

#target_link_libraries(tst_communicationmanager PRIVATE
#    ${SCOPESOURCE}
#    ${QT_LIB}
#    Qt5::Test
#)

target_link_libraries(tst_modbusconnection PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
    Qt5::Test
)

target_link_libraries(tst_modbusmaster PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
    Qt5::Test
)

target_link_libraries(tst_registervaluehandler PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
    Qt5::Test
)

target_link_libraries(tst_readregisters PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
    Qt5::Test
)

target_link_libraries(tst_expressionparser PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
    Qt5::Test
)
