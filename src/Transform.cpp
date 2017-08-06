//
//  Transform.cpp
//  LabRender
//
//  Created by Nick Porcino on 5/21/13.
//  Copyright (c) 2015 Planet IX. All rights reserved.
//

#include "LabRender/Transform.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

namespace lab
{
    void Transform::updateTransformTRS()
	{
		m44f mat = m44f_identity;
		mat.m30 = _translate.x;
		mat.m31 = _translate.y;
		mat.m32 = _translate.z;
		mat.rotateX(_ypr.x);
		mat.rotateY(_ypr.y);
		mat.rotateZ(_ypr.z);
		mat.scale(_scale);
		_transform = mat;
    }

	void Transform::setView(v3f target, v3f up)
	{
		v3f eye = _translate;
		v3f zaxis = vector_normalize(eye - target);
		v3f xaxis = vector_normalize(vector_cross(up, zaxis));
		v3f yaxis = vector_cross(zaxis, xaxis);
		_transform = M44F(xaxis.x, yaxis.x, zaxis.x, 0,
							xaxis.y, yaxis.y, zaxis.y, 0,
							xaxis.z, yaxis.z, zaxis.z, 0,
					-vector_dot(xaxis, eye), -vector_dot(yaxis, eye), -vector_dot(zaxis, eye), 1);
	}

	void Transform::setTRS(v3f t, v3f ypr_, v3f s)
	{
		 _translate = t; _ypr = ypr_; _scale = s; updateTransformTRS();
	}

	Bounds Transform::transformBounds(const Bounds & bounds)
	{
		Bounds result;
		v4f p = {bounds.first.x, bounds.first.y, bounds.first.z, 1.f};
		result.first = matrix_multiply(_transform, p).xyz;
		p = {bounds.second.x, bounds.second.y, bounds.second.z, 1.f};
		result.second = matrix_multiply(_transform, p).xyz;
		return result;
	}


}
