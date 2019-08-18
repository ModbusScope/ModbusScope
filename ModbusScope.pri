
VERSION = 1.7.0

DEFINES += DEBUG
DEFINES += nVERBOSE

DEFINES += GIT_HASH="\\\"$(shell git --git-dir \""$$PWD/.git"\" rev-parse --short HEAD)\\\""
DEFINES += GIT_BRANCH="\\\"$(shell git --git-dir \""$$PWD/.git"\" rev-parse --abbrev-ref HEAD)\\\""

DEFINES += APP_VERSION=\\\"$$VERSION\\\"

QT += core xml network printsupport serialbus
QT += gui widgets

CONFIG += c++11

INCLUDEPATH += \
    $$PWD/src/libraries/qcustomplot \
    $$PWD/src/communication \
    $$PWD/src/customwidgets \
    $$PWD/src/importexport \
    $$PWD/src/dialogs \
    $$PWD/src/graphview \
    $$PWD/src/models \
    $$PWD/src/util

SOURCES +=  \
    $$PWD/src/importexport/presetparser.cpp \
    $$PWD/src/libraries/qcustomplot/qcustomplot.cpp \
    $$PWD/src/communication/communicationmanager.cpp \
    $$PWD/src/communication/modbusmaster.cpp \
    $$PWD/src/importexport/datafileexporter.cpp \
    $$PWD/src/importexport/datafileparser.cpp \
    $$PWD/src/importexport/projectfileparser.cpp \
    $$PWD/src/dialogs/connectiondialog.cpp \
    $$PWD/src/dialogs/logdialog.cpp \
    $$PWD/src/dialogs/mainwindow.cpp \
    $$PWD/src/graphview/basicgraphview.cpp \
    $$PWD/src/graphview/extendedgraphview.cpp \
    $$PWD/src/models/errorlogfilter.cpp \
    $$PWD/src/models/guimodel.cpp \
    $$PWD/src/models/settingsmodel.cpp \
    $$PWD/src/dialogs/aboutdialog.cpp \
    $$PWD/src/util/versiondownloader.cpp \
    $$PWD/src/util/updatenotify.cpp \
    $$PWD/src/graphview/myqcustomplot.cpp \
    $$PWD/src/util/util.cpp \
    $$PWD/src/importexport/settingsauto.cpp \
    $$PWD/src/models/graphdatamodel.cpp \
    $$PWD/src/models/graphdata.cpp \
    $$PWD/src/communication/modbusresult.cpp \
    $$PWD/src/customwidgets/legend.cpp \
    $$PWD/src/dialogs/registerdialog.cpp \
    $$PWD/src/dialogs/registerconndelegate.cpp \
    $$PWD/src/customwidgets/verticalscrollareacontents.cpp \
    $$PWD/src/customwidgets/markerinfo.cpp \
    $$PWD/src/customwidgets/markerinfoitem.cpp \
    $$PWD/src/importexport/projectfileexporter.cpp \
    $$PWD/src/dialogs/markerinfodialog.cpp \
    $$PWD/src/graphview/myqcpaxistickertime.cpp \
    $$PWD/src/graphview/myqcpaxis.cpp \
    $$PWD/src/dialogs/importmbcdialog.cpp \
    $$PWD/src/importexport/mbcfileimporter.cpp \
    $$PWD/src/models/errorlog.cpp \
    $$PWD/src/models/errorlogmodel.cpp \
    $$PWD/src/dialogs/errorlogdialog.cpp \
    $$PWD/src/models/notemodel.cpp \
    $$PWD/src/models/note.cpp \
    $$PWD/src/customwidgets/notesdock.cpp \
    $$PWD/src/customwidgets/notesdockwidget.cpp \
    $$PWD/src/importexport/mbcregisterdata.cpp \
    $$PWD/src/models/mbcregistermodel.cpp \
    $$PWD/src/models/mbcregisterfilter.cpp \
    $$PWD/src/util/scopelogging.cpp \
    $$PWD/src/communication/modbusconnection.cpp \
    $$PWD/src/communication/readregisters.cpp \
    $$PWD/src/importexport/datafilehandler.cpp \
    $$PWD/src/importexport/projectfilehandler.cpp \
    $$PWD/src/models/dataparsermodel.cpp \
    $$PWD/src/dialogs/loadfiledialog.cpp

