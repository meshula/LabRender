//
//  SemanticType.h
//  LabApp
//
//  Created by Nick Porcino on 2014 02/28.
//  Copyright (c) 2014 Nick Porcino. All rights reserved.
//

#pragma once

#include <LabRender/LabRender.h>
#include <set>
#include <string>
#include <vector>

namespace lab { namespace Render {

    // https://www.opengl.org/wiki/Data_Type_(GLSL)
    // https://www.opengl.org/wiki/Sampler_(GLSL)
    // https://www.opengl.org/wiki/Atomic_Counter
    // https://www.opengl.org/wiki/Image_Load_Store

    enum class SemanticType : unsigned int {
        bool_st = 0, bvec2_st, bvec3_st, bvec4_st,
        int_st, ivec2_st, ivec3_st, ivec4_st,
        uint_st, uvec2_st, uvec3_st, uvec4_st,
        float_st, vec2_st, vec3_st, vec4_st,
        double_st, dvec2_st, dvec3_st, dvec4_st,
        mat2_st, mat3_st, mat4_st,
        mat2x3_st, mat2x4_st,
        mat3x2_st, mat3x4_st,
        mat4x2_st, mat4x3_st,
        atomic_uint_st,

        sampler1D_st, sampler2D_st, sampler3D_st, samplerCube_st,
        sampler2DRect_st, sampler1DArray_st, sampler2DArray_st, samplerCubeArray_st,
        samplerBuffer_st, sampler2DMS_st, sampler2DMSArray_st,
        sampler1DShadow_st, sampler2DShadow_st, samplerCubeShadow_st, sampler2DRectShadow_st,
        sampler1DArrayShadow_st, sampler2DArrayShadow_st, samplerCubeArrayShadow_st,
        isampler1D_st, isampler2D_st, isampler3D_st, isamplerCube_st,
        isampler2DRect_st, isampler1DArray_st, isampler2DArray_st, isamplerCubeArray_st,
        isamplerBuffer_st, isampler2DMS_st, isampler2DMSArray_st,
        isampler1DShadow_st, isampler2DShadow_st, isamplerCubeShadow_st, isampler2DRectShadow_st,
        isampler1dArrayShadow_st, isampler2DArrayShadow_st, isamplerCubeArrayShadow_st,
        usampler1D_st, usampler2D_st, usampler3D_st, usamplerCube_st,
        usampler2DRect_st, usampler1DArray_st, usampler2DArray_st, usamplerCubeArray_st,
        usamplerBuffer_st, usampler2DMS_st, usampler2DMSArray_st,
        usampler1DShadow_st, usampler2DShadow_st, usamplerCubeShadow_st, usampler2DRectShadow_st,
        usampler1DArrayShadow_st, usampler2DArrayShadow_st, usamplerCubeArrayShadow_st,

        string_st,

        unknown_st
    };

    bool semanticTypeIsSampler(SemanticType t);
	int semanticTypeToOpenGLElementType(SemanticType t);
	int semanticTypeElementCount(SemanticType t);
	int semanticTypeStride(SemanticType t);
	const char* semanticTypeName(SemanticType t);
	SemanticType semanticTypeNamed(const char*);
	std::string semanticTypeToString(SemanticType st);


}} //lab::Render
