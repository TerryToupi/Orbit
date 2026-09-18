add_executable(runtime_tests runtime_tests.cpp)
target_link_libraries(runtime_tests PRIVATE orbit_runtime)
target_compile_definitions(runtime_tests PRIVATE ORBIT_TEST_DIRECTORY="${CMAKE_CURRENT_BINARY_DIR}")
set_target_properties(runtime_tests PROPERTIES CXX_EXTENSIONS NO RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
add_test(NAME runtime_foundation COMMAND runtime_tests)
set_tests_properties(runtime_foundation PROPERTIES TIMEOUT 30 WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_executable(asset_tests asset_tests.cpp)
target_link_libraries(asset_tests PRIVATE orbit_runtime)
target_compile_definitions(asset_tests PRIVATE ORBIT_TEST_DIRECTORY="${CMAKE_CURRENT_BINARY_DIR}" ORBIT_TEST_DATA="${CMAKE_CURRENT_SOURCE_DIR}/data")
set_target_properties(asset_tests PROPERTIES CXX_EXTENSIONS NO RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
add_test(NAME static_mesh_assets COMMAND asset_tests)
set_tests_properties(static_mesh_assets PROPERTIES TIMEOUT 30 WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_executable(graphics_math_tests graphics_math_tests.cpp)
target_link_libraries(graphics_math_tests PRIVATE orbit_runtime)
set_target_properties(graphics_math_tests PROPERTIES CXX_EXTENSIONS NO RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
add_test(NAME graphics_math COMMAND graphics_math_tests)

if(ORBIT_BUILD_GPU_TESTS)
    set(TEST_SHADER_OUTPUTS)
    foreach(SHADER geometry_reload.vert gbuffer_reload.frag depth_reload.frag invalid_pipeline.frag)
        if(SHADER MATCHES "\\.vert$")
            set(PROFILE vs_6_0)
        else()
            set(PROFILE ps_6_0)
        endif()
        set(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/shaders/${SHADER}.hlsl")
        set(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${SHADER}")
        add_custom_command(OUTPUT "${OUTPUT}.spv"
            COMMAND "${ORBIT_DXC}" -spirv -T ${PROFILE} -E main -Fo "${OUTPUT}.spv" "${SOURCE}"
            DEPENDS "${SOURCE}" "${PROJECT_SOURCE_DIR}/runtime/render/shaders/geometry.vert.hlsl"
                    "${PROJECT_SOURCE_DIR}/runtime/render/shaders/gbuffer.frag.hlsl" VERBATIM)
        list(APPEND TEST_SHADER_OUTPUTS "${OUTPUT}.spv")
        if(APPLE)
            add_custom_command(OUTPUT "${OUTPUT}.msl"
                COMMAND "${ORBIT_SPIRV_CROSS}" "${OUTPUT}.spv" --msl --output "${OUTPUT}.msl"
                DEPENDS "${OUTPUT}.spv" VERBATIM)
            list(APPEND TEST_SHADER_OUTPUTS "${OUTPUT}.msl")
        elseif(WIN32)
            add_custom_command(OUTPUT "${OUTPUT}.dxil"
                COMMAND "${ORBIT_DXC}" -T ${PROFILE} -E main -Fo "${OUTPUT}.dxil" "${SOURCE}"
                DEPENDS "${SOURCE}" "${PROJECT_SOURCE_DIR}/runtime/render/shaders/geometry.vert.hlsl"
                        "${PROJECT_SOURCE_DIR}/runtime/render/shaders/gbuffer.frag.hlsl" VERBATIM)
            list(APPEND TEST_SHADER_OUTPUTS "${OUTPUT}.dxil")
        endif()
    endforeach()
    add_custom_target(rendering_test_shaders DEPENDS ${TEST_SHADER_OUTPUTS})
    add_executable(rendering_tests rendering_tests.cpp)
    target_link_libraries(rendering_tests PRIVATE orbit_runtime)
    target_compile_definitions(rendering_tests PRIVATE ORBIT_TEST_DATA="${CMAKE_CURRENT_SOURCE_DIR}/data"
                                                      ORBIT_SHADER_DIRECTORY="${CMAKE_BINARY_DIR}/bin/shaders"
                                                      ORBIT_TEST_DIRECTORY="${CMAKE_CURRENT_BINARY_DIR}")
    add_dependencies(rendering_tests orbit_render_shaders rendering_test_shaders)
    set_target_properties(rendering_tests PROPERTIES CXX_EXTENSIONS NO RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    add_test(NAME rendering_compositor COMMAND rendering_tests)
    set_tests_properties(rendering_compositor PROPERTIES TIMEOUT 30 WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif()

if(ORBIT_BUILD_GPU_TESTS AND ORBIT_BUILD_EXAMPLES)
    add_executable(rendering_demo_tests rendering_demo_tests.cpp)
    target_link_libraries(rendering_demo_tests PRIVATE orbit_runtime)
    target_compile_definitions(rendering_demo_tests PRIVATE ORBIT_TEST_DIRECTORY="${CMAKE_CURRENT_BINARY_DIR}"
        ORBIT_TEST_DATA="${CMAKE_CURRENT_SOURCE_DIR}/data" ORBIT_SHADER_DIRECTORY="${CMAKE_BINARY_DIR}/bin/shaders"
        ORBIT_DEMO_ASSET="${PROJECT_SOURCE_DIR}/examples/rendering_demo/assets/ChronographWatch.glb")
    add_dependencies(rendering_demo_tests orbit_render_shaders rendering_test_shaders)
    set_target_properties(rendering_demo_tests PROPERTIES CXX_EXTENSIONS NO RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    add_test(NAME rendering_demo COMMAND rendering_demo_tests)
    set_tests_properties(rendering_demo PROPERTIES TIMEOUT 30 WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif()
