PROJECT(Graph)

file (GLOB Graph_src src/*.c src/*.cpp)
file (GLOB Graph_headers *.h *.hpp src/*.h src/*.hpp src/*.inc)

include_directories(${Common_SOURCE_DIR} ${Graph_SOURCE_DIR}/..)

add_library(graph ${Graph_src} ${Graph_headers})

target_link_libraries(graph common)
