cmake_minimum_required(VERSION 3.19)

project(modbusscope
    LANGUAGES CXX
    DESCRIPTION "Create logs from value read via Modbus"
    VERSION 4.1.0
)

add_definitions("-DnDEBUG")

set(SCOPESOURCE ScopeSource)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/libraries
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
if (MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wa,-mbig-obj")
endif()


if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # Code Coverage Report
    if(USE_GCOV)
            message(STATUS "Enabling coverage")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
            set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} --coverage")
    endif()
endif()

enable_testing()

add_subdirectory(libraries)
add_subdirectory(src)
add_subdirectory(tests)
