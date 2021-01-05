project(ModbusScope
    LANGUAGES CXX
    DESCRIPTION "Create logs from value read via Modbus"
    VERSION 3.0.0
    )

add_definitions("-DDEBUG")

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


FILE(GLOB_RECURSE SRCS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp") #all .cpp in current "src" folder and sub folders
FILE(GLOB_RECURSE UIS "${CMAKE_CURRENT_SOURCE_DIR}/*.ui") #all .ui in current "src" folder and sub folders


add_library (${SCOPESOURCE} ${SRCS} ${UIS})


# Default GUI type is blank
set(GUI_TYPE "")

if(WIN32)
    set(GUI_TYPE WIN32)
endif()

target_link_libraries(${SCOPESOURCE} qcustomplot muparser ${QT_LIB})

add_executable(${PROJECT_NAME} ${GUI_TYPE} ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../resources/resource.qrc ${CMAKE_CURRENT_SOURCE_DIR}/../resources/app_icon.rc)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
)

# Reset GUI type to blank
set(GUI_TYPE "")
