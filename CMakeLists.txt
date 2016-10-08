cmake_minimum_required (VERSION 3.1.0 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 11)
project (coquilib_tests)

file(GLOB TEST_SOURCES tests/*.cpp)

include_directories(
	cli/
)


enable_testing()
foreach(tf ${TEST_SOURCES})
	get_filename_component(tname ${tf} NAME_WE)
	add_executable(${tname} ${tf})
	set_target_properties(${tname} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests)
	add_test(NAME ${tname} COMMAND ${tname})
endforeach()	
