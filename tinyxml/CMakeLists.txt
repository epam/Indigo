PROJECT(TinyXML)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../common/cmake/)
find_package(TINYXML)
if (NOT TINYXML_FOUND)
    file (GLOB TinyXML_src src/*.c* src/*.def)
    file (GLOB TinyXML_headers include/*.h*)

    set(TinyXML_HEADERS_DIR ${TinyXML_SOURCE_DIR}/include)

    include_directories(${TinyXML_HEADERS_DIR})

    add_library(tinyxml ${TinyXML_src} ${TinyXML_headers})
else()
    set(TinyXML_HEADERS_DIR ${TINYXML_INCLUDE_DIRS})
endif()
