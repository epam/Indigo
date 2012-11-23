PROJECT(Reaction)

file (GLOB Reaction_src src/*.c*)
file (GLOB Reaction_headers *.h)

include_directories(${Common_SOURCE_DIR} ${Common_SOURCE_DIR}/.. ${ZLib_HEADERS_DIR} ${TinyXML_HEADERS_DIR})

if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "i386;x86_64")
endif()

add_library(reaction ${Reaction_src} ${Reaction_headers})

if(VISIBILITY_HIDDEN)
    SET(COMPILE_FLAGS "${COMPILE_FLAGS} -fvisibility=hidden")
endif()    
if(UNIX OR APPLE)
    SET(COMPILE_FLAGS "${COMPILE_FLAGS} -fPIC")
endif()
set_target_properties(reaction PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")
target_link_libraries(reaction molecule)
pack_static(reaction)

