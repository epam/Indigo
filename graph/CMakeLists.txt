PROJECT(Graph)

file (GLOB Graph_src src/*.c src/*.cpp)
file (GLOB Graph_headers *.h *.hpp src/*.h src/*.hpp src/*.inc)

include_directories(${Common_SOURCE_DIR} ${Graph_SOURCE_DIR}/..)

add_library(graph OBJECT ${Graph_src} ${Graph_headers})

if(VISIBILITY_HIDDEN)
    SET(COMPILE_FLAGS "${COMPILE_FLAGS} -fvisibility=hidden")
endif()
if(UNIX OR APPLE)
    SET(COMPILE_FLAGS "${COMPILE_FLAGS} -fPIC")
endif()
set_target_properties(graph PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")
#target_link_libraries(graph common)

#pack_static(graph)