FORMS    += \
    $$PWD/src/dialogs/connectiondialog.ui \
    $$PWD/src/dialogs/logdialog.ui \
    $$PWD/src/dialogs/mainwindow.ui \
    $$PWD/src/dialogs/registerdialog.ui \
    $$PWD/src/dialogs/aboutdialog.ui \
    $$PWD/src/dialogs/markerinfodialog.ui \
    $$PWD/src/dialogs/importmbcdialog.ui \
    $$PWD/src/dialogs/errorlogdialog.ui \
    $$PWD/src/customwidgets/notesdockwidget.ui \
    $$PWD/src/dialogs/loadfiledialog.ui

HEADERS += \
    $$PWD/src/importexport/presetparser.h \
    $$PWD/src/libraries/qcustomplot/qcustomplot.h \
    $$PWD/src/communication/communicationmanager.h \
    $$PWD/src/communication/modbusmaster.h \
    $$PWD/src/importexport/datafileexporter.h \
    $$PWD/src/importexport/datafileparser.h \
    $$PWD/src/importexport/projectfileparser.h \
    $$PWD/src/dialogs/connectiondialog.h \
    $$PWD/src/dialogs/logdialog.h \
    $$PWD/src/dialogs/mainwindow.h \
    $$PWD/src/dialogs/registerdialog.h \
    $$PWD/src/dialogs/registerconndelegate.h \
    $$PWD/src/graphview/basicgraphview.h \
    $$PWD/src/graphview/extendedgraphview.h \
    $$PWD/src/models/errorlogfilter.h \
    $$PWD/src/models/guimodel.h \
    $$PWD/src/models/settingsmodel.h \
    $$PWD/src/util/util.h \
    $$PWD/src/dialogs/aboutdialog.h \
    $$PWD/src/util/versiondownloader.h \
    $$PWD/src/util/updatenotify.h \
    $$PWD/src/graphview/myqcustomplot.h \
    $$PWD/src/importexport/settingsauto.h \
    $$PWD/src/models/graphdatamodel.h \
    $$PWD/src/models/graphdata.h \
    $$PWD/src/communication/modbusresult.h \
    $$PWD/src/customwidgets/legend.h \
    $$PWD/src/customwidgets/verticalscrollareacontents.h \
    $$PWD/src/customwidgets/markerinfo.h \
    $$PWD/src/customwidgets/markerinfoitem.h \
    $$PWD/src/importexport/projectfiledefinitions.h \
    $$PWD/src/importexport/projectfileexporter.h \
    $$PWD/src/dialogs/markerinfodialog.h \
    $$PWD/src/graphview/myqcpaxistickertime.h \
    $$PWD/src/graphview/myqcpaxis.h \
    $$PWD/src/dialogs/importmbcdialog.h \
    $$PWD/src/importexport/mbcfileimporter.h \
    $$PWD/src/models/errorlog.h \
    $$PWD/src/models/errorlogmodel.h \
    $$PWD/src/dialogs/errorlogdialog.h \
    $$PWD/src/models/notemodel.h \
    $$PWD/src/models/note.h \
    $$PWD/src/customwidgets/notesdock.h \
    $$PWD/src/customwidgets/notesdockwidget.h \
    $$PWD/src/importexport/mbcregisterdata.h \
    $$PWD/src/models/mbcregistermodel.h \
    $$PWD/src/models/mbcregisterfilter.h \
    $$PWD/src/util/scopelogging.h \
    $$PWD/src/communication/modbusconnection.h \
    $$PWD/src/communication/readregisters.h \
    $$PWD/src/importexport/datafilehandler.h \
    $$PWD/src/importexport/projectfilehandler.h \
    $$PWD/src/models/dataparsermodel.h \
    $$PWD/src/dialogs/loadfiledialog.h

RESOURCES += \
    $$PWD/src/resources/resource.qrc

