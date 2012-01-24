PROJECT(Graph)

file (GLOB Graph_src src/*.c src/*.cpp)
file (GLOB Graph_headers *.h *.hpp src/*.h src/*.hpp src/*.inc)

include_directories(${Common_SOURCE_DIR} ${Graph_SOURCE_DIR}/..)

if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "i386;x86_64")
endif()

add_library(graph ${Graph_src} ${Graph_headers})

if(VISIBILITY_HIDDEN)
    SET(COMPILE_FLAGS "${COMPILE_FLAGS} -fvisibility=hidden")
endif()    
if(UNIX OR APPLE)
    SET(COMPILE_FLAGS "${COMPILE_FLAGS} -fPIC")
endif()
set_target_properties(graph PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")
target_link_libraries(graph common)
pack_static(graph)
