#include <render/graphics_math.h>
#include <cstdio>

#define CHECK(x) do { if (!(x)) { std::fprintf(stderr, "%d: %s\n", __LINE__, #x); return 1; } } while (0)

int main()
{
    b3Transform transform = {.p = {.x = 2, .y = -3, .z = 4}, .q = b3MakeQuatFromAxisAngle({.y = 1}, B3_PI / 2)};
    b3Vec3 scale = {.x = -2, .y = .5f, .z = 3};
    Mat4 world = make_transform_matrix(transform, scale);
    b3Vec3 point = b3TransformPoint(transform, scale);
    CHECK(fabsf(world.m[0] + world.m[4] + world.m[8] + world.m[12] - point.x) < 1e-5f);
    CHECK(fabsf(world.m[1] + world.m[5] + world.m[9] + world.m[13] - point.y) < 1e-5f);
    CHECK(fabsf(world.m[2] + world.m[6] + world.m[10] + world.m[14] - point.z) < 1e-5f);
    Mat4 identity = mul(make_view_matrix(transform), make_transform_matrix(transform));
    for (uint32_t i = 0; i < 16; ++i)
        CHECK(fabsf(identity.m[i] - (i % 5 == 0 ? 1.0f : 0.0f)) < 1e-5f);

    Mat4 projection = make_perspective(B3_PI / 2, 2, .25f, 100);
    CHECK(fabsf((-projection.m[10] * .25f + projection.m[14]) / .25f) < 1e-6f);
    CHECK(fabsf((-projection.m[10] * 100 + projection.m[14]) / 100 - 1) < 1e-6f);
    CHECK(fabsf(projection.m[0] * 2 - 1) < 1e-6f && fabsf(projection.m[5] - 1) < 1e-6f);
    CHECK(projection.m[11] == -1 && projection.m[15] == 0);
    Mat4 combined = mul(projection, make_view_matrix({.p = {.z = 3}, .q = b3Quat_identity}));
    CHECK(fabsf(combined.m[15] - 3) < 1e-6f);
    CHECK(combined.m[14] / combined.m[15] > 0 && combined.m[14] / combined.m[15] < 1);
    return 0;
}
