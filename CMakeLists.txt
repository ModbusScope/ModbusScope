cmake_minimum_required(VERSION 3.16)

project(ModbusScopeProject)

set(SCOPESOURCE ScopeSource)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS
    Widgets
    Xml
    Network
    SerialBus
    SerialPort

    REQUIRED
)

set(QT_LIB
    Qt::Widgets
    Qt::Xml
    Qt::Network
    Qt::SerialBus
    Qt::SerialPort
)

# Enable _UNICODE build of MuParser on Linux
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    add_definitions(-D_UNICODE)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/communication
    ${CMAKE_CURRENT_SOURCE_DIR}/src/customwidgets
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graphview
    ${CMAKE_CURRENT_SOURCE_DIR}/src/importexport
    ${CMAKE_CURRENT_SOURCE_DIR}/src/models
    ${CMAKE_CURRENT_SOURCE_DIR}/src/util

    ${CMAKE_CURRENT_SOURCE_DIR}/libraries/qcustomplot
    ${CMAKE_CURRENT_SOURCE_DIR}/libraries/muparser/include
)

enable_testing()

add_subdirectory(libraries)
add_subdirectory(src)
add_subdirectory(tests)
