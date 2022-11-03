cmake_minimum_required(VERSION 3.4)

project(smiles2cdxml LANGUAGES CXX)

add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
target_link_libraries(${PROJECT_NAME} indigo-core)
