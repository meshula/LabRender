
#include "LabRender/MathTypes.h"
#include <glm/glm.hpp>

namespace lab {


    m44f m44f_identity = { 1.f,0.f,0.f,0.f, 0.f,1.f,0.f,0.f, 0.f,0.f,1.f,0.f, 0.f,0.f,0.f,1.f };

    m44f matrix_invert(const m44f & a)
    {
        glm::mat4 m;
        memcpy(&m[0][0], &a, sizeof(float) * 16);
        m = glm::inverse(m);
        return *reinterpret_cast<m44f*>(&m[0][0]);
    }

    m44f matrix_transpose(const m44f & a)
    {
        glm::mat4 m;
        memcpy(&m[0][0], &a, sizeof(float) * 16);
        m = glm::transpose(m);
        return *reinterpret_cast<m44f*>(&m[0][0]);
    }

    v4f matrix_multiply(const m44f& a, const v4f& v) {
        const glm::mat4& m = *reinterpret_cast<const glm::mat4*>(&a);
        const glm::vec4 & vec = *reinterpret_cast<const glm::vec4*>(&v);
        glm::vec4 r = vec * m;
        return *reinterpret_cast<v4f*>(&r);
    }

    m44f matrix_multiply(const m44f& a, const m44f& b)
    {
        return a * b;
    }

    void orthonormalize(m44f& m)
    {
        m[0] = glm::normalize(m[0]); // right
        m[1] = glm::normalize(m[1]); // up
        m[2] = glm::normalize(m[2]); // dir
    }

    void decompose(m44f mat, v3f& translation, v3f& rotation, v3f& scale)
    {
        // following code in ImGuizmo

        scale.x = length(mat[0]);
        scale.y = length(mat[1]);
        scale.z = length(mat[2]);

        orthonormalize(mat);

        constexpr float  RAD2DEG = 180.f / (float)M_PI;

        rotation.x = RAD2DEG * atan2f(mat[1].z, mat[2].z);
        rotation.y = RAD2DEG * atan2f(-mat[0].z, sqrtf(mat[1].z * mat[1].z + mat[2].z * mat[2].z));
        rotation.z = RAD2DEG * atan2f(mat[0].y, mat[0].x);

        translation.x = mat[3].x;
        translation.y = mat[3].y;
        translation.z = mat[3].z;
    }

    m44f recompose(const v3f& translation, const v3f& rotation, const v3f& scale)
    {
        // following code in ImGuizmo
        m44f mat = m44f_identity;

        constexpr float  DEG2RAD = (float)M_PI / 180.f;
        rotate(mat, rotation.x, { 1, 0, 0 });
        rotate(mat, rotation.y, { 0, 1, 0 });
        rotate(mat, rotation.z, { 0, 0, 1 });

        float validScale[3];
        validScale[0] = fabsf(scale.x) < FLT_EPSILON ? 0.001f : scale.x;
        validScale[1] = fabsf(scale.y) < FLT_EPSILON ? 0.001f : scale.y;
        validScale[2] = fabsf(scale.z) < FLT_EPSILON ? 0.001f : scale.z;

        for (int i = 0; i < 3; ++i)
            mat[i] *= validScale[i];

        mat[3] = { translation.x, translation.y, translation.z, 1.f };

        return mat;
    }

    void transpose(m44f& mat)
    {
        mat = glm::transpose(mat);
    }

