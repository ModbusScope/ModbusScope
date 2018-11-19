include(ModbusScope.pri)

GOOGLETEST_DIR = $$PWD/tests_unit/googletest
include(tests_unit/gtest_dependency.pri)

QT += core gui widgets
QT += testlib

TARGET = ModbusScopeUnitTests
TEMPLATE = app
CONFIG += console c++11

HEADERS += \
    tests_unit/tst_errorlog.h \
    tests_unit/tst_errorlogmodel.h \
    tests_unit/mockgraphdatamodel.h \
    tests_unit/tst_mbcregistermodel.h \
    tests_unit/tst_readregisters.h

# Remove application main
SOURCES -= \
        src/main.cpp
        
# Add test main
SOURCES += \
        tests_unit/main.cpp
