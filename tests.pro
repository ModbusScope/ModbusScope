include(ModbusScope.pro)

GOOGLETEST_DIR = $$PWD/tests/googletest
include(tests/gtest_dependency.pri)

QT += core gui widgets
QT += testlib

TARGET = ModbusScopeTests
TEMPLATE = app
CONFIG += console c++11
CONFIG += thread

HEADERS += \
    tests/tst_errorlog.h \
    tests/tst_errorlogmodel.h \
    tests/mockgraphdatamodel.h \
    tests/tst_mbcregistermodel.h

# Remove application main
SOURCES -= \
        src/main.cpp
        
# Add test main
SOURCES += \
        tests/main.cpp
