# First-party build for TLSF, which ships no build system at all. Nothing else
# in this tree is modified.
#
# Two-Level Segregated Fit: O(1) general-purpose allocator, public domain, C.
# Backs lua_Alloc, which needs realloc/free semantics an arena cannot provide
# (AGENTS.md §3).

cmake_minimum_required(VERSION 3.25)

project(tlsf
	DESCRIPTION "Two-Level Segregated Fit allocator"
	HOMEPAGE_URL "https://github.com/mattconte/tlsf"
	LANGUAGES C
)

set(TLSF_SOURCE_FILES tlsf.c)
set(TLSF_INCLUDE_FILES tlsf.h)

add_library(tlsf STATIC ${TLSF_SOURCE_FILES} ${TLSF_INCLUDE_FILES})
add_library(tlsf::tlsf ALIAS tlsf)

set_target_properties(tlsf
	PROPERTIES
		C_STANDARD 99
		C_STANDARD_REQUIRED YES
		C_EXTENSIONS NO
		LINKER_LANGUAGE C
)

target_include_directories(tlsf SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

source_group(
	TREE
		${CMAKE_CURRENT_SOURCE_DIR}
	PREFIX
		"tlsf"
	FILES
		${TLSF_SOURCE_FILES} ${TLSF_INCLUDE_FILES}
)
