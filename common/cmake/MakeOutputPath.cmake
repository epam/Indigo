set(COMMON_OUTPUT_PATH dist/${SYSTEM_NAME}/${SUBSYSTEM_NAME})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/${COMMON_OUTPUT_PATH}/lib)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/${COMMON_OUTPUT_PATH}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${COMMON_OUTPUT_PATH}/shared)

MESSAGE(STATUS "Common output dir: ${CMAKE_BINARY_DIR}/${COMMON_OUTPUT_PATH}")

macro(PACK_STATIC proj)
	INSTALL(TARGETS ${proj}
		DESTINATION static/${SYSTEM_NAME}/${SUBSYSTEM_NAME}
		COMPONENT static)
	IF(MSVC)
		get_target_property (output_name ${proj} "OUTPUT_NAME")
		if (NOT output_name)
			set(output_name "${proj}")
		endif ()
		INSTALL(FILES ${LIBRARY_OUTPUT_PATH}/Debug/${output_name}.pdb
			DESTINATION static/${SYSTEM_NAME}/${SUBSYSTEM_NAME}
			COMPONENT static
			CONFIGURATIONS Debug)
	ENDIF(MSVC)
endmacro()

macro(PACK_SHARED proj)
	INSTALL(TARGETS ${proj}
		DESTINATION shared/${SYSTEM_NAME}/${SUBSYSTEM_NAME}
		COMPONENT shared)
endmacro()

macro(PACK_EXECUTABLE proj)
	INSTALL(TARGETS ${proj}
		DESTINATION .
		COMPONENT shared)
endmacro()

macro(PACK_BINGO proj)
    INSTALL(TARGETS ${proj}
        DESTINATION "lib"
        COMPONENT shared)
endmacro()
