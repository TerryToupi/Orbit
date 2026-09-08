cmake_minimum_required(VERSION 3.25)
project(runtime)

set(SOURCES
    utils/types.hpp
    utils/containers.hpp
    utils/fixed_function.hpp

    window/window.hpp
    window/window.cpp

    render/renderer.hpp
    render/renderer.cpp

    app/app.hpp
    app/app.cpp

    program.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

set_target_properties(${PROJECT_NAME} 
    PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
)

target_include_directories(${PROJECT_NAME}
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}
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
find_package(lua REQUIRED)
find_package(tlsf REQUIRED)

target_link_libraries(${PROJECT_NAME} PUBLIC SDL3::SDL3)
target_link_libraries(${PROJECT_NAME} PUBLIC box3d)
target_link_libraries(${PROJECT_NAME} PUBLIC lua::lua)
target_link_libraries(${PROJECT_NAME} PUBLIC tlsf::tlsf)

