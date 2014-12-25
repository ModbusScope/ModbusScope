#-------------------------------------------------
#
# Project created by QtCreator 2013-12-13T22:02:01
#
#-------------------------------------------------

QT       += core gui xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = ModbusScope
TEMPLATE = app

win32 {
LIBS += -lws2_32
}


VERSION = 0.5.1

DEFINES += APP_VERSION=\\\"$$VERSION\\\"
DEFINES += QT_DEBUG_OUTPUT

INCLUDEPATH += \
    ../libraries/libmodbus/src \
    ../libraries/qcustomplot \
    ../src

SOURCES +=  \
    ../src/mainwindow.cpp \
    ../src/modbussettings.cpp \
    ../src/main.cpp \
    ../src/modbusmaster.cpp \
    ../libraries/libmodbus/src/modbus-data.c \
    ../libraries/libmodbus/src/modbus.c \
    ../libraries/libmodbus/src/modbus-tcp.c \
    ../libraries/qcustomplot/qcustomplot.cpp \
    ../src/scopedata.cpp \
    ../src/scopegui.cpp \
    ../src/projectfileparser.cpp \
    ../src/registermodel.cpp \
    ../src/datafileparser.cpp

FORMS    += \
    ../src/mainwindow.ui

HEADERS += \
    ../src/mainwindow.h \
    ../src/modbussettings.h \
    ../src/modbusmaster.h \
    ../libraries/libmodbus/src/modbus-private.h \
    ../libraries/libmodbus/src/modbus-version.h \
    ../libraries/libmodbus/src/modbus.h \
    ../libraries/libmodbus/src/modbus-tcp.h \
    ../libraries/libmodbus/src/modbus-tcp-private.h \
    ../libraries/qcustomplot/qcustomplot.h \
    ../src/scopedata.h \
    ../src/scopegui.h \
    ../src/projectfileparser.h \
    ../src/registermodel.h \
    ../src/datafileparser.h
