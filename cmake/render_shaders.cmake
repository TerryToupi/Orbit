find_program(ORBIT_DXC dxc REQUIRED)
if(APPLE)
    find_program(ORBIT_SPIRV_CROSS spirv-cross REQUIRED)
endif()

set(ORBIT_SHADER_DIRECTORY "${CMAKE_BINARY_DIR}/bin/shaders")
file(MAKE_DIRECTORY "${ORBIT_SHADER_DIRECTORY}")
set(ORBIT_SHADER_OUTPUTS)
foreach(SHADER geometry.vert depth.frag gbuffer.frag)
    if(SHADER MATCHES "\\.vert$")
        set(PROFILE vs_6_0)
    else()
        set(PROFILE ps_6_0)
    endif()
    set(SOURCE "${PROJECT_SOURCE_DIR}/runtime/render/shaders/${SHADER}.hlsl")
    set(OUTPUT "${ORBIT_SHADER_DIRECTORY}/${SHADER}")
    add_custom_command(OUTPUT "${OUTPUT}.spv"
        COMMAND "${ORBIT_DXC}" -spirv -T ${PROFILE} -E main -Fo "${OUTPUT}.spv.tmp" "${SOURCE}"
        COMMAND "${CMAKE_COMMAND}" -E rename "${OUTPUT}.spv.tmp" "${OUTPUT}.spv"
        DEPENDS "${SOURCE}" VERBATIM)
    list(APPEND ORBIT_SHADER_OUTPUTS "${OUTPUT}.spv")
    if(APPLE)
        add_custom_command(OUTPUT "${OUTPUT}.msl"
            COMMAND "${ORBIT_SPIRV_CROSS}" "${OUTPUT}.spv" --msl --output "${OUTPUT}.msl.tmp"
            COMMAND "${CMAKE_COMMAND}" -E rename "${OUTPUT}.msl.tmp" "${OUTPUT}.msl"
            DEPENDS "${OUTPUT}.spv" VERBATIM)
        list(APPEND ORBIT_SHADER_OUTPUTS "${OUTPUT}.msl")
    elseif(WIN32)
        add_custom_command(OUTPUT "${OUTPUT}.dxil"
            COMMAND "${ORBIT_DXC}" -T ${PROFILE} -E main -Fo "${OUTPUT}.dxil.tmp" "${SOURCE}"
            COMMAND "${CMAKE_COMMAND}" -E rename "${OUTPUT}.dxil.tmp" "${OUTPUT}.dxil"
            DEPENDS "${SOURCE}" VERBATIM)
        list(APPEND ORBIT_SHADER_OUTPUTS "${OUTPUT}.dxil")
    endif()
endforeach()
add_custom_target(orbit_render_shaders DEPENDS ${ORBIT_SHADER_OUTPUTS})
