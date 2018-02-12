
#include "LabRender/MathTypes.h"
#include <glm/glm.hpp>

namespace lab {

#ifdef __APPLE__

	m44f m44f_identity = matrix_identify_float4x4;

#else

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

	m44f matrix_multiply(const m44f& a, const m44f& b) {
		const glm::mat4 & ma = *reinterpret_cast<const glm::mat4*>(&a);
		const glm::mat4 & mb = *reinterpret_cast<const glm::mat4*>(&b);
		auto m = ma * mb;
		return *reinterpret_cast<m44f*>(&m[0][0]);
	}

    void m44f::orthonormalize()
    {
        columns[0] = normalized(columns[0]); // right
        columns[1] = normalized(columns[1]); // up
        columns[2] = normalized(columns[2]); // dir
    }

    void m44f::decompose(v3f& translation, v3f& rotation, v3f& scale) const
    {
        // following code in ImGuizmo
        m44f mat = *this;

        scale.x = length(mat.columns[0]);
        scale.y = length(mat.columns[1]);
        scale.z = length(mat.columns[2]);

        mat.orthonormalize();

        constexpr float  RAD2DEG = 180.f / (float) M_PI;

        rotation.x = RAD2DEG * atan2f(mat.columns[1].z, mat.columns[2].z);
        rotation.y = RAD2DEG * atan2f(-mat.columns[0].z, sqrtf(mat.columns[1].z * mat.columns[1].z + mat.columns[2].z * mat.columns[2].z));
        rotation.z = RAD2DEG * atan2f(mat.columns[0].y, mat.columns[0].x);

        translation.x = mat.columns[3].x;
        translation.y = mat.columns[3].y;
        translation.z = mat.columns[3].z;
    }

    void m44f::recompose(const v3f& translation, const v3f& rotation, const v3f& scale)
    {
        // following code in ImGuizmo
        m44f mat = m44f_identity;

        constexpr float  DEG2RAD = (float)M_PI / 180.f;
        mat.rotate(rotation.x, { 1, 0, 0 });
        mat.rotate(rotation.y, { 0, 1, 0 });
        mat.rotate(rotation.z, { 0, 0, 1 });

        float validScale[3];
        validScale[0] = fabsf(scale.x) < FLT_EPSILON ? 0.001f : scale.x;
        validScale[1] = fabsf(scale.y) < FLT_EPSILON ? 0.001f : scale.y;
        validScale[2] = fabsf(scale.z) < FLT_EPSILON ? 0.001f : scale.z;

        for (int i = 0; i < 3; ++i)
            columns[i] = mat.columns[i] * validScale[i];

        columns[3] = { translation.x, translation.y, translation.z, 1.f };
    }



#endif

} // LabRender
