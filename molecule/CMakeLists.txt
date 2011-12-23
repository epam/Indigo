PROJECT(Molecule)

file (GLOB Molecule_src src/*.c src/*.cpp)
file (GLOB Molecule_headers *.h *.hpp src/*.h src/*.hpp src/*.inc)

# Collect file list for GZip
file (GLOB GZip_src ${Common_SOURCE_DIR}/gzip/*.c*)
file (GLOB GZip_headers ${Common_SOURCE_DIR}/gzip/*.h*)
source_group("GZip" FILES ${GZip_src} ${GZip_headers})

include_directories(${Common_SOURCE_DIR} ${Molecule_SOURCE_DIR}/.. ${ZLib_HEADERS_DIR} ${TinyXML_HEADERS_DIR})

if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "i386;x86_64")
endif()

add_library(molecule ${Molecule_src} ${Molecule_headers} ${GZip_src} ${GZip_headers})

target_link_libraries(molecule graph z tinyxml)
