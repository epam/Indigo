macro (DEFINE_TEST test files libs)
	add_executable(${test} ${files})
	target_link_libraries(${test} ${libs})

	if(UNIX OR APPLE)
		target_link_libraries(${test} pthread)
	endif()

	set_property(TARGET ${test} PROPERTY FOLDER "tests")
	add_test(NAME ${test} COMMAND ${test})
endmacro()
