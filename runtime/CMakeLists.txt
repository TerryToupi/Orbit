set(SOURCES
    arena.h arena.cpp
    span.h
    thread_context.h thread_context.cpp
    memory_setup.h memory_setup.cpp
    rpmalloc.h rpmalloc.c
    command_stream.h
    content.h content.cpp
    file_stream.h file_stream.cpp
    artifact_cache.h artifact_cache.cpp
    servers.h servers.cpp
    path.h path.cpp
    mesh.h mesh.cpp
    gltf_import.h gltf_import.cpp
    asset_factory.h asset_factory.cpp
    scene.h
    render/gpu_mesh.h render/gpu_mesh.cpp
    render/geometry_pass.h render/geometry_pass.cpp
    render/depth_prepass.h render/depth_prepass.cpp
    render/gbuffer_pass.h render/gbuffer_pass.cpp
    render/rendering_compositor.h render/rendering_compositor.cpp
)

find_package(SDL REQUIRED)
find_package(box3d REQUIRED)
find_package(luajit REQUIRED)
find_package(EnTT REQUIRED)

add_library(orbit_runtime STATIC ${SOURCES})
target_include_directories(orbit_runtime PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(orbit_runtime SYSTEM PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../vendor/cgltf)
target_compile_features(orbit_runtime PUBLIC cxx_std_20)
target_compile_definitions(orbit_runtime PUBLIC ENABLE_STATISTICS=1 ENABLE_UNMAP=1)
target_link_libraries(orbit_runtime PUBLIC SDL3::SDL3)
set_target_properties(orbit_runtime PROPERTIES CXX_EXTENSIONS NO C_STANDARD 11 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX runtime FILES ${SOURCES})

add_executable(runtime program.cpp)
include(${PROJECT_SOURCE_DIR}/cmake/render_shaders.cmake)
add_dependencies(runtime orbit_render_shaders)
target_link_libraries(runtime PRIVATE orbit_runtime box3d luajit EnTT)
set_target_properties(runtime PROPERTIES CXX_EXTENSIONS NO RUNTIME_OUTPUT_DIRECTORY "$<1:${CMAKE_BINARY_DIR}/bin>")
