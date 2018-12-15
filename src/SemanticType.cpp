//
//  SemanticType.cpp
//  LabApp
//
//  Created by Nick Porcino on 2014 02/28.
//  Copyright (c) 2014 Nick Porcino. All rights reserved.
//

#include <LabRender/LabRender.h>
#include <LabRender/SemanticType.h>
#include <LabRender/Uniform.h>
#include "gl4.h"


#ifdef PLATFORM_WINDOWS
#define strncasecmp _strnicmp
#endif

namespace lab { namespace Render {

	struct SemanticTypeData {
		SemanticType type; const char* name; int count; int elementType; int stride;
	};

	SemanticTypeData semanticTypeData[] = {
		SemanticType::bool_st,   "bool",   1,  GL_BOOL,         sizeof(bool),
		SemanticType::bvec2_st,  "bvec2",  2,  GL_BOOL,         sizeof(bool[2]),
		SemanticType::bvec3_st,  "bvec3",  3,  GL_BOOL,         sizeof(bool[3]),
		SemanticType::bvec4_st,  "bvec4",  4,  GL_BOOL,         sizeof(bool[4]),
		SemanticType::int_st,    "int",    1,  GL_INT,          sizeof(int32_t),
		SemanticType::ivec2_st,  "ivec2",  2,  GL_INT,          sizeof(int32_t[2]),
		SemanticType::ivec3_st,  "ivec3",  3,  GL_INT,          sizeof(int32_t[3]),
		SemanticType::ivec4_st,  "ivec4",  4,  GL_INT,          sizeof(int32_t[4]),
		SemanticType::uint_st,   "uint",   1,  GL_UNSIGNED_INT, sizeof(uint32_t),
		SemanticType::uvec2_st,  "uvec2",  2,  GL_UNSIGNED_INT, sizeof(uint32_t[2]),
		SemanticType::uvec3_st,  "uvec3",  3,  GL_UNSIGNED_INT, sizeof(uint32_t[3]),
		SemanticType::uvec4_st,  "uvec4",  4,  GL_UNSIGNED_INT, sizeof(uint32_t[4]),
		SemanticType::float_st,  "float",  1,  GL_FLOAT,        sizeof(float),
		SemanticType::vec2_st,   "vec2",   2,  GL_FLOAT,        sizeof(float[2]),
		SemanticType::vec3_st,   "vec3",   3,  GL_FLOAT,        sizeof(float[3]),
		SemanticType::vec4_st,   "vec4",   4,  GL_FLOAT,        sizeof(float[4]),
		SemanticType::double_st, "double", 1,  GL_DOUBLE,       sizeof(double),
		SemanticType::dvec2_st,  "dvec2",  2,  GL_DOUBLE,       sizeof(double[2]),
		SemanticType::dvec3_st,  "dvec3",  3,  GL_DOUBLE,       sizeof(double[3]),
		SemanticType::dvec4_st,  "dvec4",  4,  GL_DOUBLE,       sizeof(double[4]),
		SemanticType::mat2_st,   "mat2",   4,  GL_FLOAT,        sizeof(float[4]),
		SemanticType::mat3_st,   "mat3",   9,  GL_FLOAT,        sizeof(float[9]),
		SemanticType::mat4_st,   "mat4",   16, GL_FLOAT,        sizeof(float[16]),
		SemanticType::mat2x3_st, "mat2x3", 6,  GL_FLOAT,        sizeof(float[6]),
		SemanticType::mat2x4_st, "mat2x4", 8,  GL_FLOAT,        sizeof(float[8]),
		SemanticType::mat3x2_st, "mat3x2", 6,  GL_FLOAT,        sizeof(float[6]),
		SemanticType::mat3x4_st, "mat3x4", 12, GL_FLOAT,        sizeof(float[12]),
		SemanticType::mat4x2_st, "mat4x2", 8,  GL_FLOAT,        sizeof(float[8]),
		SemanticType::mat4x3_st, "mat4x3", 12, GL_FLOAT,        sizeof(float[12]),
		SemanticType::atomic_uint_st, "atomic_uint", 1, GL_UNSIGNED_INT, sizeof(int32_t),

		SemanticType::sampler1D_st,   "sampler1D",   1, GL_INT, sizeof(int32_t),
		SemanticType::sampler2D_st,   "sampler2D",   1, GL_INT, sizeof(int32_t),
		SemanticType::sampler3D_st,   "sampler3D",   1, GL_INT, sizeof(int32_t),
		SemanticType::samplerCube_st, "samplerCube", 1, GL_INT, sizeof(int32_t),

		SemanticType::sampler2DRect_st,    "sampler2DRect", 1, GL_INT, sizeof(int32_t),
		SemanticType::sampler1DArray_st,   "sampler1DArray", 1, GL_INT, sizeof(int32_t),
		SemanticType::sampler2DArray_st,   "sampler2DArray", 1, GL_INT, sizeof(int32_t),
		SemanticType::samplerCubeArray_st, "samplerCubeArray", 1, GL_INT, sizeof(int32_t),

		SemanticType::samplerBuffer_st,    "samplerBuffer", 1, GL_INT, sizeof(int32_t),
		SemanticType::sampler2DMS_st,      "sampler2DMS", 1, GL_INT, sizeof(int32_t),
		SemanticType::sampler2DMSArray_st, "sampler2DMSArray", 1, GL_INT, sizeof(int32_t),

		SemanticType::sampler1DShadow_st,     "sampler1DShadow", 1, GL_INT, sizeof(int32_t),
		SemanticType::sampler2DShadow_st,     "sampler2DShadow", 1, GL_INT, sizeof(int32_t),
		SemanticType::samplerCubeShadow_st,   "samplerCubeShadow", 1, GL_INT, sizeof(int32_t),
		SemanticType::sampler2DRectShadow_st, "sampler2DRectShadow", 1, GL_INT, sizeof(int32_t),

		SemanticType::sampler1DArrayShadow_st,   "Sampler1DArrayShadow", 1, GL_INT, sizeof(int32_t),
		SemanticType::sampler2DArrayShadow_st,   "sampler2DArrayShadow", 1, GL_INT, sizeof(int32_t),
		SemanticType::samplerCubeArrayShadow_st, "samplerCubeArrayShadow", 1, GL_INT, sizeof(int32_t),

		SemanticType::isampler1D_st,   "isampler1D", 1, GL_INT, sizeof(int32_t),
		SemanticType::isampler2D_st,   "isampler2D", 1, GL_INT, sizeof(int32_t),
		SemanticType::isampler3D_st,   "isampler3D", 1, GL_INT, sizeof(int32_t),
		SemanticType::isamplerCube_st, "isamplerCube", 1, GL_INT, sizeof(int32_t),

		SemanticType::isampler2DRect_st,    "isampler2DRect", 1, GL_INT, sizeof(int32_t),
		SemanticType::isampler1DArray_st,   "isampler1DArray", 1, GL_INT, sizeof(int32_t),
		SemanticType::isampler2DArray_st,   "isampler2DArray", 1, GL_INT, sizeof(int32_t),
		SemanticType::isamplerCubeArray_st, "isamplerCubeArray", 1, GL_INT, sizeof(int32_t),

		SemanticType::isamplerBuffer_st,    "isamplerBuffer", 1, GL_INT, sizeof(int32_t),
		SemanticType::isampler2DMS_st,      "isampler2DMS", 1, GL_INT, sizeof(int32_t),
		SemanticType::isampler2DMSArray_st, "isampler2DMSArray", 1, GL_INT, sizeof(int32_t),

		SemanticType::isampler1DShadow_st,     "isampler1DShadow", 1, GL_INT, sizeof(int32_t),
		SemanticType::isampler2DShadow_st,     "isampler2DShadow", 1, GL_INT, sizeof(int32_t),
		SemanticType::isamplerCubeShadow_st,   "isamplerCubeShadow", 1, GL_INT, sizeof(int32_t),
		SemanticType::isampler2DRectShadow_st, "isampler2DRectShadow", 1, GL_INT, sizeof(int32_t),

		SemanticType::isampler1dArrayShadow_st,   "iSampler1dArrayShadow", 1, GL_INT, sizeof(int32_t),
		SemanticType::isampler2DArrayShadow_st,   "isampler2DArrayShadow", 1, GL_INT, sizeof(int32_t),
		SemanticType::isamplerCubeArrayShadow_st, "isamplerCubeArrayShadow", 1, GL_INT, sizeof(int32_t),

		SemanticType::usampler1D_st,   "usampler1D", 1, GL_INT, sizeof(int32_t),
		SemanticType::usampler2D_st,   "usampler2D", 1, GL_INT, sizeof(int32_t),
		SemanticType::usampler3D_st,   "usampler3D", 1, GL_INT, sizeof(int32_t),
		SemanticType::usamplerCube_st, "usamplerCube", 1, GL_INT, sizeof(int32_t),

		SemanticType::usampler2DRect_st,    "usampler2DRect", 1, GL_INT, sizeof(int32_t),
		SemanticType::usampler1DArray_st,   "usampler1DArray", 1, GL_INT, sizeof(int32_t),
		SemanticType::usampler2DArray_st,   "usampler2DArray", 1, GL_INT, sizeof(int32_t),
		SemanticType::usamplerCubeArray_st, "usamplerCubeArray", 1, GL_INT, sizeof(int32_t),

		SemanticType::usamplerBuffer_st,    "usamplerBuffer", 1, GL_INT, sizeof(int32_t),
		SemanticType::usampler2DMS_st,      "usampler2DMS", 1, GL_INT, sizeof(int32_t),
		SemanticType::usampler2DMSArray_st, "usampler2DMSArray", 1, GL_INT, sizeof(int32_t),

		SemanticType::usampler1DShadow_st,     "usampler1DShadow", 1, GL_INT, sizeof(int32_t),
		SemanticType::usampler2DShadow_st,     "usampler2DShadow", 1, GL_INT, sizeof(int32_t),
		SemanticType::usamplerCubeShadow_st,   "usamplerCubeShadow", 1, GL_INT, sizeof(int32_t),
		SemanticType::usampler2DRectShadow_st, "usampler2DRectShadow", 1, GL_INT, sizeof(int32_t),

		SemanticType::usampler1DArrayShadow_st,   "uSampler1dArrayShadow", 1, GL_INT, sizeof(int32_t),
		SemanticType::usampler2DArrayShadow_st,   "usampler2DArrayShadow", 1, GL_INT, sizeof(int32_t),
		SemanticType::usamplerCubeArrayShadow_st, "usamplerCubeArrayShadow", 1, GL_INT, sizeof(int32_t),

		SemanticType::unknown_st, "unknown", 0, GL_INT, sizeof(int32_t),
	};

