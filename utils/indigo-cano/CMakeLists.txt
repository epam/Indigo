cmake_minimum_required(VERSION 3.4)

project(indigo-cano LANGUAGES C)

add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/main.c)
target_link_libraries(${PROJECT_NAME} indigo)

add_test(NAME cano-simple-test COMMAND ${PROJECT_NAME} - "[C@@H](N)(C)C(=O)O")
add_test(NAME cano-layered-test COMMAND ${PROJECT_NAME} - "[C@@H](N)(C)C(=O)O" -layered)
add_test(NAME cano-option-test COMMAND ${PROJECT_NAME} - "[C@@H](N)(C)C(=O)O" -no-arom -no-cistrans -no-tetrahedral)
