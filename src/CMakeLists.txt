project(ModbusScope
    LANGUAGES CXX
    DESCRIPTION "Create logs from value read via Modbus"
    VERSION 2.0.0
    )

add_definitions("-DNDEBUG")

# Get the current working branch
execute_process(
  COMMAND git rev-parse --abbrev-ref HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_BRANCH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get the latest abbreviated commit hash of the working branch
execute_process(
  COMMAND git log -1 --format=%h
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/util/version.h.in ${CMAKE_CURRENT_SOURCE_DIR}/util/version.h)

SET(SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/graphview/graphviewzoom.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/importexport/presetparser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libraries/qcustomplot/qcustomplot.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/communication/communicationmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/communication/modbusmaster.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/importexport/datafileexporter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/importexport/datafileparser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/importexport/projectfileparser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/connectiondialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/logdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/mainwindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/graphview/basicgraphview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/graphview/extendedgraphview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/models/errorlogfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/models/guimodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/models/settingsmodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/aboutdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/util/versiondownloader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/util/updatenotify.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/graphview/myqcustomplot.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/util/util.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/importexport/settingsauto.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/models/graphdatamodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/models/graphdata.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/communication/modbusresult.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/customwidgets/legend.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/registerdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/registerconndelegate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/customwidgets/verticalscrollareacontents.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/customwidgets/markerinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/customwidgets/markerinfoitem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/importexport/projectfileexporter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/markerinfodialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/graphview/myqcpaxistickertime.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/graphview/myqcpaxis.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/importmbcdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/importexport/mbcfileimporter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/models/errorlog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/models/errorlogmodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/errorlogdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/models/notemodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/models/note.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/customwidgets/notesdock.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/customwidgets/notesdockwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/importexport/mbcregisterdata.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/models/mbcregistermodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/models/mbcregisterfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/util/scopelogging.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/communication/modbusconnection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/communication/readregisters.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/importexport/datafilehandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/importexport/projectfilehandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/models/dataparsermodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/loadfiledialog.cpp
)

SET(UIS
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/logdialog.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/mainwindow.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/aboutdialog.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/errorlogdialog.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/loadfiledialog.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/registerdialog.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/importmbcdialog.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/connectiondialog.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/markerinfodialog.ui
)

include_directories(
    ${QT_INCLUDES}
)

# Default GUI type is blank
set(GUI_TYPE "")

if(WIN32)
    set(GUI_TYPE WIN32)
endif()

add_library (${SCOPESOURCE} ${SRCS} ${UIS})
add_executable(${PROJECT_NAME} ${GUI_TYPE} ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp ${CMAKE_CURRENT_SOURCE_DIR}/resources/resource.qrc ${CMAKE_CURRENT_SOURCE_DIR}/resources/app_icon.rc)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
)

# Reset GUI type to blank
set(GUI_TYPE "")
