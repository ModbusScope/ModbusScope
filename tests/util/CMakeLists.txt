
find_package(Qt5 COMPONENTS
    Test

    REQUIRED
)

include_directories(${Qt5test_INCLUDE_DIRS})

include_directories(
    ${QT_INCLUDES}
)

enable_testing()

add_executable(tst_updatenotify ${CMAKE_CURRENT_SOURCE_DIR}/testupdatenotify.cpp ${TEST_DATA_SRCS} ${GOOGLE_TEST_SOURCE})

add_test(tst_updatenotify COMMAND tst_updatenotify)

target_link_libraries(tst_updatenotify  PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
    Qt5::Test
    Threads::Threads
)

