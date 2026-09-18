void main(float3 world_position : TEXCOORD0)
{
    if (world_position.x > 10000)
        discard;
}
