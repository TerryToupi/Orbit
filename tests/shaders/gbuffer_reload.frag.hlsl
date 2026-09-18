#define main original_main
#include "../../runtime/render/shaders/gbuffer.frag.hlsl"
#undef main

Output main(float3 world_position : TEXCOORD0, float3 normal : TEXCOORD1, bool front : SV_IsFrontFace)
{
    Output output = original_main(world_position, normal, front);
    output.base_color = float4(1, 0, 0, 1);
    return output;
}
