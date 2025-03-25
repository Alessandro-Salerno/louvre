cmake_minimum_required(VERSION 3.16)
project(louvre)

include(CTest)

file(GLOB SOURCES "src/*.cpp")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories("include")
add_library(${PROJECT_NAME} STATIC ${SOURCES})
add_subdirectory(tests)

install(TARGETS louvre
        DESTINATION lib)

install(DIRECTORY include/
        DESTINATION include)
