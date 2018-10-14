//
//  UtilityModel.cpp
//  LabApp
//
//  Created by Nick Porcino on 2014 03/10.
//  Copyright (c) 2014 Nick Porcino. All rights reserved.
//

#include "LabRender/UtilityModel.h"
#include <LabMath/LabMath.h>
#include <algorithm>

namespace lab {

    using namespace std;

    UtilityModel::UtilityModel()
    : ModelPart()
    , xSegments(1), ySegments(1), zSegments(1), radius(1.f) 
	{
	}

    void UtilityModel::createSphere(float radius_, int widthSegments_, int heightSegments_,
                                    float phiStart_, float phiLength_, float thetaStart_, float thetaLength_,
                                    bool uvw) 
	{
        if (uvw)
            setVAO(std::unique_ptr<VAO>(
                    new VAO(std::make_shared<Buffer<VertPT3N>>(BufferBase::BufferType::VertexBuffer))),
                            std::make_pair(V3F(-radius,-radius,-radius), V3F(radius,radius,radius)));
        else
            setVAO(std::unique_ptr<VAO>(new VAO(std::make_shared<Buffer<VertPTN>>(BufferBase::BufferType::VertexBuffer))),
                   std::make_pair(V3F(-radius,-radius,-radius), V3F(radius,radius,radius)));

        radius = radius_;
        xSegments = std::max(3, widthSegments_);
        ySegments = std::max(2, heightSegments_);
        phiStart = phiStart_;
        phiLength = phiLength_;
        thetaStart = thetaStart_;
        thetaLength = thetaLength_;

        // <= because wrap around to start
        for (int y = 0; y <= ySegments; ++y) {
            for (int x = 0; x <= xSegments; ++x) {
                float u = (float)x / (float)xSegments;
                float v = (float)y / (float)ySegments;
                float vx = -1.0f * cosf(phiStart + u * phiLength ) * sinf(thetaStart + v * thetaLength );
                float vy =         cosf(thetaStart + v * thetaLength );
                float vz =         sinf(phiStart + u * phiLength ) * sinf(thetaStart + v * thetaLength );
                if (uvw)
                    _verts->vertexData<VertPT3N>(true)->push_back(VertPT3N(v3f(radius * vx, radius * vy, radius * vz),
                                                                           v3f(vx, vy, vz),
                                                                           v3f(vx, vy, vz)));
                else
                    _verts->vertexData<VertPTN>(true)->push_back(VertPTN(v3f(radius * vx, radius * vy, radius * vz),
                                                                         v2f(u, 1.0f - v),
                                                                         v3f(vx, vy, vz)));
            }
        }

        std::shared_ptr<IndexBuffer> indices = std::make_shared<IndexBuffer>();
        for (int y = 0; y < ySegments; ++y) {
            for (int x = 0; x < xSegments; x ++ ) {
                int base = y * xSegments + 1;
                int quad[4] = {  base + x, base + (x+1),
                    base + x + xSegments+1, base + (x+1) + xSegments+1 };
                indices->push_back(quad[0]);
                indices->push_back(quad[1]);
                indices->push_back(quad[2]);
                indices->push_back(quad[1]);
                indices->push_back(quad[2]);
                indices->push_back(quad[3]);
            }
        }
        _verts->setIndices(indices);
    }

    namespace {

        void makeCubeFace(
            lab::VAO* verts, 
            lab::IndexBuffer* indices,
            int &vertexOffset,
            v3f extent,
            m33f swizzle,
            int uSegments, int vSegments, bool insideOut, bool uvw_texture)
        {
            v3f scale = { extent.x / float(uSegments), extent.y / float(vSegments), 1.f };
            v3f offset = extent * -0.5f;

            for (int v = 0; v <= vSegments; ++v) 
            {
                for (int u = 0; u <= uSegments; ++u) 
                {
                    v2f uv { float(u), float(v) };
                    v3f uvw { uv.x, uv.y, 1 };
                    v3f p = swizzle * (scale * uvw + offset);
                    v3f n = swizzle * v3f { 0, 0, insideOut? 1 : -1 };

                    if (insideOut && !uvw_texture)
                        verts->vertexData<VertPN>(true)->push_back(VertPN(p, n));
                    else if (uvw_texture)
                        verts->vertexData<VertPT3N>(true)->push_back(VertPT3N(p, uvw, n));
                    else
                        verts->vertexData<VertPTN>(true)->push_back(VertPTN(p, uv, n));
                }
            }
 
            for (int v = 0; v < vSegments; ++v) 
            {
                for (int u = 0; u < uSegments; ++u) 
                {
                    int base = vertexOffset + u + v * (uSegments + 1);
                    int quad[4] = { base,                 base + 1,
                                    base + uSegments + 1, base + 1 + uSegments + 1 };
                    indices->push_back(quad[0]);
                    indices->push_back(quad[1]);
                    indices->push_back(quad[2]);
                    indices->push_back(quad[1]);
                    indices->push_back(quad[3]);
                    indices->push_back(quad[2]);
                }
            }

            vertexOffset += (uSegments + 1) * (vSegments + 1);
        }

    }

