cmake_minimum_required(VERSION 3.19)

project(notonlymodbusscope
    LANGUAGES CXX
    DESCRIPTION "Create logs from value read via Modbus"
    VERSION 0.0.1
)

set(SCOPESOURCE ScopeSource)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTORCC ON)

option(USE_GCOV "Enable gcov support" OFF)

# Find the QtCore library
find_package(QT NAMES Qt6 COMPONENTS Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)

message(STATUS "Using Qt${QT_VERSION_MAJOR} version ${Qt${QT_VERSION_MAJOR}Core_VERSION}")

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS
    Widgets
    Xml
    Network
    PrintSupport
    SerialBus
    SerialPort

    REQUIRED
)

qt_standard_project_setup()

set(QT_LIB
    Qt::Widgets
    Qt::Xml
    Qt::Network
    Qt::PrintSupport
    Qt::SerialBus
    Qt::SerialPort
)

add_compile_options(-Wall -Wextra -Werror)
if (MINGW)
    add_compile_options(-Wa,-mbig-obj)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # Code Coverage Report
    if(USE_GCOV)
        message(STATUS "Enabling coverage")
        add_compile_options(--coverage)
        add_link_options(--coverage)
    endif()
endif()

enable_testing()

add_subdirectory(libraries)
add_subdirectory(adapters)
add_subdirectory(src)
add_subdirectory(tests)
