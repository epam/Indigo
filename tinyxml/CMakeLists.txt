PROJECT(TinyXML)

file (GLOB TinyXML_src src/*.c* src/*.def)
file (GLOB TinyXML_headers include/*.h*)

set(TinyXML_HEADERS_DIR ${TinyXML_SOURCE_DIR}/include)

include_directories(${TinyXML_HEADERS_DIR})

add_library(tinyxml ${TinyXML_src} ${TinyXML_headers})