	int semanticTypeToOpenGLElementType(SemanticType t) 
    {
		if (t > SemanticType::unknown_st)
			return GL_INT;

		unsigned int index = static_cast<typename std::underlying_type<SemanticType>::type>(t);
		return semanticTypeData[index].elementType;
	}

	int semanticTypeElementCount(SemanticType t) 
    {
		if (t > SemanticType::unknown_st)
			return 1;

		unsigned int index = static_cast<typename std::underlying_type<SemanticType>::type>(t);
		return semanticTypeData[index].count;
	}

	int semanticTypeStride(SemanticType t) 
    {
		if (t > SemanticType::unknown_st)
			return sizeof(int);

		unsigned int index = static_cast<typename std::underlying_type<SemanticType>::type>(t);
		return semanticTypeData[index].stride;
	}

	const char* semanticTypeName(SemanticType t) 
    {
		if (t > SemanticType::unknown_st)
			return semanticTypeData[static_cast<typename std::underlying_type<SemanticType>::type>(SemanticType::unknown_st)].name;

		unsigned int index = static_cast<typename std::underlying_type<SemanticType>::type>(t);
		return semanticTypeData[index].name;
	}

    bool semanticTypeIsSampler(SemanticType t)
    {
        return t >= SemanticType::sampler1D_st && t <= SemanticType::usamplerCubeArrayShadow_st;
    }