    void rotateX(m44f& mat, float degrees) 
    {
        float radians = degrees * float(M_PI / 180);
        float s = sinf(radians), c = cosf(radians);
        float t01 = mat[0].y, t02 = mat[0].z;
        float t11 = mat[1].y, t12 = mat[1].z;
        float t21 = mat[2].y, t22 = mat[2].z;
        float t31 = mat[3].y, t32 = mat[3].z;
        mat[0].y = c * t01 - s * t02;
        mat[0].z = c * t02 + s * t01;
        mat[1].y = c * t11 - s * t12;
        mat[1].z = c * t12 + s * t11;
        mat[2].y = c * t21 - s * t22;
        mat[2].z = c * t22 + s * t21;
        mat[3].y = c * t31 - s * t32;
        mat[3].z = c * t32 + s * t31;
        //float t01 = mat[0].y, t02 = mat[0].z;
        //float t11 = mat[1].y, t12 = mat[1].z;
        //float t21 = mat[2].y, t22 = mat[2].z;
        //float t31 = mat[3].y, t32 = mat[3].z;
        //mat[0].y = c * t01 - s * t02;
        //mat[0].z = c * t02 + s * t01;
        //mat[1].y = c * t11 - s * t12;
        //mat[1].z = c * t12 + s * t11;
        //mat[2].y = c * t21 - s * t22;
        //mat[2].z = c * t22 + s * t21;
        //mat[3].y = c * t31 - s * t32;
        //mat[3].z = c * t32 + s * t31;
    }

    void rotateY(m44f& mat, float degrees) 
    {
        float radians = degrees * float(M_PI / 180);
        float s = sinf(radians), c = cosf(radians);
        float t02 = mat[0].z, t00 = mat[0].x;
        float t12 = mat[1].z, t10 = mat[1].x;
        float t22 = mat[2].z, t20 = mat[2].x;
        float t32 = mat[3].z, t30 = mat[3].x;
        mat[0].z = c * t02 - s * t00;
        mat[0].x = c * t00 + s * t02;
        mat[1].z = c * t12 - s * t10;
        mat[1].x = c * t10 + s * t12;
        mat[2].z = c * t22 - s * t20;
        mat[2].x = c * t20 + s * t22;
        mat[3].z = c * t32 - s * t30;
        mat[3].x = c * t30 + s * t32;
        //float t02 = mat[0].z, t00 = mat[0].x;
        //float t12 = mat[1].z, t10 = mat[1].x;
        //float t22 = mat[2].z, t20 = mat[2].x;
        //float t32 = mat[3].z, t30 = mat[3].x;
        //mat[0].z = c * t02 - s * t00;
        //mat[0].x = c * t00 + s * t02;
        //mat[1].z = c * t12 - s * t10;
        //mat[1].x = c * t10 + s * t12;
        //mat[2].z = c * t22 - s * t20;
        //mat[2].x = c * t20 + s * t22;
        //mat[3].z = c * t32 - s * t30;
        //mat[3].x = c * t30 + s * t32;
    }

    void rotateZ(m44f& mat, float degrees) 
    {
        float radians = degrees * float(M_PI / 180);
        float s = sinf(radians), c = cosf(radians);
        float t00 = mat[0].x, t01 = mat[0].y;
        float t10 = mat[1].x, t11 = mat[1].y;
        float t20 = mat[2].x, t21 = mat[2].y;
        float t30 = mat[3].x, t31 = mat[3].y;
        mat[0].x = c * t00 - s * t01;
        mat[0].y = c * t01 + s * t00;
        mat[1].x = c * t10 - s * t11;
        mat[1].y = c * t11 + s * t10;
        mat[2].x = c * t20 - s * t21;
        mat[2].y = c * t21 + s * t20;
        mat[3].x = c * t30 - s * t31;
        mat[3].y = c * t31 + s * t30;
        //float t00 = mat[0].x, t01 = mat[0].y;
        //float t10 = mat[1].x, t11 = mat[1].y;
        //float t20 = mat[2].x, t21 = mat[2].y;
        //float t30 = mat[3].x, t31 = mat[3].y;
        //mat[0].x = c * t00 - s * t01;
        //mat[0].y = c * t01 + s * t00;
        //mat[1].x = c * t10 - s * t11;
        //mat[1].y = c * t11 + s * t10;
        //mat[2].x = c * t20 - s * t21;
        //mat[2].y = c * t21 + s * t20;
        //mat[3].x = c * t30 - s * t31;
        //mat[3].y = c * t31 + s * t30;
    }

