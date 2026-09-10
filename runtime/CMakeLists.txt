cmake_minimum_required(VERSION 3.25)
project(runtime)

set(SOURCES
    rpmalloc.h
    rpmalloc.c

    program.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

set_target_properties(${PROJECT_NAME} 
    PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        C_STANDARD 11
        C_STANDARD_REQUIRED YES
        C_EXTENSIONS NO
)

target_include_directories(${PROJECT_NAME}
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(${PROJECT_NAME} 
    PUBLIC 
        RPMALLOC_ENABLE_OVERRIDE=0
)

source_group(
    TREE 
        ${CMAKE_CURRENT_SOURCE_DIR} 
    PREFIX 
        "runtime" 
    FILES 
        ${SOURCES}
)

find_package(SDL REQUIRED)
find_package(box3d REQUIRED)
find_package(luajit REQUIRED)

target_link_libraries(${PROJECT_NAME} PUBLIC SDL3::SDL3)
target_link_libraries(${PROJECT_NAME} PUBLIC box3d)
target_link_libraries(${PROJECT_NAME} PUBLIC luajit)

