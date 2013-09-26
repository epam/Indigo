PROJECT(Layout)

file (GLOB Layout_src src/*.c src/*.cpp)
file (GLOB Layout_headers *.h* src/*.h*) #src/*.inc)

include_directories(${Common_SOURCE_DIR} ${Common_SOURCE_DIR}/..)

add_library(layout OBJECT ${Layout_src} ${Layout_headers})

if(VISIBILITY_HIDDEN)
    SET(COMPILE_FLAGS "${COMPILE_FLAGS} -fvisibility=hidden")
endif()
if(UNIX OR APPLE)
    SET(COMPILE_FLAGS "${COMPILE_FLAGS} -fPIC")
endif()
set_target_properties(layout PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")

#pack_static(layout)
