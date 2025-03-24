
add_executable(basic-document basic-document.cpp)
target_link_libraries(basic-document ${PROJECT_NAME})

enable_testing()
add_test(NAME basic-document COMMAND $<TARGET_FILE:basic-document>)
