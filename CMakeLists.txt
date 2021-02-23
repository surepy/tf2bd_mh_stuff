cmake_minimum_required(VERSION 3.16.3)

project(mh_stuff CXX)

option(MH_STUFF_BUILD_SHARED_LIBS "Build mh_stuff as a shared lib" OFF)
if (MH_STUFF_BUILD_SHARED_LIBS)
	set(BUILD_SHARED_LIBS ON)
endif()

include("cmake/CheckCoroutineSupport.cmake")
include("cmake/CheckUnicodeSupport.cmake")
include(CheckCXXCompilerFlag)
include(GenerateExportHeader)

option(MH_STUFF_COMPILE_LIBRARY "True to compile some code into to a static library for less overhead" ON)
if (MH_STUFF_COMPILE_LIBRARY)
	set(MH_PUBLIC_OR_INTERFACE "PUBLIC")
	set(MH_INTERFACE_OR_EMPTY "")
else()
	set(MH_PUBLIC_OR_INTERFACE "INTERFACE")
	set(MH_INTERFACE_OR_EMPTY "INTERFACE")
endif()

add_library(mh_stuff ${MH_INTERFACE_OR_EMPTY}
	"cpp/include/mh/algorithm/algorithm.hpp"
	"cpp/include/mh/algorithm/multi_compare.hpp"

	"cpp/include/mh/chrono/chrono_helpers.hpp"
	"cpp/include/mh/chrono/chrono_helpers.inl"

	"cpp/include/mh/concurrency/async.hpp"
	"cpp/include/mh/concurrency/dispatcher.hpp"
	"cpp/include/mh/concurrency/dispatcher.inl"
	"cpp/include/mh/concurrency/main_thread.hpp"
	"cpp/include/mh/concurrency/thread_pool.hpp"
	"cpp/include/mh/concurrency/thread_pool.inl"
	"cpp/include/mh/concurrency/thread_sentinel.hpp"
	"cpp/include/mh/concurrency/thread_sentinel.inl"

	"cpp/include/mh/containers/heap.hpp"

	"cpp/include/mh/coroutine/coroutine_include.hpp"
	"cpp/include/mh/coroutine/coroutine_common.hpp"
	"cpp/include/mh/coroutine/future.hpp"
	"cpp/include/mh/coroutine/generator.hpp"
	"cpp/include/mh/coroutine/task.hpp"
	"cpp/include/mh/coroutine/thread.hpp"
	"cpp/include/mh/coroutine/thread.inl"

	"cpp/include/mh/data/bit_float.hpp"
	"cpp/include/mh/data/bits.hpp"
	"cpp/include/mh/data/variable_pusher.hpp"

	"cpp/include/mh/error/ensure.inl"
	"cpp/include/mh/error/ensure.hpp"
	"cpp/include/mh/error/error_code_exception.hpp"
	"cpp/include/mh/error/exception_details.hpp"
	"cpp/include/mh/error/exception_details.inl"
	"cpp/include/mh/error/expected.hpp"
	"cpp/include/mh/error/nested_exception.hpp"
	"cpp/include/mh/error/not_implemented_error.hpp"
	"cpp/include/mh/error/status.hpp"

	"cpp/include/mh/io/file.hpp"
	"cpp/include/mh/io/filesystem_helpers.hpp"
	"cpp/include/mh/io/filesystem_helpers.inl"
	"cpp/include/mh/io/getopt.hpp"

	"cpp/include/mh/math/angles.hpp"
	"cpp/include/mh/math/interpolation.hpp"
	"cpp/include/mh/math/uint128.hpp"

	"cpp/include/mh/memory/buffer.inl"
	"cpp/include/mh/memory/buffer.hpp"
	"cpp/include/mh/memory/cached_variable.hpp"
	"cpp/include/mh/memory/checked_ptr.hpp"
	"cpp/include/mh/memory/memory_helpers.hpp"
	"cpp/include/mh/memory/stack_info.hpp"
	"cpp/include/mh/memory/stack_info.inl"
	"cpp/include/mh/memory/unique_object.hpp"

	"cpp/include/mh/raii/scope_exit.hpp"

	"cpp/include/mh/reflection/enum.hpp"
	"cpp/include/mh/reflection/struct.hpp"

	"cpp/include/mh/text/formatters/error_code.hpp"
	"cpp/include/mh/text/case_insensitive_string.hpp"
	"cpp/include/mh/text/charconv_helper.hpp"
	"cpp/include/mh/text/codecvt.hpp"
	"cpp/include/mh/text/codecvt.inl"
	"cpp/include/mh/text/fmtstr.hpp"
	"cpp/include/mh/text/format.hpp"
	"cpp/include/mh/text/indenting_ostream.hpp"
	"cpp/include/mh/text/insertion_conversion.hpp"
	"cpp/include/mh/text/memstream.hpp"
	"cpp/include/mh/text/string_insertion.hpp"
	"cpp/include/mh/text/stringops.hpp"

	"cpp/include/mh/types/disable_copy_move.hpp"
	"cpp/include/mh/types/enum_class_bit_ops.hpp"

	"cpp/include/mh/compiler.hpp"
	"cpp/include/mh/future.hpp"
	"cpp/include/mh/source_location.hpp"
	"cpp/include/mh/utility.hpp"
	"cpp/include/mh/variant.hpp"
)

