PROJECT(InChI)

file (GLOB InChI_src inchi_dll/*.c)
file (GLOB InChI_headers inchi_dll/*.h)

add_library(inchi STATIC ${InChI_src} ${InChI_headers})
set_target_properties(inchi PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")

set_property(TARGET inchi PROPERTY FOLDER "third_party")
pack_static(inchi)
