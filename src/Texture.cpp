//
//  Texture.cpp
//  LabApp
//
//  Created by Nick Porcino on 2014 02/28.
//  Copyright (c) 2014 Nick Porcino. All rights reserved.
//

#include "LabRender/Texture.h"
#include "gl4.h"
#include "LabRender/Utils.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

using namespace std;

namespace lab {

Texture::Texture()
: id(0), target(GL_TEXTURE_2D), width(0), height(0), depth(1), depthTexture(false)
, type(GL_UNSIGNED_BYTE), format(GL_RGBA8)
{
}

Texture::~Texture()
{
	glDeleteTextures(1,& id);
}

void Texture::bind(int unit)   const {
    glActiveTexture(GL_TEXTURE0 + unit);
    //    if (target == GL_TEXTURE_CUBE_MAP)
    //  glEnable(GL_TEXTURE_CUBE_MAP);
    glBindTexture(target, id);
}
void Texture::unbind(int unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    // if (target == GL_TEXTURE_CUBE_MAP)
    //  glDisable(GL_TEXTURE_CUBE_MAP);
    glBindTexture(target, 0);
}

int glType(TextureType t) {
    switch (t) {
        case TextureType::f32x1:
        case TextureType::f32x2:
        case TextureType::f32x3:
        case TextureType::f32x4: return GL_FLOAT;
        case TextureType::f16x1:
        case TextureType::f16x2:
        case TextureType::f16x3:
        case TextureType::f16x4: return GL_HALF_FLOAT;
        case TextureType::u8x1:
        case TextureType::u8x2:
        case TextureType::u8x3:
        case TextureType::u8x4: return GL_UNSIGNED_BYTE;
        case TextureType::s8x1:
        case TextureType::s8x2:
        case TextureType::s8x3:
        case TextureType::s8x4: return GL_BYTE;
        default: return 0;
    }
}

int glTypeByteSize(int t) {
    switch(t) {
        case GL_FLOAT: return 4;
        case GL_HALF_FLOAT: return 2;
        case GL_UNSIGNED_BYTE: return 1;
        case GL_BYTE: return 1;
    }
    return 0;
}

int glInternalFormat(TextureType t) {
    switch (t) {
        case TextureType::f32x1: return GL_R32F;
        case TextureType::f32x2: return GL_RG32F;
        case TextureType::f32x3: return GL_RGB32F;
        case TextureType::f32x4: return GL_RGBA32F;
        case TextureType::f16x1: return GL_R16F;
        case TextureType::f16x2: return GL_RG16F;
        case TextureType::f16x3: return GL_RGB16F;
        case TextureType::f16x4: return GL_RGBA16F;
        case TextureType::u8x1:  return GL_R8;
        case TextureType::u8x2:  return GL_RG8;
        case TextureType::u8x3:  return GL_RGB8;
        case TextureType::u8x4:  return GL_RGBA8;
        case TextureType::s8x1:  return GL_R8_SNORM;
        case TextureType::s8x2:  return GL_RG8_SNORM;
        case TextureType::s8x3:  return GL_RGB8_SNORM;
        case TextureType::s8x4:  return GL_RGBA8_SNORM;
        default: return 0;
    }
}

int glChannelCount(int t) {
    switch(t) {
        case GL_R32F: return 1;
        case GL_RG32F: return 2;
        case GL_RGB32F: return 3;
        case GL_RGBA32F: return 4;
        case GL_R16F: return 1;
        case GL_RG16F: return 2;
        case GL_RGB16F: return 3;
        case GL_RGBA16F: return 4;
        case GL_R8: return 1;
        case GL_RG8: return 2;
        case GL_RGB8: return 3;
        case GL_RGBA8: return 4;
        case GL_R8_SNORM: return 1;
        case GL_RG8_SNORM: return 2;
        case GL_RGB8_SNORM: return 3;
        case GL_RGBA8_SNORM: return 4;
        case GL_RED: return 1;
        case GL_GREEN: return 1;
        case GL_BLUE: return 1;
        case GL_ALPHA: return 1;
        case GL_RGB: return 3;
        case GL_RGBA: return 4;
        case GL_LUMINANCE: return 1;
        case GL_LUMINANCE_ALPHA: return 1;
    }
    return 0;
}

SemanticType glFormatToSemanticType(int f) {
    switch (f) {
        case GL_RED:  return SemanticType::float_st;
        case GL_RG:   return SemanticType::vec2_st;
        case GL_RGB:  return SemanticType::vec3_st;
        case GL_BGR:  return SemanticType::vec3_st;
        case GL_RGBA: return SemanticType::vec4_st;
        case GL_BGRA: return SemanticType::vec4_st;
        case GL_DEPTH_COMPONENT: return SemanticType::float_st;
        case GL_STENCIL_INDEX:   return SemanticType::int_st;
    }
    return SemanticType::vec4_st; // arbitrary default
}

int glFormat(TextureType t) {
    switch (t) {
        case TextureType::f32x1: return GL_RED;
        case TextureType::f32x2: return GL_RG;
        case TextureType::f32x3: return GL_RGB32F;
        case TextureType::f32x4: return GL_RGBA32F;
        case TextureType::f16x1: return GL_RED;
        case TextureType::f16x2: return GL_RG;
        case TextureType::f16x3: return GL_RGB16F;
        case TextureType::f16x4: return GL_RGBA16F;
        case TextureType::u8x1: return GL_RED;
        case TextureType::u8x2: return GL_RG;
        case TextureType::u8x3: return GL_RGB;
        case TextureType::u8x4: return GL_RGBA;
        case TextureType::s8x1: return GL_RED;
        case TextureType::s8x2: return GL_RG;
        case TextureType::s8x3: return GL_RGB;
        case TextureType::none:
        case TextureType::s8x4: return GL_RGBA;
		default: return GL_NONE;
    }
}

int glColorChannels(TextureType t) {
	switch (t) {
	case TextureType::f32x1: return GL_RED;
	case TextureType::f32x2: return GL_RG;
	case TextureType::f32x3: return GL_RGB;
	case TextureType::f32x4: return GL_RGBA;
	case TextureType::f16x1: return GL_RED;
	case TextureType::f16x2: return GL_RG;
	case TextureType::f16x3: return GL_RGB;
	case TextureType::f16x4: return GL_RGBA;
	case TextureType::u8x1: return GL_RED;
	case TextureType::u8x2: return GL_RG;
	case TextureType::u8x3: return GL_RGB;
	case TextureType::u8x4: return GL_RGBA;
	case TextureType::s8x1: return GL_RED;
	case TextureType::s8x2: return GL_RG;
	case TextureType::s8x3: return GL_RGB;
	case TextureType::none:
	case TextureType::s8x4: return GL_RGBA;
	default: return GL_NONE;
	}
}




size_t Texture::pixelByteSize(TextureType t) {
    switch (t) {
        case TextureType::none: return 0;
        case TextureType::f32x1: return 4;
        case TextureType::f32x2: return 8;
        case TextureType::f32x3: return 12;
        case TextureType::f32x4: return 16;
        case TextureType::f16x1: return 2;
        case TextureType::f16x2: return 4;
        case TextureType::f16x3: return 6;
        case TextureType::f16x4: return 8;
        case TextureType::u8x1: return 1;
        case TextureType::u8x2: return 2;
        case TextureType::u8x3: return 3;
        case TextureType::u8x4: return 4;
        case TextureType::s8x1: return 1;
        case TextureType::s8x2: return 2;
        case TextureType::s8x3: return 3;
        case TextureType::s8x4: return 4;
        default: return 0;
    }
}

Texture& Texture::createDepth(int w, int h)
{
    target = GL_TEXTURE_2D;
    width = w;
    height = h;
    depth = 1;
    type = GL_FLOAT;
    depthTexture = true;
    if (!id)
        glGenTextures(1, &id);

    bind();
    // http://www.opengl.org/wiki/Framebuffer_Object_Examples
	// http://www.lighthouse3d.com/tutorials/opengl_framebuffer_objects/
    //You can also try GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24 for the internal format.
    //If GL_DEPTH24_STENCIL8_EXT, go ahead and use it (GL_EXT_packed_depth_stencil)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    unbind();
    return *this;
}

Texture& Texture::create(int w, int h, TextureType type, int filter, int wrap) {
    return create(w, h, 1, type, filter, wrap, type, nullptr);
}
Texture& Texture::create(int w, int h, TextureType type, int filter, int wrap, TextureType dataType, void* data) {
    return create(w, h, 1, type, filter, wrap, dataType, data);
}

Texture& Texture::create(int w, int h, int d, TextureType type_, int filter, int wrap, TextureType datatype, void* data)
{
    target = (d == 1) ? GL_TEXTURE_2D : GL_TEXTURE_3D;
    width = w;
    height = h;
    depth = d;
    depthTexture = false;
    type = glType(datatype);
    format = glColorChannels(type_);
    if (!id)
        glGenTextures(1, &id);
    bind();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(target, GL_TEXTURE_WRAP_R, wrap);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, wrap);
    if (target == GL_TEXTURE_2D) 
	{
        // NULL means don't load data
		glTexImage2D(target, 0, glFormat(type_), w, h, 0, format, glType(datatype), NULL);
    }
    else 
	{
		// NULL means don't load data
		glTexImage3D(target, 0, glFormat(type_), w, h, d, 0, format, glType(datatype), data);
    }
	format = glInternalFormat(type_);
    unbind();
    return *this;
}



