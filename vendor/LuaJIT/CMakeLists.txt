cmake_minimum_required(VERSION 3.20.0 FATAL_ERROR)
project(luajit)

if(APPLE AND CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
    set(LUAJIT_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/bin/OSX_ARM64/libluajit.a")
else()
    message(FATAL_ERROR
        "LuaJIT: unsupported platform/architecture: "
        "${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}"
    )
endif()

if(NOT EXISTS "${LUAJIT_LIBRARY}")
    message(FATAL_ERROR
        "LuaJIT static library not found: ${LUAJIT_LIBRARY}"
    )
endif()

add_library(luajit STATIC IMPORTED GLOBAL)

set_target_properties(luajit PROPERTIES
    IMPORTED_LOCATION               "${LUAJIT_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES   "${CMAKE_CURRENT_SOURCE_DIR}/src"
)