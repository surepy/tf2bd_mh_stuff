cmake_minimum_required(VERSION 3.16.3)

include(CheckCXXCompilerFlag)

function(check_cxx_coroutine_support IS_SUPPORTED COROUTINES_FLAGS)
	set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

	check_cxx_compiler_flag(-fcoroutines COROUTINES_FLAG_FCOROUTINES)
	if (NOT COROUTINES_FLAG_FCOROUTINES)
		check_cxx_compiler_flag(-fcoroutines-ts COROUTINES_FLAG_FCOROUTINES_TS)
	endif()

	if (COROUTINES_FLAG_FCOROUTINES)
		set(COROUTINES_FLAGS -fcoroutines)
	elseif (COROUTINES_FLAG_FCOROUTINES_TS)
		set(COROUTINES_FLAGS -fcoroutines-ts)
	endif()

	set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${COROUTINES_FLAG})

	try_compile(${IS_SUPPORTED}
		${CMAKE_CURRENT_BINARY_DIR}
		"${PROJECT_SOURCE_DIR}/cmake/CheckCoroutineSupport.cpp"
		CXX_STANDARD 20
		OUTPUT_VARIABLE TRY_COMPILE_OUTPUT)

	message("check_cxx_coroutine_support IS_SUPPORTED = ${${IS_SUPPORTED}}")
	if (NOT ${${IS_SUPPORTED}})
		message("check_cxx_coroutine_support output = ${TRY_COMPILE_OUTPUT}")
	endif()

endfunction()