    void rotate(m44f& mat, float degrees, float x, float y, float z) 
    {
        float radians = degrees * float(M_PI / 180);
        float d = sqrtf(x*x + y * y + z * z);
        float s = sinf(radians), c = cosf(radians), t = 1 - c;
        x /= d; y /= d; z /= d;
        mat *= m44f(
            x*x*t + c,     x*y*t - z * s, x*z*t + y * s, 0,
            y*x*t + z * s, y*y*t + c,     y*z*t - x * s, 0,
            z*x*t - y * s, z*y*t + x * s, z*z*t + c,     0,
            0,             0,             0,             1);
    }

    void scale(m44f& mat, float x, float y, float z) 
    {
        mat[0].x *= x; mat[0].y *= y; mat[0].z *= z;
        mat[1].x *= x; mat[1].y *= y; mat[1].z *= z;
        mat[2].x *= x; mat[2].y *= y; mat[2].z *= z;
        mat[3].x *= x; mat[3].y *= y; mat[3].z *= z;
    }

    void translate(m44f& mat, float x, float y, float z) 
    {
        mat[0].z += mat[0].x * x + mat[0].y * y + mat[0].z * z;
        mat[1].z += mat[1].x * x + mat[1].y * y + mat[1].z * z;
        mat[2].z += mat[2].x * x + mat[2].y * y + mat[2].z * z;
        mat[3].z += mat[3].x * x + mat[3].y * y + mat[3].z * z;
    }

    void ortho(m44f& mat, float l, float r, float b, float t, float n, float f) 
    {
        mat *= m44f(
            2 / (r - l), 0, 0, (r + l) / (l - r),
            0, 2 / (t - b), 0, (t + b) / (b - t),
            0, 0, 2 / (n - f), (f + n) / (n - f),
            0, 0, 0, 1);
    }

    void frustum(m44f& mat, float l, float r, float b, float t, float n, float f) 
    {
        mat *= m44f(
            2 * n / (r - l), 0, (r + l) / (r - l), 0,
            0, 2 * n / (t - b), (t + b) / (t - b), 0,
            0, 0, (f + n) / (n - f), 2 * f * n / (n - f),
            0, 0, -1, 0);
    }

    void perspective(m44f& mat, float fov, float aspect, float znear, float zfar) 
    {
        float y = tanf(fov * float(M_PI / 360)) * znear, x = y * aspect;
        return frustum(mat, -x, x, -y, y, znear, zfar);
    }

