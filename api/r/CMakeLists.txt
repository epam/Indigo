cmake_minimum_required(VERSION 3.4)

project(rindigo)

find_package(R REQUIRED)

add_library(${PROJECT_NAME} SHARED ${CMAKE_CURRENT_SOURCE_DIR}/src/rindigo.cpp)
set_target_properties(${PROJECT_NAME}
        PROPERTIES PREFIX "")
target_link_libraries(${PROJECT_NAME}
        PUBLIC indigo-static
        PUBLIC indigo-renderer-static
        R::R)
