include(ModbusScope.pro)

GOOGLETEST_DIR = $$PWD/tests/googletest
include(tests/gtest_dependency.pri)

QT += core gui widgets

TEMPLATE = app
CONFIG += console c++11
CONFIG += thread

HEADERS += \
    tests/tst_errorlog.h

# Remove application main
SOURCES -= \
        src/main.cpp
        
# Add test main
SOURCES += \
        tests/main.cpp
