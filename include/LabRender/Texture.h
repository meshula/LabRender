//
//  Texture.h
//  LabApp
//
//  Created by Nick Porcino on 2014 02/28.
//  Copyright (c) 2014 Nick Porcino. All rights reserved.
//

#pragma once

#include <LabRender/LabRender.h>
#include <LabRender/InOut.h>
#include <LabRender/SemanticType.h>
#include <LabRender/TextureType.h>

#include <map>
#include <vector>

namespace lab {

    struct Texture
    {
        static size_t pixelByteSize(TextureType);

        enum class Role {
            texture2d, textureDepth, textureCube };

        Role role = Role::texture2d;

		int width = 0;
		int height = 0;
		int depth = 1;   // dimensions

		unsigned int id = 0;
		int target; // Texture::Kind in GL terms, eg Texture2D
		bool depthTexture = false;
        int format; // TextureType, in gl terms, eg GL_RGBA8
        int type; // channel type, in gl terms, eg GL_FLOAT
        int channels; // GL_RED, RG, RGB, or RGBA

        Texture();
        ~Texture();

        void bind(int unit = 0) const;
        void unbind(int unit = 0) const;

        Texture & create(int w, int h, TextureType, int filter, int wrap);

        Texture & create(int w, int h, TextureType resultType, int filter, int wrap, TextureType srcDataType, void *data);
        Texture & create(int w, int h, int depth, TextureType resultType, int filter, int wrap, TextureType srcDataType, void *data);

        // create a depth texture
        Texture & createDepth(int w, int h);

        // cube from faces
        Texture & createCube(int w, int h, TextureType resultType, int filter, int wrap, TextureType srcDataType,
                             void* px, void* nx, void* py, void* ny, void* pz, void* nz);

        enum class CubeImageDataType { vstrip };
        Texture & createCube(int w, int h, TextureType resultType, int filter, int wrap, TextureType srcDataType,
                             CubeImageDataType cubeType, void* image);

        // offset is the texel offset in the texture where the data will be copied
        // format is the format of the supplied pixel data, see ibid
        // type is the data type of the supplied pixel data, see ibid
        // todo should be Type srcDataType, not glType
        Texture & update(int xoffset, int yoffset, int width, int height, int format, int glType, void* data);

        // copy the texture data
        std::vector<uint8_t> get();

        LR_API void save(const char * path);
    };


    class TextureSet {
    public:
        void add_texture(const std::string & id, std::shared_ptr<Texture> t) {
            _textures[id] = t;
        }
        std::shared_ptr<Texture> texture(const std::string & id) const {
            auto t = _textures.find(id);
            return t != _textures.end()? t->second : std::shared_ptr<Texture>();
        }

    private:
        std::map<std::string, std::shared_ptr<Texture>> _textures;
    };


    class TextureProvider {
    public:
        virtual ~TextureProvider() {}
        virtual std::shared_ptr<Texture> texture() const = 0;
    };

    /// @TUDO should also have a cube texture provider

    class FileTextureProvider : public TextureProvider {
    public:
        FileTextureProvider(const std::string & path);
        virtual ~FileTextureProvider() {}
        virtual std::shared_ptr<Texture> texture() const override { return _texture; }

    private:
        std::shared_ptr<Texture> _texture;
    };

    int glFormat(TextureType);
    int glInternalFormat(TextureType);
    int glType(TextureType);
    SemanticType glFormatToSemanticType(int f);
    SemanticType textureTypeToSemanticType(TextureType type);

} // Lab
