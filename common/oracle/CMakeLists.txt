cmake_minimum_required(VERSION 2.6)
project(CommonOracle C CXX)

include(ConfigureCommon)
include_directories(${OCI_INCLUDE_DIRS} ${Common_SOURCE_DIR})

file (GLOB CommonOracle_src *.cpp)
file (GLOB CommonOracle_headers *.h)

add_library(common-oracle ${CommonOracle_src} ${CommonOracle_headers})
set_target_properties(common-oracle PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")

pack_static(common-oracle)