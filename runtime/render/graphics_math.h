#ifndef ORBIT_GRAPHICS_MATH_H
#define ORBIT_GRAPHICS_MATH_H

#include <box3d/math_functions.h>

// Meters, radians, right-handed world, +Y up. Cameras look along local -Z.
// Column-major matrices acting on column vectors; SDL GPU NDC has +Y up and depth 0..1.
struct Mat4
{
    float m[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
};

// Rotation must be normalized. Scale acts in local space before rotation and translation.
Mat4 make_transform_matrix(const b3Transform& transform, b3Vec3 scale = {.x = 1, .y = 1, .z = 1});
Mat4 make_view_matrix(const b3Transform& camera);
// 0 < vertical_fov < pi, aspect > 0, 0 < near_plane < far_plane.
Mat4 make_perspective(float vertical_fov, float aspect, float near_plane, float far_plane);
Mat4 mul(const Mat4& a, const Mat4& b);

#endif
