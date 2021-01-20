
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
add_executable(tst_qmuparser ${CMAKE_CURRENT_SOURCE_DIR}/testqmuparser.cpp ${TEST_DATA_SRCS})
add_executable(tst_formatrelativetime ${CMAKE_CURRENT_SOURCE_DIR}/testformatrelativetime.cpp ${TEST_DATA_SRCS})

add_test(NAME tst_updatenotify COMMAND tst_updatenotify)
add_test(NAME tst_qmuparser COMMAND tst_qmuparser)
add_test(NAME tst_formatrelativetime COMMAND tst_formatrelativetime)

target_link_libraries(tst_updatenotify  PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
    Qt5::Test
    Threads::Threads
)

target_link_libraries(tst_qmuparser PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
    Qt5::Test
    Threads::Threads
)

target_link_libraries(tst_formatrelativetime PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
    Qt5::Test
    Threads::Threads
)