    void invert(m44f& mat) {
        float t00 = mat[0].x, t01 = mat[0].y, t02 = mat[0].z, t03 = mat[0].w;
        mat = m44f(
            mat[1].y*mat[2].z*mat[3].w - mat[1].y * mat[3].z*mat[2].w - mat[1].z * mat[2].y*mat[3].w + mat[1].z * mat[3].y*mat[2].w + mat[1].w * mat[2].y*mat[3].z - mat[1].w * mat[3].y*mat[2].z,
            -mat[0].y * mat[2].z*mat[3].w + mat[0].y * mat[3].z*mat[2].w + mat[0].z * mat[2].y*mat[3].w - mat[0].z * mat[3].y*mat[2].w - mat[0].w * mat[2].y*mat[3].z + mat[0].w * mat[3].y*mat[2].z,
            mat[0].y*mat[1].z*mat[3].w - mat[0].y * mat[3].z*mat[1].w - mat[0].z * mat[1].y*mat[3].w + mat[0].z * mat[3].y*mat[1].w + mat[0].w * mat[1].y*mat[3].z - mat[0].w * mat[3].y*mat[1].z,
            -mat[0].y * mat[1].z*mat[2].w + mat[0].y * mat[2].z*mat[1].w + mat[0].z * mat[1].y*mat[2].w - mat[0].z * mat[2].y*mat[1].w - mat[0].w * mat[1].y*mat[2].z + mat[0].w * mat[2].y*mat[1].z,

            -mat[1].x * mat[2].z*mat[3].w + mat[1].x * mat[3].z*mat[2].w + mat[1].z * mat[2].x*mat[3].w - mat[1].z * mat[3].x*mat[2].w - mat[1].w * mat[2].x*mat[3].z + mat[1].w * mat[3].x*mat[2].z,
            mat[0].x*mat[2].z*mat[3].w - mat[0].x * mat[3].z*mat[2].w - mat[0].z * mat[2].x*mat[3].w + mat[0].z * mat[3].x*mat[2].w + mat[0].w * mat[2].x*mat[3].z - mat[0].w * mat[3].x*mat[2].z,
            -mat[0].x * mat[1].z*mat[3].w + mat[0].x * mat[3].z*mat[1].w + mat[0].z * mat[1].x*mat[3].w - mat[0].z * mat[3].x*mat[1].w - mat[0].w * mat[1].x*mat[3].z + mat[0].w * mat[3].x*mat[1].z,
            mat[0].x*mat[1].z*mat[2].w - mat[0].x * mat[2].z*mat[1].w - mat[0].z * mat[1].x*mat[2].w + mat[0].z * mat[2].x*mat[1].w + mat[0].w * mat[1].x*mat[2].z - mat[0].w * mat[2].x*mat[1].z,

            mat[1].x*mat[2].y*mat[3].w - mat[1].x * mat[3].y*mat[2].w - mat[1].y * mat[2].x*mat[3].w + mat[1].y * mat[3].x*mat[2].w + mat[1].w * mat[2].x*mat[3].y - mat[1].w * mat[3].x*mat[2].y,
            -mat[0].x * mat[2].y*mat[3].w + mat[0].x * mat[3].y*mat[2].w + mat[0].y * mat[2].x*mat[3].w - mat[0].y * mat[3].x*mat[2].w - mat[0].w * mat[2].x*mat[3].y + mat[0].w * mat[3].x*mat[2].y,
            mat[0].x*mat[1].y*mat[3].w - mat[0].x * mat[3].y*mat[1].w - mat[0].y * mat[1].x*mat[3].w + mat[0].y * mat[3].x*mat[1].w + mat[0].w * mat[1].x*mat[3].y - mat[0].w * mat[3].x*mat[1].y,
            -mat[0].x * mat[1].y*mat[2].w + mat[0].x * mat[2].y*mat[1].w + mat[0].y * mat[1].x*mat[2].w - mat[0].y * mat[2].x*mat[1].w - mat[0].w * mat[1].x*mat[2].y + mat[0].w * mat[2].x*mat[1].y,

            -mat[1].x * mat[2].y*mat[3].z + mat[1].x * mat[3].y*mat[2].z + mat[1].y * mat[2].x*mat[3].z - mat[1].y * mat[3].x*mat[2].z - mat[1].z * mat[2].x*mat[3].y + mat[1].z * mat[3].x*mat[2].y,
            mat[0].x*mat[2].y*mat[3].z - mat[0].x * mat[3].y*mat[2].z - mat[0].y * mat[2].x*mat[3].z + mat[0].y * mat[3].x*mat[2].z + mat[0].z * mat[2].x*mat[3].y - mat[0].z * mat[3].x*mat[2].y,
            -mat[0].x * mat[1].y*mat[3].z + mat[0].x * mat[3].y*mat[1].z + mat[0].y * mat[1].x*mat[3].z - mat[0].y * mat[3].x*mat[1].z - mat[0].z * mat[1].x*mat[3].y + mat[0].z * mat[3].x*mat[1].y,
            mat[0].x*mat[1].y*mat[2].z - mat[0].x * mat[2].y*mat[1].z - mat[0].y * mat[1].x*mat[2].z + mat[0].y * mat[2].x*mat[1].z + mat[0].z * mat[1].x*mat[2].y - mat[0].z * mat[2].x*mat[1].y
        );
        float det = mat[0].x * t00 + mat[1].x * t01 + mat[2].x * t02 + mat[3].x * t03;
        float* m = &mat[0].x;
        for (int i = 0; i < 16; i++) 
            m[i] /= det;
    }

