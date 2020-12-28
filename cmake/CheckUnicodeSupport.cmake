cmake_minimum_required(VERSION 3.16.3)

include(CheckCXXSourceCompiles)

function(check_cxx_unicode_support IS_SUPPORTED)

	set(CMAKE_CXX_STANDARD 20)

	if (CMAKE_CXX_COMPILER_ID MATCHES Clang)
		set(CMAKE_REQUIRED_FLAGS "-stdlib=libc++ -lc++abi")
	endif()

	try_compile(${IS_SUPPORTED}
		${CMAKE_CURRENT_BINARY_DIR}
		"${PROJECT_SOURCE_DIR}/cmake/CheckUnicodeSupport.cpp"
		CXX_STANDARD 20
		OUTPUT_VARIABLE TRY_COMPILE_OUTPUT)

	message("check_cxx_unicode_support IS_SUPPORTED = ${${IS_SUPPORTED}}")
	if (NOT ${${IS_SUPPORTED}})
		message("check_cxx_unicode_support output = ${TRY_COMPILE_OUTPUT}")
	endif()

endfunction()
