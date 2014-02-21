#-------------------------------------------------
#
# Project created by QtCreator 2013-12-13T22:02:01
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = ModbusScope
TEMPLATE = app

INCLUDEPATH += \
    ../libraries/libmodbus/src \
    ../libraries/qcustomplot \
    ../src

SOURCES +=  \
    ../src/ModbusThread.cpp \
    ../src/mainwindow.cpp \
    ../src/main.cpp \
    ../src/ModbusMaster.cpp \
    ../libraries/libmodbus/src/modbus-data.c \
    ../libraries/libmodbus/src/modbus.c \
    ../libraries/libmodbus/src/modbus-tcp.c \
    ../libraries/qcustomplot/qcustomplot.cpp \

FORMS    += \
    ../src/mainwindow.ui

HEADERS += \
    ../src/mainwindow.h \
    ../src/ModbusMaster.h \
    ../src/ModbusThread.h \
    ../libraries/libmodbus/src/modbus-private.h \
    ../libraries/libmodbus/src/modbus-version.h \
    ../libraries/libmodbus/src/modbus.h \
    ../libraries/libmodbus/src/modbus-tcp.h \
    ../libraries/libmodbus/src/modbus-tcp-private.h \
    ../libraries/libmodbus/src/config.h \
    ../libraries/qcustomplot/qcustomplot.h \
