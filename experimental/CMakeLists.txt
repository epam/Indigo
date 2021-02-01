PROJECT(Experimental)

file (GLOB Experimental_src src/*.c src/*.cpp)
file (GLOB Experimental_headers *.h *.hpp src/*.h src/*.hpp src/*.inc)

include_directories(${Common_SOURCE_DIR} ${Experimental_SOURCE_DIR}/..)

add_library(experimental OBJECT ${Experimental_src} ${Experimental_headers})

if(UNIX OR APPLE)
    set(COMPILE_FLAGS "${COMPILE_FLAGS} -Wall")
endif()

set_target_properties(experimental PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")
