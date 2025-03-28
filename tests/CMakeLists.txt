
add_executable(basic-document basic-document.cpp)
target_link_libraries(basic-document ${PROJECT_NAME})

add_executable(random-text random-text.cpp)
target_link_libraries(random-text ${PROJECT_NAME})

enable_testing()
add_test(NAME basic-document COMMAND $<TARGET_FILE:basic-document>)
add_test(NAME random-text COMMAND $<TARGET_FILE:random-text>)