    /*

    m44f &m44f::operator *= (const m44f &t) {
        *this = m44f(
            mat[0].x*t.mat[0].x + mat[0].y * t.mat[1].x + mat[0].z * t.mat[2].x + mat[0].w * t.mat[3].x,
            mat[0].x*t.mat[0].y + mat[0].y * t.mat[1].y + mat[0].z * t.mat[2].y + mat[0].w * t.mat[3].y,
            mat[0].x*t.mat[0].z + mat[0].y * t.mat[1].z + mat[0].z * t.mat[2].z + mat[0].w * t.mat[3].z,
            mat[0].x*t.mat[0].w + mat[0].y * t.mat[1].w + mat[0].z * t.mat[2].w + mat[0].w * t.mat[3].w,

            mat[1].x*t.mat[0].x + mat[1].y * t.mat[1].x + mat[1].z * t.mat[2].x + mat[1].w * t.mat[3].x,
            mat[1].x*t.mat[0].y + mat[1].y * t.mat[1].y + mat[1].z * t.mat[2].y + mat[1].w * t.mat[3].y,
            mat[1].x*t.mat[0].z + mat[1].y * t.mat[1].z + mat[1].z * t.mat[2].z + mat[1].w * t.mat[3].z,
            mat[1].x*t.mat[0].w + mat[1].y * t.mat[1].w + mat[1].z * t.mat[2].w + mat[1].w * t.mat[3].w,

            mat[2].x*t.mat[0].x + mat[2].y * t.mat[1].x + mat[2].z * t.mat[2].x + mat[2].w * t.mat[3].x,
            mat[2].x*t.mat[0].y + mat[2].y * t.mat[1].y + mat[2].z * t.mat[2].y + mat[2].w * t.mat[3].y,
            mat[2].x*t.mat[0].z + mat[2].y * t.mat[1].z + mat[2].z * t.mat[2].z + mat[2].w * t.mat[3].z,
            mat[2].x*t.mat[0].w + mat[2].y * t.mat[1].w + mat[2].z * t.mat[2].w + mat[2].w * t.mat[3].w,

            mat[3].x*t.mat[0].x + mat[3].y * t.mat[1].x + mat[3].z * t.mat[2].x + mat[3].w * t.mat[3].x,
            mat[3].x*t.mat[0].y + mat[3].y * t.mat[1].y + mat[3].z * t.mat[2].y + mat[3].w * t.mat[3].y,
            mat[3].x*t.mat[0].z + mat[3].y * t.mat[1].z + mat[3].z * t.mat[2].z + mat[3].w * t.mat[3].z,
            mat[3].x*t.mat[0].w + mat[3].y * t.mat[1].w + mat[3].z * t.mat[2].w + mat[3].w * t.mat[3].w
        );
        return *this;
    }

    v4f m44f::operator * (const v4f &v) {
        return v4f(
            mat[0].x*v.x + mat[0].y * v.y + mat[0].z * v.z + mat[0].w * v.w,
            mat[1].x*v.x + mat[1].y * v.y + mat[1].z * v.z + mat[1].w * v.w,
            mat[2].x*v.x + mat[2].y * v.y + mat[2].z * v.z + mat[2].w * v.w,
            mat[3].x*v.x + mat[3].y * v.y + mat[3].z * v.z + mat[3].w * v.w
        );
    }

    v4f operator * (const v4f &v, const m44f &t) {
        return v4f(
            t.mat[0].x*v.x + t.mat[1].x*v.y + t.mat[2].x*v.z + t.mat[3].x*v.w,
            t.mat[0].y*v.x + t.mat[1].y*v.y + t.mat[2].y*v.z + t.mat[3].y*v.w,
            t.mat[0].z*v.x + t.mat[1].z*v.y + t.mat[2].z*v.z + t.mat[3].z*v.w,
            t.mat[0].w*v.x + t.mat[1].w*v.y + t.mat[2].w*v.z + t.mat[3].w*v.w
        );
    }
    */

    std::ostream &operator << (std::ostream &out, const m44f &mat) 
    {
        return out << "m44f("
            << mat[0].x << ", " << mat[0].y << ", " << mat[0].z << ", " << mat[0].w << ",\n     "
            << mat[1].x << ", " << mat[1].y << ", " << mat[1].z << ", " << mat[1].w << ",\n     "
            << mat[2].x << ", " << mat[2].y << ", " << mat[2].z << ", " << mat[2].w << ",\n     "
            << mat[3].x << ", " << mat[3].y << ", " << mat[3].z << ", " << mat[3].w << ")";
    }

} // LabRender
