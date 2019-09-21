include(../ModbusScope.pri)

GOOGLETEST_DIR = $$PWD/googletest
include(gtest_dependency.pri)

QT += core gui widgets
QT += testlib

TARGET = ModbusScopeUnitTests
TEMPLATE = app
CONFIG += console c++11

HEADERS += \
    tst_errorlog.h \
    tst_errorlogmodel.h \
    mockgraphdatamodel.h \
    tst_mbcregistermodel.h \
    tst_readregisters.h \
    tst_graphdata.h

# Remove application main
SOURCES -= \
        ../src/main.cpp
        
# Add test main
SOURCES += \
        main.cpp
