cmake_minimum_required(VERSION 3.4)

project(bingo-file LANGUAGES CXX)

add_executable(bingo-index ${CMAKE_CURRENT_SOURCE_DIR}/bingo-index.cpp)
target_link_libraries(bingo-index indigo-core)
add_custom_command(TARGET bingo-index POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${DIST_DIRECTORY}/utils
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:bingo-index> ${DIST_DIRECTORY}/utils/
)

add_executable(bingo-search ${CMAKE_CURRENT_SOURCE_DIR}/bingo-search.cpp)
target_link_libraries(bingo-search indigo-core)
# add openmp
target_compile_options(bingo-search PRIVATE -fopenmp)

add_custom_command(TARGET bingo-search POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${DIST_DIRECTORY}/utils
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:bingo-search> ${DIST_DIRECTORY}/utils/
)
