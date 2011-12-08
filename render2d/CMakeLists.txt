project(Render2D)

file (GLOB Render2D_src src/*.c src/*.cpp)
file (GLOB Render2D_headers *.h *.hpp src/*.h src/*.hpp src/*.inc)

# Collect file list for GZip
#file (GLOB GZip_src ${Common_SOURCE_DIR}/gzip/*.c*)
#file (GLOB GZip_headers ${Common_SOURCE_DIR}/gzip/*.h*)
#source_group("GZip" FILES ${GZip_src} ${GZip_headers})

include_directories(. src ${Cairo_headers_dir} ${Common_SOURCE_DIR} ${Molecule_SOURCE_DIR}/..)# ${ZLib_HEADERS_DIR} ${TinyXML_HEADERS_DIR})

add_library(render2d ${Render2D_src} ${Render2D_headers}) #${GZip_src} ${GZip_headers})

target_link_libraries(render2d)
