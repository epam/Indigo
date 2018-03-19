set(COMMON_OUTPUT_PATH dist/${SYSTEM_NAME}/${SUBSYSTEM_NAME})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/${COMMON_OUTPUT_PATH}/lib)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/${COMMON_OUTPUT_PATH}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${COMMON_OUTPUT_PATH}/lib)

message(STATUS "Common output dir: ${CMAKE_BINARY_DIR}/${COMMON_OUTPUT_PATH}")

macro(PACK_STATIC proj)
    install(TARGETS ${proj}
        DESTINATION static/${SYSTEM_NAME}/${SUBSYSTEM_NAME}
        COMPONENT static)
    if(MSVC)
        get_target_property (output_name ${proj} "OUTPUT_NAME")
        if (NOT output_name)
            set(output_name "${proj}")
        endif ()
    endif(MSVC)
endmacro()

macro(PACK_SHARED proj)
    install(TARGETS ${proj}
        RUNTIME DESTINATION shared/${SYSTEM_NAME}/${SUBSYSTEM_NAME} COMPONENT shared)
endmacro()

macro(PACK_EXECUTABLE proj)
    install(TARGETS ${proj}
        DESTINATION .
        COMPONENT shared)
endmacro()

macro(PACK_BINGO proj)
    install(TARGETS ${proj}
        DESTINATION "lib"
        COMPONENT shared)
endmacro()