if (MH_STUFF_COMPILE_LIBRARY)
	target_sources(mh_stuff PRIVATE
		"cpp/src/io/file.cpp"
		"cpp/src/text/case_insensitive_string.cpp"
		"cpp/src/text/string_insertion.cpp"
		"cpp/src/source_location.cpp"
	)

	target_compile_definitions(mh_stuff ${MH_PUBLIC_OR_INTERFACE}
		"MH_COMPILE_LIBRARY"
		"MH_COMPILE_LIBRARY_INLINE="
	)

	# Create a cpp file of all .inl files and add it to the target
	file(GLOB_RECURSE MH_INL_FILES "*.inl")
	# message("MH_INL_FILES = ${MH_INL_FILES}")

	file(REMOVE "${CMAKE_CURRENT_BINARY_DIR}/library.cpp")
	foreach(X IN LISTS MH_INL_FILES)
		file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/library.cpp" "\n#include \"${X}\"")
	endforeach()
	target_sources(mh_stuff PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/library.cpp")

	if (MSVC)
		target_compile_options(mh_stuff PRIVATE /W3 /permissive-)
	endif()

	generate_export_header(mh_stuff
		EXPORT_MACRO_NAME MH_STUFF_API
	)
	target_precompile_headers(mh_stuff ${MH_PUBLIC_OR_INTERFACE} "${CMAKE_CURRENT_BINARY_DIR}/mh_stuff_export.h")

else()
	target_compile_definitions(mh_stuff INTERFACE
		"MH_COMPILE_LIBRARY_INLINE=inline"
		"MH_STUFF_API="
	)
endif()

check_cxx_coroutine_support(SUPPORTS_COROUTINES COROUTINES_FLAGS)
target_compile_options(mh_stuff ${MH_PUBLIC_OR_INTERFACE} ${COROUTINES_FLAGS})

target_include_directories(mh_stuff ${MH_PUBLIC_OR_INTERFACE} "cpp/include")
target_compile_features(mh_stuff ${MH_PUBLIC_OR_INTERFACE} "cxx_std_20")

if (MSVC)
	# There is a bug in visual studio that prevents intellisense from realizing
	# /std:c++latest is on the command line if you only use target_compile_features(cxx_std_20)
	target_compile_options(mh_stuff ${MH_PUBLIC_OR_INTERFACE} "/std:c++latest")
elseif (CMAKE_CXX_COMPILER_ID MATCHES GNU)
	target_link_libraries(mh_stuff ${MH_PUBLIC_OR_INTERFACE} stdc++fs)
elseif (CMAKE_CXX_COMPILER_ID MATCHES Clang)
	target_compile_options(mh_stuff ${MH_PUBLIC_OR_INTERFACE} -stdlib=libc++)
	target_link_options(mh_stuff ${MH_PUBLIC_OR_INTERFACE} -stdlib=libc++)
	target_link_libraries(mh_stuff ${MH_PUBLIC_OR_INTERFACE} c++fs c++abi)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES GNU OR CMAKE_CXX_COMPILER_ID MATCHES Clang)
	target_compile_options(mh_stuff ${MH_PUBLIC_OR_INTERFACE} -pthread)
	target_link_options(mh_stuff ${MH_PUBLIC_OR_INTERFACE} -pthread)
endif()

check_cxx_compiler_flag(-fconcepts FCONCEPTS_FLAG)
if (FCONCEPTS_FLAG)
	target_compile_options(mh_stuff ${MH_PUBLIC_OR_INTERFACE} -fconcepts)
endif()

check_cxx_unicode_support(SUPPORTS_UNICODE)
if (NOT SUPPORTS_UNICODE)
	target_compile_definitions(mh_stuff ${MH_PUBLIC_OR_INTERFACE} "MH_BROKEN_UNICODE")
endif()

add_library(mh::stuff ALIAS mh_stuff)

if ((CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR) OR BUILD_TESTING)
	include(CTest)
	add_subdirectory("test")
endif()
