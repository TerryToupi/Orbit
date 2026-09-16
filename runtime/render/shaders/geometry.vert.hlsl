cbuffer Geometry : register(b0, space1)
{
    column_major float4x4 world;
    column_major float4x4 view_projection;
};

struct Output
{
    float4 position : SV_Position;
    float3 world_position : TEXCOORD0;
    float3 normal : TEXCOORD1;
};

Output main(float3 position : TEXCOORD0, float3 normal : TEXCOORD1)
{
    Output output;
    float4 p = mul(world, float4(position, 1));
    output.position = mul(view_projection, p);
    output.world_position = p.xyz;
    float3 x = float3(world[0][0], world[1][0], world[2][0]);
    float3 y = float3(world[0][1], world[1][1], world[2][1]);
    float3 z = float3(world[0][2], world[1][2], world[2][2]);
    float3 cofactor = cross(y, z) * normal.x + cross(z, x) * normal.y + cross(x, y) * normal.z;
    output.normal = cofactor * (dot(x, cross(y, z)) < 0 ? -1 : 1);
    return output;
}