    void UtilityModel::createBox(float xHalf, float yHalf, float zHalf, int xSegments_, int ySegments_, int zSegments_, bool insideOut, bool uvw) 
    {
        xSegments = std::max(1, xSegments_);
        ySegments = std::max(1, ySegments_);
        zSegments = std::max(1, zSegments_);

        v3f half{xHalf, yHalf, zHalf};
        v3f extent = half * 2.f;

        const int xAxis = 0;
        const int yAxis = 1;
        const int zAxis = 2;

        float flip = insideOut ? -1.f : 1.f;

        if (insideOut && !uvw)
            setVAO(std::unique_ptr<VAO>(new VAO(std::make_shared<Buffer<VertPN>>(BufferBase::BufferType::VertexBuffer))),
                   std::make_pair(-half, half));
        else if (uvw)
            setVAO(std::unique_ptr<VAO>(new VAO(std::make_shared<Buffer<VertPT3N>>(BufferBase::BufferType::VertexBuffer))),
                   std::make_pair(-half, half));
        else
            setVAO(std::unique_ptr<VAO>(new VAO(std::make_shared<Buffer<VertPTN>>(BufferBase::BufferType::VertexBuffer))),
                   std::make_pair(-half, half));

        auto indices = std::make_shared<IndexBuffer>();
        int vertexOffset = 0;

        // +Y
        makeCubeFace(_verts.get(), indices.get(), vertexOffset,
            extent, {{0,0,1},{1,0,0},{0,1,0}}, // u to z, v to x, w to y 
            zSegments, xSegments, insideOut, uvw);

        // -Y
        makeCubeFace(_verts.get(), indices.get(), vertexOffset,
            extent, {{0,0,1},{1,0,0},{0,-1,0}}, 
            zSegments, xSegments, insideOut, uvw);
            
        // +X
        makeCubeFace(_verts.get(), indices.get(), vertexOffset,
            extent, {{0,0,1},{0,1,0},{1,0,0}}, 
            zSegments, ySegments, insideOut, uvw);

        // -X
        makeCubeFace(_verts.get(), indices.get(), vertexOffset,
            extent, {{0,0,1},{0,1,0},{-1,0,0}}, 
            zSegments, ySegments, insideOut, uvw);
            
        // +Z
        makeCubeFace(_verts.get(), indices.get(), vertexOffset,
            extent, {{1,0,0},{0,1,0},{0,0,1}}, 
            xSegments, ySegments, insideOut, uvw);

        // -Z
        makeCubeFace(_verts.get(), indices.get(), vertexOffset,
            extent, {{1,0,0},{0,1,0},{0,0,-1}}, 
            xSegments, ySegments, insideOut, uvw);

        _verts->setIndices(indices);
    }

    void UtilityModel::createSkyBox(int xSegments_, int ySegments_, int zSegments_) 
    {
        createBox(0.5f, 0.5f, 0.5f, xSegments_, ySegments_, zSegments_, true, false);
    }

    void UtilityModel::createFrustum(float x1Half, float y1Half, float x2Half, float y2Half, float znear, float zfar) {}

    void UtilityModel::createFrustum(float znear, float zfar, float yfov, float aspect) {}

    void UtilityModel::createPlane(float xHalf, float yHalf, int xSegments_, int ySegments_) 
    {
        setVAO(std::unique_ptr<VAO>(new VAO(std::make_shared<Buffer<VertPTN>>(BufferBase::BufferType::VertexBuffer))),
               std::make_pair(V3F(-xHalf,-yHalf,0), V3F(xHalf,yHalf,0)));

        std::shared_ptr<IndexBuffer> indices = std::make_shared<IndexBuffer>();
        xSegments = std::max(1, xSegments_);
        ySegments = std::max(1, ySegments_);

        for (int y = 0; y <= ySegments_; ++y) {
            for (int x = 0; x <= xSegments_; ++x) {
                float u = (float)x / (float)xSegments_;
                float v = (float)y / (float)ySegments_;
                _verts->vertexData<VertPTN>(true)->push_back(VertPTN(v3f(xHalf * (u*2.f-1.f), 0, yHalf * (v*2.f-1.f)),
                                                                     v2f(u, v),
                                                                     v3f(0,1,0)));
            }
        }
        for (int y = 0; y < ySegments; ++y) {
            for (int x = 0; x < xSegments; ++x) {
                int base = y * (xSegments + 1);
                int quad[4] = { base + x,                 base + x + 1,
                                base + x + xSegments + 1, base + x + 1 + xSegments_ + 1 };
                indices->push_back(quad[0]);
                indices->push_back(quad[1]);
                indices->push_back(quad[2]);
                indices->push_back(quad[1]);
                indices->push_back(quad[3]);
                indices->push_back(quad[2]);
            }
        }
        _verts->setIndices(indices);
    }
    
