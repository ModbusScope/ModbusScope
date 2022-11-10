cmake_minimum_required(VERSION 3.16)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS
    Widgets
    PrintSupport

    REQUIRED
)

set(QT_PLOT_LIB
    Qt6::Widgets
    Qt6::PrintSupport
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/qcustomplot
)

SET(PLOT_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/qcustomplot.cpp
)

add_library (qcustomplot ${PLOT_SRCS})

target_link_libraries(qcustomplot PUBLIC ${QT_PLOT_LIB})