Texture& Texture::createCube(int w, int h, TextureType type_, int filter, int wrap, TextureType datatype,
                                CubeImageDataType cubeType, void* image)
{
    target = GL_TEXTURE_CUBE_MAP;
    width = w;
    height = h;
    depth = 1;
    depthTexture = false;
    type = glType(datatype);
    format = glFormat(type_);
    if (!id)
        glGenTextures(1, &id);
    bind();

    void *px, *nx, *py, *ny, *pz, *nz;
    switch (cubeType) {
        case CubeImageDataType::vstrip: {
            size_t faceStride = pixelByteSize(datatype) * w * h;
            uint8_t* img = (uint8_t*)image;
            px = img + 0 * faceStride;
            nx = img + 1 * faceStride;
            py = img + 2 * faceStride;
            ny = img + 3 * faceStride;
            pz = img + 4 * faceStride;
            nz = img + 5 * faceStride;
            break;
        }
    }

	int internalFormat = glInternalFormat(type_);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(target, GL_TEXTURE_WRAP_R, wrap);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, wrap);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, internalFormat, w, h, 0, format, type, px);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, internalFormat, w, h, 0, format, type, nx);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, internalFormat, w, h, 0, format, type, py);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, internalFormat, w, h, 0, format, type, ny);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, internalFormat, w, h, 0, format, type, pz);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, internalFormat, w, h, 0, format, type, nz);
    unbind();
    return *this;
}

    Texture& Texture::createCube(int w, int h, TextureType type_, int filter, int wrap, TextureType datatype,
                                 void* px, void* nx, void* py, void* ny, void* pz, void* nz)
	{
        target = GL_TEXTURE_CUBE_MAP;
        width = w;
        height = h;
        depth = 1;
        type = glType(datatype);
        depthTexture = false;
        format = glFormat(type_);
        if (!id)
            glGenTextures(1, &id);
        bind();

		int internalFormat = glInternalFormat(type_);


        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, filter);
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, filter);
        glTexParameteri(target, GL_TEXTURE_WRAP_R, wrap);
        glTexParameteri(target, GL_TEXTURE_WRAP_S, wrap);
        glTexParameteri(target, GL_TEXTURE_WRAP_T, wrap);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, internalFormat, w, h, 0, format, type, px);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, internalFormat, w, h, 0, format, type, nx);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, internalFormat, w, h, 0, format, type, py);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, internalFormat, w, h, 0, format, type, ny);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, internalFormat, w, h, 0, format, type, pz);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, internalFormat, w, h, 0, format, type, nz);
        unbind();
        return *this;
    }

