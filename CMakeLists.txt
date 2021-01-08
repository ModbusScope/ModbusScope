cmake_minimum_required(VERSION 3.5)

project(ModbusScopeProject)

set(SCOPESOURCE ScopeSource)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS
    Widgets
    Xml
    Network
    SerialBus

    REQUIRED
)

set(QT_INCLUDES
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5Xml_INCLUDE_DIRS}
    ${Qt5Network_INCLUDE_DIRS}
    ${Qt5SerialBus_INCLUDE_DIRS}
)

set(QT_LIB
    Qt5::Widgets
    Qt5::Xml
    Qt5::Network
    Qt5::SerialBus
)


include_directories(
    ${QT_INCLUDES}

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
