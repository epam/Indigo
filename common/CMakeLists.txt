cmake_minimum_required(VERSION 2.8)
project(Common C CXX)
# Collect file list for BaseC
file (GLOB BaseC_src base_c/*.c*)
if (MSVC)
    file (GLOB BaseC_src_excl base_c/*_posix.c*)
else()
    file (GLOB BaseC_src_excl base_c/*_win.c* base_c/*_win32.c*)
endif()
list (REMOVE_ITEM BaseC_src ${BaseC_src_excl})
file (GLOB BaseC_headers base_c/*.h*)

# Collect file list for BaseCpp
file (GLOB BaseCpp_src base_cpp/*.c*)
if (MSVC)
    file (GLOB BaseCpp_src_excl base_cpp/*_posix.c*)
else()
    file (GLOB BaseCpp_src_excl base_cpp/*_win.c* base_cpp/*_win32.c*)
endif()
list (REMOVE_ITEM BaseCpp_src ${BaseCpp_src_excl})
file (GLOB BaseCpp_headers base_cpp/*.h*)

# comment to regenerate files

# Collect file list for LZW
file (GLOB LZW_src lzw/*.c*)
file (GLOB LZW_headers lzw/*.h*)

# Collect file list for Math
file (GLOB Math_src math/*.c*)
file (GLOB Math_headers math/*.h*)

source_group("BaseC" FILES ${BaseC_src} ${BaseC_headers})
source_group("BaseCpp" FILES ${BaseCpp_src} ${BaseCpp_headers})
source_group("LZW" FILES ${LZW_src} ${LZW_headers})
source_group("Math" FILES ${Math_src} ${Math_headers})

include_directories(${Common_SOURCE_DIR})
include_directories(${ZLib_HEADERS_DIR})

if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "i386;x86_64")
endif()

add_library(common ${BaseC_src} ${BaseC_headers} ${BaseCpp_src} ${BaseCpp_headers} ${LZW_src} ${LZW_headers} ${Math_src} ${Math_headers})

if(VISIBILITY_HIDDEN)
    SET(COMPILE_FLAGS "${COMPILE_FLAGS} -fvisibility=hidden")
endif()    
if(UNIX OR APPLE)
    SET(COMPILE_FLAGS "${COMPILE_FLAGS} -fPIC")
endif()

set_target_properties(common PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")
if(UNIX)
	TARGET_LINK_LIBRARIES(common "-lpthread")
endif()
pack_static(common)

