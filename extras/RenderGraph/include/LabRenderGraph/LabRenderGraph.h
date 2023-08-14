
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

#include <stddef.h>

// can be reinterpret casted to labfx
struct labfx_t {};

// can be reinterpret casted to generated_shaders
struct labfx_gen_t {};

LRG_API labfx_t* parse_labfx(char const*const input, size_t length);
LRG_API void free_labfx(labfx_t*);
LRG_API labfx_gen_t* generate_shaders(labfx_t*);
LRG_API void free_labfx_gen(labfx_gen_t*);

#ifdef __cplusplus

#include <LabRenderTypes/Texture.h>
#include <LabRenderTypes/DepthTest.h>
#include <LabRenderTypes/SemanticType.h>

#include <string>
#include <vector>

namespace lab { namespace fx {

struct texture
{
    std::string name;
    std::string path;
    lab::Render::TextureType format { lab::Render::TextureType::none };
    float scale {1.f};
};

struct buffer
{
    std::string name;
    std::vector<texture> textures;
    bool has_depth {false};
};

struct buffer_select
{
    std::string name, texture;
};

enum class pass_draw
{
    none, blit, quad, opaque_geometry, plug
};

struct pass
{
    std::string name;
    std::string shader;

    pass_draw draw {pass_draw::none};
    lab::Render::DepthTest test {lab::Render::DepthTest::always};
    bool active {true};
    bool clear_depth {false};
    bool clear_outputs {false};
    bool write_depth {false};

    std::vector<buffer_select> input_textures;

    std::string output_buffer;
    std::vector<std::string> output_textures;
};

struct uniform
{
    uniform(const std::string& n, lab::Render::SemanticType st, const std::string& a)
   : name(n), type(st), automatic(a) {};

    std::string name;
    lab::Render::SemanticType type;
    std::string automatic;
};

struct generated_shaders
{
    std::vector<std::string> vsh;
    std::vector<std::string> fsh;
};

struct shader
{
    std::string name;
    std::string vsh_source;
    std::string fsh_source;
    std::vector<uniform> attributes;
    std::vector<uniform> uniforms;
    std::vector<uniform> varyings;
};

struct labfx
{
    std::string name;
    std::string version;
    std::vector<buffer>  buffers;
    std::vector<pass>    passes;
    std::vector<shader>  shaders;
    std::vector<texture> textures;
};

}} // lab::render

#endif

