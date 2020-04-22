
add_definitions(-DGTEST_LANGUAGE_CXX11)

find_package(Threads REQUIRED)

if ($ENV{GOOGLETEST_DIR})
    set(GOOGLETEST_DIR $ENV{GOOGLETEST_DIR})
else ()
    message("Set local path")
    set(GOOGLETEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/googletest)
endif ()

if (EXISTS ${GOOGLETEST_DIR})
    set(GTestSrc ${GOOGLETEST_DIR}/googletest)
    set(GMockSrc ${GOOGLETEST_DIR}/googlemock)
else ()
    message( FATAL_ERROR "No googletest src dir found - set GOOGLETEST_DIR to enable!")
endif ()


include_directories(${GTestSrc} ${GTestSrc}/include ${GMockSrc} ${GMockSrc}/include)

set(
    GOOGLE_TEST_SOURCE
    ${GTestSrc}/src/gtest-all.cc
    ${GMockSrc}/src/gmock-all.cc
)


find_package(Qt5 COMPONENTS
    Test

    REQUIRED
)
include_directories(${Qt5test_INCLUDE_DIRS})


set(
    UNIT_TEST_SOURCES
    tst_mbcregistermodel.h
)

add_executable(test_unit main.cpp
    ${UNIT_TEST_SOURCES}
    ${GOOGLE_TEST_SOURCE}
)

add_test(test_unit COMMAND test_unit)

target_link_libraries(test_unit PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
    Qt5::Test
    Threads::Threads
)
