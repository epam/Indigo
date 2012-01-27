project(Render2D)

file (GLOB Render2D_src src/*.c src/*.cpp)
file (GLOB Render2D_headers *.h *.hpp src/*.h src/*.hpp src/*.inc)

include_directories(. src ${Cairo_headers_dir} ${Common_SOURCE_DIR} ${Molecule_SOURCE_DIR}/..)

if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "i386;x86_64")
endif()

add_library(render2d ${Render2D_src} ${Render2D_headers})

if(VISIBILITY_HIDDEN)
    SET(COMPILE_FLAGS "${COMPILE_FLAGS} -fvisibility=hidden")
endif()
if(UNIX OR APPLE)
    SET(COMPILE_FLAGS "${COMPILE_FLAGS} -fPIC")
endif()
set_target_properties(render2d PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")
target_link_libraries(render2d cairo)
pack_static(render2d)
