PROJECT(InChI)

file (GLOB INCHI_BASE_src INCHI_BASE/src/*.c)
file (GLOB INCHI_BASE_headers INCHI_BASE/src/*.h)
file (GLOB INCHI_API_src INCHI_API/libinchi/src/*.c)
file (GLOB INCHI_API_headers INCHI_API/libinchi/s*.h)
add_library(inchi STATIC ${INCHI_BASE_src} ${INCHI_BASE_headers} ${INCHI_API_src} ${INCHI_API_headers})
set_target_properties(inchi PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} -DTARGET_API_LIB")

set_property(TARGET inchi PROPERTY FOLDER "third_party")
if (WITH_STATIC)
    pack_static(inchi)
endif()

if (CMAKE_C_COMPILER_ID MATCHES "Clang" AND NOT DEFINED $ENV{VERBOSE})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-pointer-bool-conversion -Wno-parentheses -Wno-sizeof-pointer-memaccess -Wno-tautological-compare -Wno-unsequenced -Wno-comment -Wno-logical-op-parentheses")
endif()
