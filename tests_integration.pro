include(ModbusScope.pro)

QT += core gui widgets
QT += testlib

TARGET = ModbusScopeIntegrationTests
CONFIG += console c++11

# Remove application main
SOURCES -= \
        src/main.cpp

SOURCES += \
    tests_integration/testmodbusconnection.cpp \
    tests_integration/testslave/testslavedata.cpp \
    tests_integration/testslave/testslavemodbus.cpp

HEADERS += \
    tests_integration/testmodbusconnection.h \
    tests_integration/testslave/testslavedata.h \
    tests_integration/testslave/testslavemodbus.h
