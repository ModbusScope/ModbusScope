include(../ModbusScope.pri)
include(testslave/testslave.pri)

QT -= core gui widgets
QT += testlib

CONFIG += console

# Remove application main
SOURCES -= \
        ../src/main.cpp


