
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
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../resources/modbusscope.rc.in ${CMAKE_CURRENT_SOURCE_DIR}/../resources/modbusscope.rc)

file(GLOB_RECURSE SRCS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h") #all .cpp/.h in current "src" folder and sub folders
file(GLOB_RECURSE UIS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.ui") #all .ui in current "src" folder and sub folders

qt_add_library(${SCOPESOURCE} STATIC ${SRCS} ${UIS})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${SCOPESOURCE} PRIVATE DEBUG)
endif()

target_include_directories(${SCOPESOURCE} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(${SCOPESOURCE} SYSTEM PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../libraries
)

# Default GUI type is blank
set(GUI_TYPE "")

if(WIN32)
    set(GUI_TYPE WIN32)
    set(target_dir "bin/win")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(target_dir "bin/linux")
else()
    message(SEND_ERROR "You are on an unsupported platform! (Not Win or Unix)")
endif(WIN32)

qt_add_executable(${PROJECT_NAME} ${GUI_TYPE}
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../resources.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/../resources/modbusscope.rc)

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${target_dir} )

target_link_libraries(${SCOPESOURCE} PRIVATE qcustomplot muparser ${QT_LIB})

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
)

# Reset GUI type to blank
set(GUI_TYPE "")

# Copy a prebuilt adapter next to the main executable so it can be found at runtime.
# The adapter binary is a tracked input, so an updated adapter is re-copied without relinking the application.
function(copy_adapter_to_output ADAPTER_TARGET OUTPUT_DIR)
    get_target_property(adapter_source ${ADAPTER_TARGET} IMPORTED_LOCATION)
    if(NOT adapter_source)
        message(FATAL_ERROR "copy_adapter_to_output: ${ADAPTER_TARGET} has no IMPORTED_LOCATION")
    endif()
    get_filename_component(adapter_name "${adapter_source}" NAME)
    set(adapter_output "${OUTPUT_DIR}/${adapter_name}")

    add_custom_command(
        OUTPUT "${adapter_output}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${OUTPUT_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy "${adapter_source}" "${adapter_output}"
        DEPENDS "${adapter_source}"
        COMMENT "Copying ${ADAPTER_TARGET} to output directory"
        VERBATIM
    )
    add_custom_target(copy_${ADAPTER_TARGET} ALL DEPENDS "${adapter_output}")
    add_dependencies(${PROJECT_NAME} copy_${ADAPTER_TARGET})
endfunction()

set(adapter_output_dir "${CMAKE_CURRENT_BINARY_DIR}/${target_dir}")

copy_adapter_to_output(modbusadapter "${adapter_output_dir}")
copy_adapter_to_output(iec104adapter "${adapter_output_dir}")

# dummyadapter is dev/test-only: copy it alongside modbusadapter in non-Release builds so it
# is discoverable for manual testing, but keep it out of Release builds so a future deploy
# script relying on "copy everything from the output dir" can't accidentally ship it.
if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    copy_adapter_to_output(dummyadapter "${adapter_output_dir}")
endif()

# Copy the demo data file next to the main executable (loaded from the Quick Start dialog)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/data/demo.csv
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMENT "Copying demo.csv to output directory"
)

# Do platform specific post target stuff
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")

    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

elseif(WIN32)
    # not required
endif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
