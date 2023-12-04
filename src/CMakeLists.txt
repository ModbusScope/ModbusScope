project(modbusscope
    LANGUAGES CXX
    DESCRIPTION "Create logs from value read via Modbus"
    VERSION 3.9.0
    )

add_definitions("-DnDEBUG")

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
    set(target_dir "bin/win")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(target_dir "bin/linux")
else()
    message(SEND_ERROR "You are on an unsupported platform! (Not Win or Unix)")
endif(WIN32)

add_executable(${PROJECT_NAME} ${GUI_TYPE} ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../resources.qrc ${CMAKE_CURRENT_SOURCE_DIR}/../resources/app_icon.rc)

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${target_dir} )

target_link_libraries(${SCOPESOURCE} qcustomplot muparser ${QT_LIB})

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
)

# Reset GUI type to blank
set(GUI_TYPE "")

# Do platform specific post target stuff
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

elseif(WIN32)
    # not required
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
