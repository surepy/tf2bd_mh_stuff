cmake_minimum_required(VERSION 3.17)

project(mh_stuff CXX)

option(MH_STUFF_COMPILE_LIBRARY "True to compile some code into to a static library for less overhead" ON)

if (MH_STUFF_COMPILE_LIBRARY)
	add_library(mh_stuff STATIC
		"cpp/include/mh/algorithm/algorithm.hpp"
		"cpp/include/mh/algorithm/multi_compare.hpp"

		"cpp/include/mh/concurrency/main_thread.hpp"
		"cpp/include/mh/concurrency/thread_pool.hpp"

		"cpp/include/mh/data/bit_float.hpp"
		"cpp/include/mh/data/bits.hpp"
		"cpp/include/mh/data/variable_pusher.hpp"

		"cpp/include/mh/error/ensure.hpp"
		"cpp/include/mh/error/ensure.inl"
		"cpp/include/mh/error/error_code_exception.hpp"
		"cpp/include/mh/error/expected.hpp"
		"cpp/include/mh/error/nested_exception.hpp"
		"cpp/include/mh/error/status.hpp"

		"cpp/include/mh/io/getopt.hpp"

		"cpp/include/mh/math/interpolation.hpp"
		"cpp/include/mh/math/uint128.hpp"

		"cpp/include/mh/memory/buffer.hpp"
		"cpp/include/mh/memory/cached_variable.hpp"
		"cpp/include/mh/memory/checked_ptr.hpp"
		"cpp/include/mh/memory/unique_object.hpp"

		"cpp/include/mh/raii/scope_exit.hpp"

		"cpp/include/mh/reflection/enum.hpp"

		"cpp/include/mh/text/formatters/error_code.hpp"
		"cpp/include/mh/text/case_insensitive_string.hpp"
		"cpp/include/mh/text/charconv_helper.hpp"
		"cpp/include/mh/text/codecvt.hpp"
		"cpp/include/mh/text/fmtstr.hpp"
		"cpp/include/mh/text/format.hpp"
		"cpp/include/mh/text/insertion_conversion.hpp"
		"cpp/include/mh/text/memstream.hpp"
		"cpp/include/mh/text/string_insertion.hpp"
		"cpp/include/mh/text/stringops.hpp"

		"cpp/include/mh/types/disable_copy_move.hpp"

		"cpp/include/mh/compiler.hpp"
		"cpp/include/mh/future.hpp"
		"cpp/include/mh/source_location.hpp"
		"cpp/include/mh/utility.hpp"
		"cpp/include/mh/variant.hpp"
	)

	set(MH_COMPILE_LIBRARY_WORKAROUND "$MH_COMPILE_LIBRARY")
	target_compile_definitions(mh_stuff PUBLIC ${MH_COMPILE_LIBRARY_WORKAROUND} "${MH_COMPILE_LIBRARY_WORKAROUND}1")
	target_compile_definitions(mh_stuff PUBLIC
		"$$MH_COMPILE_LIBRARY"
		"$$MH_COMPILE_LIBRARY_INLINE=inline"
	)
	# target_compile_options(mh_stuff PUBLIC "-DMH_COMPILE_LIBRARY")

	# Create a cpp file of all .inl files and add it to the target
	file(GLOB_RECURSE MH_INL_FILES "*.inl")
	message("MH_INL_FILES = ${MH_INL_FILES}")

	file(REMOVE "${CMAKE_CURRENT_BINARY_DIR}/library.cpp")
	foreach(X IN LISTS MH_INL_FILES)
		file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/library.cpp" "\n#include \"${X}\"")
	endforeach()

	target_sources(mh_stuff PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/library.cpp")

	set_target_properties(mh_stuff PROPERTIES CXX_STANDARD 20)

else()
	add_library(mh_stuff INTERFACE)
endif()

add_library(mh::stuff ALIAS mh_stuff)

target_include_directories(mh_stuff PUBLIC "cpp/include")
