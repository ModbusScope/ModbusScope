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

VERSION = 0.9.1

DEFINES += DEBUG

DEFINES += GIT_HASH="\\\"$(shell git --git-dir \""$$PWD/../.git"\" rev-parse --short HEAD)\\\""
DEFINES += GIT_BRANCH="\\\"$(shell git --git-dir \""$$PWD/../.git"\" rev-parse --abbrev-ref HEAD)\\\""

DEFINES += APP_VERSION=\\\"$$VERSION\\\"

DEFINES += QT_DEBUG_OUTPUT

INCLUDEPATH += \
    ../libraries/libmodbus/src \
    ../libraries/qcustomplot \
    ../src/communication \
    ../src/importexport \
    ../src/dialogs \
    ../src/graphview \
    ../src/models \
    ../src/util

SOURCES +=  \
    ../libraries/libmodbus/src/modbus-data.c \
    ../libraries/libmodbus/src/modbus.c \
    ../libraries/libmodbus/src/modbus-tcp.c \
    ../libraries/qcustomplot/qcustomplot.cpp \
    ../src/communication/communicationmanager.cpp \
    ../src/communication/modbusmaster.cpp \
    ../src/importexport/datafileexporter.cpp \
    ../src/importexport/datafileparser.cpp \
    ../src/importexport/projectfileparser.cpp \
    ../src/importexport/registerdata.cpp \
    ../src/dialogs/connectiondialog.cpp \
    ../src/dialogs/logdialog.cpp \
    ../src/dialogs/mainwindow.cpp \
    ../src/dialogs/registerdialog.cpp \
    ../src/graphview/basicgraphview.cpp \
    ../src/graphview/extendedgraphview.cpp \
    ../src/models/guimodel.cpp \
    ../src/models/registermodel.cpp \
    ../src/models/settingsmodel.cpp \
    ../src/main.cpp \
    ../src/dialogs/aboutdialog.cpp

FORMS    += \
    ../src/dialogs/connectiondialog.ui \
    ../src/dialogs/logdialog.ui \
    ../src/dialogs/mainwindow.ui \
    ../src/dialogs/registerdialog.ui \
    ../src/dialogs/aboutdialog.ui

HEADERS += \
    ../libraries/libmodbus/src/modbus-private.h \
    ../libraries/libmodbus/src/modbus-version.h \
    ../libraries/libmodbus/src/modbus.h \
    ../libraries/libmodbus/src/modbus-tcp.h \
    ../libraries/libmodbus/src/modbus-tcp-private.h \
    ../libraries/qcustomplot/qcustomplot.h \
    ../src/communication/communicationmanager.h \
    ../src/communication/modbusmaster.h \
    ../src/importexport/datafileexporter.h \
    ../src/importexport/datafileparser.h \
    ../src/importexport/projectfileparser.h \
    ../src/importexport/registerdata.h \
    ../src/dialogs/connectiondialog.h \
    ../src/dialogs/logdialog.h \
    ../src/dialogs/mainwindow.h \
    ../src/dialogs/registerdialog.h \
    ../src/graphview/basicgraphview.h \
    ../src/graphview/extendedgraphview.h \
    ../src/models/graphdata.h \
    ../src/models/guimodel.h \
    ../src/models/registermodel.h \
    ../src/models/settingsmodel.h \
    ../src/util/util.h \
    ../src/dialogs/aboutdialog.h

RESOURCES += \
    ../resources/resource.qrc
