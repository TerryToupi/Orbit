struct Output
{
    float4 base_color : SV_Target0;
    float4 normal_roughness : SV_Target1;
};

Output main(float3 world_position : TEXCOORD0, float3 normal : TEXCOORD1, bool front : SV_IsFrontFace)
{
    if (dot(normal, normal) < 1e-12)
        normal = cross(ddy(world_position), ddx(world_position)) * (front ? 1 : -1);
    normal *= rsqrt(max(dot(normal, normal), 1e-12));
    Output output;
    output.base_color = float4(abs(normal) * .6 + .2, 1);
    output.normal_roughness = float4(normal * .5 + .5, .8);
    return output;
}