	std::string semanticTypeToString(SemanticType st) 
    {
		return std::string(semanticTypeName(st));
	}

	SemanticType semanticTypeNamed(const char* name) 
    {
		size_t l = strlen(name);
		if (l > 32)
			return SemanticType::unknown_st;

		for (int i = 0; i < sizeof(semanticTypeData) / sizeof(SemanticTypeData); ++i)
			if (!strncasecmp(name, semanticTypeData[i].name, l))
				return semanticTypeData[i].type;

		return SemanticType::unknown_st;
	}

	TextureType stringToTextureType(const std::string & s)
	{
		if (s == "f32x1") return TextureType::f32x1;
		if (s == "f32x2") return TextureType::f32x2;
		if (s == "f32x3") return TextureType::f32x3;
		if (s == "f32x4") return TextureType::f32x4;
		if (s == "f16x1") return TextureType::f16x1;
		if (s == "f16x2") return TextureType::f16x2;
		if (s == "f16x3") return TextureType::f16x3;
		if (s == "f16x4") return TextureType::f16x4;
		if (s == "u8x1") return TextureType::u8x1;
		if (s == "u8x2") return TextureType::u8x2;
		if (s == "u8x3") return TextureType::u8x3;
		if (s == "u8x4") return TextureType::u8x4;
		if (s == "s8x1") return TextureType::s8x1;
		if (s == "s8x2") return TextureType::s8x2;
		if (s == "s8x3") return TextureType::s8x3;
		if (s == "s8x4") return TextureType::s8x4;
		return TextureType::s8x4;
	}

