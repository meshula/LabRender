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
		mat[3].x = _translate.x;
		mat[3].y = _translate.y;
		mat[3].z = _translate.z;
		rotateX(mat, _ypr.x);
		rotateY(mat, _ypr.y);
		rotateZ(mat, _ypr.z);
		lab::scale(mat, _scale);
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
        result.first = v3f{matrix_multiply(_transform, p)};
		p = {bounds.second.x, bounds.second.y, bounds.second.z, 1.f};
        result.second = v3f{matrix_multiply(_transform, p)};
		return result;
	}


}
