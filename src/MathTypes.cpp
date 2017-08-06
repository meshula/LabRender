
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

	v4f matrix_multiply(const m44f & a, const v4f & v) {
		const glm::mat4& m = *reinterpret_cast<const glm::mat4*>(&a);
		const glm::vec4 & vec = *reinterpret_cast<const glm::vec4*>(&v);
		glm::vec4 r = vec * m;
		return *reinterpret_cast<v4f*>(&r);
	}

	m44f matrix_multiply(const m44f & a, const m44f & b) {
		const glm::mat4 & ma = *reinterpret_cast<const glm::mat4*>(&a);
		const glm::mat4 & mb = *reinterpret_cast<const glm::mat4*>(&b);
		auto m = ma * mb;
		return *reinterpret_cast<m44f*>(&m[0][0]);
	}


#endif

} // LabRender