	AutomaticUniform stringToAutomaticUniform(const std::string & s)
	{
		if (s == "resolution")
			return AutomaticUniform::frameBufferResolution;
		else if (s == "sky_matrix")
			return AutomaticUniform::skyMatrix;
		else if (s == "render_time")
			return AutomaticUniform::renderTime;
		else if (s == "mouse_position")
			return AutomaticUniform::mousePosition;
		return AutomaticUniform::none;
	}

	SemanticType textureTypeToSemanticType(TextureType type) 
	{
		switch (type) {
		case TextureType::f32x1: return SemanticType::float_st;
		case TextureType::f32x2: return SemanticType::vec2_st;
		case TextureType::f32x3: return SemanticType::vec3_st;
		case TextureType::f32x4: return SemanticType::vec4_st;
		case TextureType::f16x1: return SemanticType::float_st;
		case TextureType::f16x2: return SemanticType::vec2_st;
		case TextureType::f16x3: return SemanticType::vec3_st;
		case TextureType::f16x4: return SemanticType::vec4_st;
		case TextureType::u8x1:  return SemanticType::float_st;
		case TextureType::u8x2:  return SemanticType::vec2_st;
		case TextureType::u8x3:  return SemanticType::vec3_st;
		case TextureType::u8x4:  return SemanticType::vec4_st;
		case TextureType::s8x1:  return SemanticType::float_st;
		case TextureType::s8x2:  return SemanticType::vec2_st;
		case TextureType::s8x3:  return SemanticType::vec3_st;
		case TextureType::none:
		case TextureType::s8x4:  return SemanticType::vec4_st;
		default: return SemanticType::unknown_st;
		}
	}

}} // lab::Render
