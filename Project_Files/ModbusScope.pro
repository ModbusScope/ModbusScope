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
RC_ICONS = ../icon/application.ico
}


VERSION = 0.7.3

DEFINES += APP_VERSION=\\\"$$VERSION\\\"
DEFINES += QT_DEBUG_OUTPUT

INCLUDEPATH += \
    ../libraries/libmodbus/src \
    ../libraries/qcustomplot \
    ../src

SOURCES +=  \
    ../src/mainwindow.cpp \
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
    ../src/datafileparser.cpp \
    ../src/registerdata.cpp \
    ../src/connectiondialog.cpp \
    ../src/registerdialog.cpp \
    ../src/connectionmodel.cpp

FORMS    += \
    ../src/mainwindow.ui \
    ../src/connectiondialog.ui \
    ../src/registerdialog.ui

HEADERS += \
    ../src/mainwindow.h \
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
    ../src/datafileparser.h \
    ../src/util.h \
    ../src/registerdata.h \
    ../src/connectiondialog.h \
    ../src/registerdialog.h \
    ../src/connectionmodel.h
