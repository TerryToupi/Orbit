# First-party build for Lua 5.4.8, which ships a hand-written Makefile and no
# CMake build of its own. Nothing else in this tree is modified.
#
# Built as C on purpose: Lua-as-C raises errors through setjmp/longjmp, which is
# what lets the rest of the binary compile with exceptions off (AGENTS.md §3).

cmake_minimum_required(VERSION 3.25)

project(lua
	VERSION 5.4.8
	DESCRIPTION "Lua 5.4 scripting VM"
	HOMEPAGE_URL "https://www.lua.org"
	LANGUAGES C
)

# Every core + library translation unit. lua.c (interpreter) and luac.c
# (bytecode compiler) are mains and are deliberately excluded.
set(LUA_SOURCE_FILES
	src/lapi.c
	src/lauxlib.c
	src/lbaselib.c
	src/lcode.c
	src/lcorolib.c
	src/lctype.c
	src/ldblib.c
	src/ldebug.c
	src/ldo.c
	src/ldump.c
	src/lfunc.c
	src/lgc.c
	src/linit.c
	src/liolib.c
	src/llex.c
	src/lmathlib.c
	src/lmem.c
	src/loadlib.c
	src/lobject.c
	src/lopcodes.c
	src/loslib.c
	src/lparser.c
	src/lstate.c
	src/lstring.c
	src/lstrlib.c
	src/ltable.c
	src/ltablib.c
	src/ltm.c
	src/lundump.c
	src/lutf8lib.c
	src/lvm.c
	src/lzio.c)

set(LUA_INCLUDE_FILES
	src/lua.h
	src/lua.hpp
	src/luaconf.h
	src/lualib.h
	src/lauxlib.h)

add_library(lua STATIC ${LUA_SOURCE_FILES} ${LUA_INCLUDE_FILES})
add_library(lua::lua ALIAS lua)

set_target_properties(lua
	PROPERTIES
		C_STANDARD 99
		C_STANDARD_REQUIRED YES
		C_EXTENSIONS NO
		LINKER_LANGUAGE C
)

target_include_directories(lua SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

if(APPLE)
	target_compile_definitions(lua PRIVATE LUA_USE_MACOSX)
elseif(UNIX)
	# LUA_USE_LINUX also implies LUA_USE_READLINE, but that is referenced only by
	# lua.c, which is not part of this target — no readline dependency.
	target_compile_definitions(lua PRIVATE LUA_USE_LINUX)
endif()

if(UNIX)
	target_link_libraries(lua PRIVATE m ${CMAKE_DL_LIBS})
endif()

source_group(
	TREE
		${CMAKE_CURRENT_SOURCE_DIR}
	PREFIX
		"lua"
	FILES
		${LUA_SOURCE_FILES} ${LUA_INCLUDE_FILES}
)
