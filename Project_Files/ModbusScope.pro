#-------------------------------------------------
#
# Project created by QtCreator 2013-12-13T22:02:01
#
#-------------------------------------------------

QT       += core gui xml network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = ModbusScope
TEMPLATE = app

CONFIG += c++11

win32 {
LIBS += -lws2_32
RC_ICONS = ../icon/application.ico
}

VERSION = 1.5.0

DEFINES += DEBUG

DEFINES += GIT_HASH="\\\"$(shell git --git-dir \""$$PWD/../.git"\" rev-parse --short HEAD)\\\""
DEFINES += GIT_BRANCH="\\\"$(shell git --git-dir \""$$PWD/../.git"\" rev-parse --abbrev-ref HEAD)\\\""

DEFINES += APP_VERSION=\\\"$$VERSION\\\"

DEFINES += QCUSTOMPLOT_USE_OPENGL

INCLUDEPATH += \
    ../libraries/libmodbus/src \
    ../libraries/qcustomplot \
    ../src/communication \
    ../src/customwidgets \
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
    ../src/dialogs/connectiondialog.cpp \
    ../src/dialogs/logdialog.cpp \
    ../src/dialogs/mainwindow.cpp \
    ../src/graphview/basicgraphview.cpp \
    ../src/graphview/extendedgraphview.cpp \
    ../src/models/guimodel.cpp \
    ../src/models/settingsmodel.cpp \
    ../src/main.cpp \
    ../src/dialogs/aboutdialog.cpp \
    ../src/util/versiondownloader.cpp \
    ../src/util/updatenotify.cpp \
    ../src/graphview/myqcustomplot.cpp \
    ../src/util/util.cpp \
    ../src/importexport/settingsauto.cpp \
    ../src/models/graphdatamodel.cpp \
    ../src/models/graphdata.cpp \
    ../src/communication/modbusresult.cpp \
    ../src/customwidgets/legend.cpp \
    ../src/dialogs/registerdialog.cpp \
    ../src/customwidgets/verticalscrollareacontents.cpp \
    ../src/customwidgets/markerinfo.cpp \
    ../src/customwidgets/markerinfoitem.cpp \
    ../src/importexport/projectfileexporter.cpp \
    ../src/dialogs/markerinfodialog.cpp \
    ../src/graphview/myqcpaxistickertime.cpp \
    ../src/graphview/myqcpaxis.cpp \
    ../src/dialogs/importmbcdialog.cpp \
    ../src/importexport/mbcfileimporter.cpp \
    ../src/models/errorlog.cpp \
    ../src/models/errorlogmodel.cpp \
    ../src/dialogs/errorlogdialog.cpp \
    ../src/models/notemodel.cpp \
    ../src/models/note.cpp \
    ../src/customwidgets/notesdock.cpp \
    ../src/customwidgets/notesdockwidget.cpp

FORMS    += \
    ../src/dialogs/connectiondialog.ui \
    ../src/dialogs/logdialog.ui \
    ../src/dialogs/mainwindow.ui \
    ../src/dialogs/registerdialog.ui \
    ../src/dialogs/aboutdialog.ui \
    ../src/dialogs/markerinfodialog.ui \
    ../src/dialogs/importmbcdialog.ui \
    ../src/dialogs/errorlogdialog.ui \
    ../src/customwidgets/notesdockwidget.ui

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
    ../src/dialogs/connectiondialog.h \
    ../src/dialogs/logdialog.h \
    ../src/dialogs/mainwindow.h \
    ../src/dialogs/registerdialog.h \
    ../src/graphview/basicgraphview.h \
    ../src/graphview/extendedgraphview.h \
    ../src/models/guimodel.h \
    ../src/models/settingsmodel.h \
    ../src/util/util.h \
    ../src/dialogs/aboutdialog.h \
    ../src/util/versiondownloader.h \
    ../src/util/updatenotify.h \
    ../src/graphview/myqcustomplot.h \
    ../src/importexport/settingsauto.h \
    ../src/models/graphdatamodel.h \
    ../src/models/graphdata.h \
    ../src/communication/modbusresult.h \
    ../src/customwidgets/legend.h \
    ../src/customwidgets/verticalscrollareacontents.h \
    ../src/customwidgets/markerinfo.h \
    ../src/customwidgets/markerinfoitem.h \
    ../src/importexport/projectfiledefinitions.h \
    ../src/importexport/projectfileexporter.h \
    ../src/dialogs/markerinfodialog.h \
    ../src/graphview/myqcpaxistickertime.h \
    ../src/graphview/myqcpaxis.h \
    ../src/dialogs/importmbcdialog.h \
    ../src/importexport/mbcfileimporter.h \
    ../src/models/errorlog.h \
    ../src/models/errorlogmodel.h \
    ../src/dialogs/errorlogdialog.h \
    ../src/models/notemodel.h \
    ../src/models/note.h \
    ../src/customwidgets/notesdock.h \
    ../src/customwidgets/notesdockwidget.h

RESOURCES += \
    ../resources/resource.qrc
