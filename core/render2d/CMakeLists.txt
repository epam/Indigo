cmake_minimum_required(VERSION 3.6)

project(render2d LANGUAGES C CXX)

file(GLOB ${PROJECT_NAME}_SOURCES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SOURCES})
get_target_property(IndigoCore_INCLUDE_DIRECTORIES indigo-core INCLUDE_DIRECTORIES)
target_include_directories(${PROJECT_NAME}
        PRIVATE ${IndigoCore_INCLUDE_DIRECTORIES}
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/)

if (BUILD_STANDALONE)
    target_link_libraries(${PROJECT_NAME}
            PUBLIC cairo)
else ()
    find_package(Cairo REQUIRED)
    target_link_libraries(${PROJECT_NAME}
            PUBLIC Cairo::Cairo)
endif ()
