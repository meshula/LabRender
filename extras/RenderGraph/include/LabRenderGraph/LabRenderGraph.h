
#pragma once


#if defined(_MSC_VER) && defined(LABRENDER_RENDERGRAPH_DLL)
# ifdef BUILDING_LabRenderGraph
#  define LRG_CAPI extern "C" __declspec(dllexport)
#  define LRG_API __declspec(dllexport)
#  define LRG_CLASS __declspec(dllexport)
# else
#  define LRG_CAPI extern "C" __declspec(dllimport)
#  define LRG_API __declspec(dllimport)
#  define LRG_CLASS __declspec(dllimport)
# endif
#else
# define LRG_API
# define LRG_CAPI
# define LRG_CLASS
#endif


LRG_API void* parse_labfx(char const*const input, size_t length);
LRG_API void free_labfx(void*);

#ifdef __cplusplus

#include <string>
#include <vector>

namespace lab { namespace Render {

enum class texture_type
{
    none,
    f32x1, f32x2, f32x3, f32x4,
    f16x1, f16x2, f16x3, f16x4,
    u8x1,  u8x2,  u8x3,  u8x4,
    s8x1,  s8x2,  s8x3,  s8x4
};

struct texture
{
    std::string name;
    texture_type format { texture_type::none };
    float scale {1.f};
};

struct buffer
{
    std::string name;
    std::vector<texture> textures;
    bool has_depth {false};
};

enum class pass_draw
{
    none, opaque_geometry, quad
};

enum class depth_test
{
    less, lequal, never, equal, greater, notequal, gequal, always, none
};

struct pass
{
    std::string name;
    std::string shader;
    pass_draw draw {pass_draw::none};
    depth_test test {depth_test::none};
    bool clear_depth {false};
    bool clear_outputs {false};
    bool write_depth {false};
    std::vector<std::pair<std::string, std::string>> inputs;
    std::vector<std::pair<std::string, std::string>> outputs;
};


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


struct uniform
{
    uniform(const std::string& n, SemanticType st, const std::string& a)
   : name(n), type(st), automatic(a) {};

    std::string name;
    SemanticType type;
    std::string automatic;
};

enum class program_type
{
    vsh, fsh, none
};

struct program
{
    program_type type {program_type::none};
    std::vector<uniform> uniforms;
    std::string source;
};

struct shader
{
    std::string name;
    program vsh { program_type::vsh };
    program fsh { program_type::fsh };
    std::vector<uniform> varyings;
};

struct labfx
{
    std::string name;
    std::string version;
    std::vector<buffer> buffers;
    std::vector<pass> passes;
    std::vector<shader> shaders;
};

}} // lab::render

#endif

