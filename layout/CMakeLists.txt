PROJECT(Layout)

file (GLOB Layout_src src/*.c src/*.cpp)
file (GLOB Layout_headers *.h* src/*.h* src/*.inc)

include_directories(${Common_SOURCE_DIR} ${Common_SOURCE_DIR}/..)

if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "i386;x86_64")
endif()

add_library(layout ${Layout_src} ${Layout_headers})

target_link_libraries(layout molecule)
