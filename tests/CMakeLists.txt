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
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks/gmockutils.cpp
)

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS
    Test
    REQUIRED
)

function(add_xtest SOURCE_NAME)
	add_executable(${SOURCE_NAME}
        ${SOURCE_NAME}.cpp
		${ARGV1}
		${ARGV2})
	target_link_libraries(${SOURCE_NAME} Qt::Test ${QT_LIB} ${SCOPESOURCE})
	add_test(NAME ${SOURCE_NAME} COMMAND ${SOURCE_NAME})
endfunction()

add_subdirectory(communication)
add_subdirectory(importexport)
add_subdirectory(models)
add_subdirectory(util)


