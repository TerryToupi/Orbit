#define main original_main
#include "../../runtime/render/shaders/geometry.vert.hlsl"
#undef main

Output main(float3 position : TEXCOORD0, float3 normal : TEXCOORD1)
{
    Output output = original_main(position, normal);
    output.position.x += .05 * output.position.w;
    return output;
}
