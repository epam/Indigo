PROJECT(Layout)

file (GLOB Layout_src src/*.c src/*.cpp)
file (GLOB Layout_headers *.h* src/*.h* src/*.inc)

include_directories(${Common_SOURCE_DIR} ${Common_SOURCE_DIR}/..)

if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "i386;x86_64")
endif()

add_library(layout ${Layout_src} ${Layout_headers})

if(VISIBILITY_HIDDEN)
    SET(COMPILE_FLAGS "${COMPILE_FLAGS} -fvisibility=hidden")
endif()    
if(UNIX OR APPLE)
    SET(COMPILE_FLAGS "${COMPILE_FLAGS} -fPIC")
endif()
set_target_properties(layout PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")    
target_link_libraries(layout molecule)
pack_static(layout)
