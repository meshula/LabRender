//
//  UtilityModel.h
//  LabApp
//
//  Created by Nick Porcino on 2014 03/10.
//  Copyright (c) 2014 Nick Porcino. All rights reserved.
//

#pragma once
#include <LabRender/LabRender.h>
#include "LabRender/Model.h"

namespace lab {

class UtilityModel : public ModelPart {
public:
	LR_API UtilityModel();
	LR_API virtual ~UtilityModel() { }

	LR_API void createFullScreenQuad();
    LR_API void createFullScreenTri();
	LR_API void createPlane(float xHalf, float yHalf, int xSegments, int ySegments);
	LR_API void createCylinder(float radiusTop, float radiusBottom, float height,
                               int radialSegments, int heightSegments, bool openEnded);
	LR_API void createSphere(float radius_, int widthSegments_, int heightSegments_,
                             float phiStart_, float phiLength_, float thetaStart_, float thetaLength_, bool uvw);
	LR_API void createIcosahedron(float radius);
	LR_API void createBox(float xHalf, float yHalf, float zHalf,
                          int xSegments_, int ySegments_, int zSegments_, bool insideOut, bool uvw);
	LR_API void createSkyBox(int xSegments_, int ySegments_, int zSegments_);
	LR_API void createFrustum(float x1Half, float y1Half, float x2Half, float y2Half, float znear, float zfar);
	LR_API void createFrustum(float znear, float zfar, float yfov, float aspect);

protected:
    float radius;
    int xSegments, ySegments, zSegments;
    float phiStart,   phiLength;
    float thetaStart, thetaLength;
};

} // Lab