    void UtilityModel::createFullScreenQuad() 
    {
        static const struct {
            float pos[3];
            float tex[2];
        } kFullscreenVertices[] = {
            { {  1.0f,  1.0f, 0.0f }, { 1.0f, 1.0f } },
            { { -1.0f,  1.0f, 0.0f }, { 0.0f, 1.0f } },
            { { -1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f } },
            { {  1.0f, -1.0f, 0.0f }, { 1.0f, 0.0f } },
        };
        static const uint32_t kFullscreenIndices[] = {
            0, 1, 2,
            0, 2, 3,
        };
        
        setVAO(std::unique_ptr<VAO>(new VAO(std::make_shared<Buffer<VertPT>>(BufferBase::BufferType::VertexBuffer))),
               std::make_pair(V3F(-1.f,-1.f,0), V3F(1.f,1.f,0)));
        
        for (int i = 0; i < 6; ++i) {
            int i2 = kFullscreenIndices[i];
            _verts->vertexData<VertPT>(true)->push_back(VertPT(v3f(kFullscreenVertices[i2].pos[0],
                                                                   kFullscreenVertices[i2].pos[1],
                                                                   kFullscreenVertices[i2].pos[2]),
                                                               v2f(kFullscreenVertices[i2].tex[0],
                                                                   kFullscreenVertices[i2].tex[1])));
        }
    }

    void UtilityModel::createFullScreenTri() 
    {
        static const struct {
            float pos[3];
            float tex[2];
        } kFullscreenVertices[] = {
            { { -1.0f,  2.0f, 0.0f }, { 0.0f, 2.0f } },
            { { -1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f } },
            { {  2.0f, -1.0f, 0.0f }, { 2.0f, 0.0f } },
        };
        static const uint32_t kFullscreenIndices[] = {
            0, 1, 2,
        };
        
        setVAO(std::unique_ptr<VAO>(new VAO(
                  std::make_shared<Buffer<VertPT>>(BufferBase::BufferType::VertexBuffer))),
                  std::make_pair(V3F(-1.f,-1.f,0), V3F(2.f,2.f,0)));
        
        for (int i = 0; i < 3; ++i) {
            int i2 = kFullscreenIndices[i];
            _verts->vertexData<VertPT>(true)->push_back(VertPT(v3f(kFullscreenVertices[i2].pos[0],
                                                                   kFullscreenVertices[i2].pos[1],
                                                                   kFullscreenVertices[i2].pos[2]),
                                                               v2f(kFullscreenVertices[i2].tex[0],
                                                                   kFullscreenVertices[i2].tex[1])));
        }
    }


    void UtilityModel::createIcosahedron(float radius) 
    {
        setVAO(std::unique_ptr<VAO>(new VAO(std::make_shared<Buffer<VertPTN>>(BufferBase::BufferType::VertexBuffer))),
               std::make_pair(V3F(-radius,-radius,-radius), V3F(radius,radius,radius)));
        const float X = 0.525731112119133606f;
        const float Z = 0.850650808352039932f;

        static float vdata[12][3] = {
            {-X, 0.0, Z}, {X, 0.0, Z}, {-X, 0.0, -Z}, {X, 0.0, -Z},
            {0.0, Z, X}, {0.0, Z, -X}, {0.0, -Z, X}, {0.0, -Z, -X},
            {Z, X, 0.0}, {-Z, X, 0.0}, {Z, -X, 0.0}, {-Z, -X, 0.0}
        };

        for (int i = 0; i < 12; i++)
            _verts->vertexData<VertPTN>(true)->push_back(VertPTN(v3f(radius * vdata[i][0], radius * vdata[i][1], radius * vdata[i][2]),
                                                                 v2f(0, 0),    /// @TODO what are good UVs?
                                                                 v3f(vdata[i][0], vdata[i][1], vdata[i][2])));

        static uint32_t tindices[20][3] = {
            {0,4,1}, {0,9,4}, {9,5,4}, {4,5,8}, {4,8,1},
            {8,10,1}, {8,3,10}, {5,3,8}, {5,2,3}, {2,7,3},
            {7,10,3}, {7,6,10}, {7,11,6}, {11,0,6}, {0,1,6},
            {6,1,10}, {9,0,11}, {9,11,2}, {9,2,5}, {7,2,11}
        };

        std::shared_ptr<IndexBuffer> indices = std::make_shared<IndexBuffer>();

        for (int i = 0; i < 20; i++) {
            indices->push_back(tindices[i][0]);
            indices->push_back(tindices[i][1]);
            indices->push_back(tindices[i][2]);
        }
        _verts->setIndices(indices);
    }


