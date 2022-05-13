
add_executable(tst_updatenotify ${CMAKE_CURRENT_SOURCE_DIR}/tst_updatenotify.cpp ${TEST_DATA_SRCS} ${GOOGLE_TEST_SOURCE})
add_executable(tst_qmuparser ${CMAKE_CURRENT_SOURCE_DIR}/tst_qmuparser.cpp ${TEST_DATA_SRCS})
add_executable(tst_formatrelativetime ${CMAKE_CURRENT_SOURCE_DIR}/tst_formatrelativetime.cpp ${TEST_DATA_SRCS})

add_test(NAME tst_updatenotify COMMAND tst_updatenotify)
add_test(NAME tst_qmuparser COMMAND tst_qmuparser)
add_test(NAME tst_formatrelativetime COMMAND tst_formatrelativetime)

target_link_libraries(tst_updatenotify  PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
    Qt::Test
    Threads::Threads
)

target_link_libraries(tst_qmuparser PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
    Qt::Test
)

target_link_libraries(tst_formatrelativetime PRIVATE
    ${SCOPESOURCE}
    ${QT_LIB}
    Qt::Test
)

