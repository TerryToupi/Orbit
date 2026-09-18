#include <render/graphics_math.h>
#include <cassert>

Mat4 make_transform_matrix(const b3Transform& transform, b3Vec3 scale)
{
    b3Matrix3 rotation = b3MakeMatrixFromQuat(transform.q);
    return {.m = {
        rotation.cx.x * scale.x, rotation.cx.y * scale.x, rotation.cx.z * scale.x, 0,
        rotation.cy.x * scale.y, rotation.cy.y * scale.y, rotation.cy.z * scale.y, 0,
        rotation.cz.x * scale.z, rotation.cz.y * scale.z, rotation.cz.z * scale.z, 0,
        transform.p.x, transform.p.y, transform.p.z, 1
    }};
}

Mat4 make_view_matrix(const b3Transform& camera)
{
    return make_transform_matrix(b3InvertTransform(camera));
}

Mat4 make_perspective(float vertical_fov, float aspect, float near_plane, float far_plane)
{
    assert(vertical_fov > 0 && vertical_fov < B3_PI && aspect > 0 && near_plane > 0 && far_plane > near_plane);
    float y = 1 / tanf(vertical_fov * .5f);
    float z = far_plane / (near_plane - far_plane);
    return {.m = {y / aspect, 0, 0, 0, 0, y, 0, 0, 0, 0, z, -1, 0, 0, z * near_plane, 0}};
}

Mat4 mul(const Mat4& a, const Mat4& b)
{
    Mat4 result = {.m = {}};
    for (uint32_t column = 0; column < 4; ++column) {
        for (uint32_t row = 0; row < 4; ++row) {
            for (uint32_t k = 0; k < 4; ++k)
                result.m[column * 4 + row] += a.m[k * 4 + row] * b.m[column * 4 + k];
        }
    }
    return result;
}