    void UtilityModel::createCylinder(float radiusTop, float radiusBottom, float height, int radialSegments, int heightSegments, bool openEnded ) 
    {
        float r = std::max(radiusTop, radiusBottom);
        setVAO(std::unique_ptr<VAO>(new VAO(std::make_shared<Buffer<VertPTN>>(BufferBase::BufferType::VertexBuffer))),
               std::make_pair(V3F(-r,-height * 0.5f,-r), V3F(r, height *0.5f,r)));

        const float CYLINDER_EPS = 1.e-5f;
        std::shared_ptr<IndexBuffer> indices = std::make_shared<IndexBuffer>();
        radialSegments = std::max(3, radialSegments);
        heightSegments = std::max(1, heightSegments);
        if (fabsf(height) < CYLINDER_EPS)
            height = CYLINDER_EPS;

        float heightHalf = height / 2;
        float tanTheta = ( radiusBottom - radiusTop ) / height;

        float invTheta = 1.f / float(radialSegments);

        for (int y = 0; y <= heightSegments; ++y) {
            float v = float(y) / float(heightSegments);
            float radius = v * (radiusBottom - radiusTop) + radiusTop;

            for (int x = 0; x < radialSegments+1; ++x) {
                float u = float(x) * invTheta;

                float vx = sinf(u * 2.f * float(M_PI));
                float vy = - v * height + heightHalf;
                float vz = cosf(u * 2.f * float(M_PI));

                float nx = vx;
                float nz = vz;
                float ny = sqrtf(nx*nx + nz*nz) * tanTheta;
                float nl = 1.f / sqrtf(nx*nx + ny*ny + nz*nz);
                nx *= nl; ny *= nl; nz *= nl;

                _verts->vertexData<VertPTN>(true)->push_back(VertPTN(v3f(radius * vx, vy, radius * vz),
                                                                     v2f(u, 1.f - v),
                                                                     v3f(nx, ny, nz)));
            }
        }
        for (int y = 0; y < heightSegments; ++y) {
            for (int x = 0; x < radialSegments; x ++ ) {
                int base = y * (radialSegments+1);
                int quad[4] = {  base + x,                      base + (x+1),
                                 base + x + radialSegments + 1, base + (x+1) + radialSegments + 1};
                indices->push_back(quad[0]);
                indices->push_back(quad[1]);
                indices->push_back(quad[2]);
                indices->push_back(quad[1]); // ordered to keep outside edges on barycentric unit values
                indices->push_back(quad[3]);
                indices->push_back(quad[2]);
            }
        }

        // top cap
        if (!openEnded && fabsf(radiusTop) > CYLINDER_EPS) 
        {
            int topBase = (radialSegments + 1) * heightSegments;
            int base = (int) _verts->vertexData<VertPTN>(false)->count();
            for (int i = 0; i < radialSegments + 1; ++i) {
                VertPTN vert = _verts->vertexData<VertPTN>(false)->elementAt(i + topBase);
                vert.normal[0] = 0; vert.normal[1] = 1; vert.normal[2] = 0;
                _verts->vertexData<VertPTN>(true)->push_back(vert);
            }


            //        2 3 4
            //      1       5
            //     0         6
            //      11      7
            //        10 9 8

            for (int i = 0; i < radialSegments; ++i) 
            {
                indices->push_back(base); 
                indices->push_back(base+i); 
                indices->push_back(base+i+1); 
            }
            
            if (fabsf(radiusBottom) > CYLINDER_EPS) 
            {
                topBase = 0;
                base = (int) _verts->vertexData<VertPTN>(false)->count();
                for (int i = 0; i < radialSegments + 1; ++i) {
                    VertPTN vert = _verts->vertexData<VertPTN>(false)->elementAt(i + topBase);
                    vert.normal[0] = 0; vert.normal[1] = 1; vert.normal[2] = 0;
                    _verts->vertexData<VertPTN>(true)->push_back(vert);
                }
                for (int i = 0; i < radialSegments; ++i) 
                {
                    indices->push_back(base); 
                    indices->push_back(base+i); 
                    indices->push_back(base+i+1); 
                }
            }
        }
        
        _verts->setIndices(indices);
    }
    
    
} // Lab