Texture& Texture::update(int xoffset, int yoffset, int width, int height, int format, int type, void* data) {
    bind();
    glTexSubImage2D(target,
                    0, // LOD level
                    xoffset, yoffset,
                    width, height,
                    format, type, data);
    unbind();
    return *this;
}

vector<uint8_t> Texture::get()
{
    bind();
    size_t sz = width * height * glTypeByteSize(type) * glChannelCount(format);
	if (!sz)
		return vector<uint8_t>();

    vector<uint8_t> data(sz);
    glReadPixels(0, 0, width, height, format, type, &data[0]);
    unbind();
    return data;
}

void Texture::save(const char * path)
{
    if (!path)
        return;

    vector<uint8_t> t = get();
    stbi_write_png(path, width, height, glChannelCount(format), &t[0], glTypeByteSize(type) * glChannelCount(format) * width);
}


FileTextureProvider::FileTextureProvider(const string & path) {

    std::string filename = lab::expandPath(path.c_str());

    int w, h, n;
    unsigned char* img;
    stbi_set_unpremultiply_on_load(1);
    stbi_convert_iphone_png_to_rgb(1);
    img = stbi_load(filename.c_str(), &w, &h, &n, 4);

    if (img != NULL) {
        _texture = std::make_shared<Texture>();
        _texture->create(w, h, TextureType::u8x4, GL_LINEAR, GL_CLAMP_TO_EDGE, TextureType::u8x4, img);

        stbi_image_free(img);
    }
}



} // Lab
