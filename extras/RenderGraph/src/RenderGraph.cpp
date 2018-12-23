
#include <LabRenderGraph/LabRenderGraph.h>
#include <LabRenderTypes/Texture.h>

#include <LabText/TextScanner.h>

#include <iostream>
#include <string>
#include <sstream>

using namespace lab::Text;
using namespace lab::fx;

namespace {

StrView SkipComments(StrView str)
{
    if (*str.curr == '-' && str.sz > 0)
        str = ScanForNonWhiteSpace(ScanForBeginningOfNextLine(str));

    return str;
}



using lab::Render::SemanticType;

StrView tok_yes{"yes", 3};
StrView tok_no{"no", 2};
StrView tok_true{"true", 4};
StrView tok_false{"false", 5};

enum class RenderToken
{
    name,
    version,
    buffer, has_depth, textures,
    texture, path,
    pass,
    draw,
    clear_depth, write_depth, depth_test,
    clear_outputs,
    use_shader,
    inputs, outputs,
    shader, vsh, fsh,
    varying, uniforms,
    unknown
};

struct NamedRenderToken
{
    RenderToken token;
    StrView name;
};

NamedRenderToken tokens[] =
{
    { RenderToken::name,       {"name", 4} },
    { RenderToken::version,    {"version", 7} },
    { RenderToken::buffer,     {"buffer", 6} },
    { RenderToken::has_depth,  {"has depth", 9} },
    { RenderToken::textures,   {"textures", 8} },
    { RenderToken::texture,    {"texture", 7} },
    { RenderToken::pass,       {"pass", 4} },
    { RenderToken::path,       {"path", 4} },
    { RenderToken::shader,     {"shader", 6} },
    { RenderToken::use_shader, {"use shader", 10} },
    { RenderToken::draw,       {"draw", 4} },
    { RenderToken::inputs,     {"inputs", 6} },
    { RenderToken::outputs,    {"outputs", 7} },
    { RenderToken::clear_depth, {"clear depth", 11} },
    { RenderToken::write_depth, {"write depth", 11} },
    { RenderToken::clear_outputs, {"clear outputs", 13} },
    { RenderToken::depth_test, {"depth test", 10} },
    { RenderToken::varying,  { "varying", 7 } },
    { RenderToken::uniforms, { "uniforms", 8 } },
    { RenderToken::vsh,      { "vsh", 3 } },
    { RenderToken::fsh,      { "fsh", 3 } },
    { RenderToken::unknown,  {"unknown", 7} } // must be last
};

RenderToken parse_token(StrView str)
{
    if (!str.sz || !str.curr)
        return RenderToken::unknown;

    NamedRenderToken* curr = &tokens[0];
    while (curr->token != RenderToken::unknown)
    {
        if (curr->name == str)
            return curr->token;
        ++curr;
    }

    return RenderToken::unknown;
}

#ifdef _MSC_VER
#define strncasecmp _strnicmp
#endif

struct SemanticTypeData {
    SemanticType type; const char* name; int count; int stride;
};

SemanticTypeData semanticTypeData[] = {
    SemanticType::bool_st,   "bool",   1,  sizeof(bool),
    SemanticType::bvec2_st,  "bvec2",  2,  sizeof(bool[2]),
    SemanticType::bvec3_st,  "bvec3",  3,  sizeof(bool[3]),
    SemanticType::bvec4_st,  "bvec4",  4,  sizeof(bool[4]),
    SemanticType::int_st,    "int",    1,  sizeof(int32_t),
    SemanticType::ivec2_st,  "ivec2",  2,  sizeof(int32_t[2]),
    SemanticType::ivec3_st,  "ivec3",  3,  sizeof(int32_t[3]),
    SemanticType::ivec4_st,  "ivec4",  4,  sizeof(int32_t[4]),
    SemanticType::uint_st,   "uint",   1,  sizeof(uint32_t),
    SemanticType::uvec2_st,  "uvec2",  2,  sizeof(uint32_t[2]),
    SemanticType::uvec3_st,  "uvec3",  3,  sizeof(uint32_t[3]),
    SemanticType::uvec4_st,  "uvec4",  4,  sizeof(uint32_t[4]),
    SemanticType::float_st,  "float",  1,  sizeof(float),
    SemanticType::vec2_st,   "vec2",   2,  sizeof(float[2]),
    SemanticType::vec3_st,   "vec3",   3,  sizeof(float[3]),
    SemanticType::vec4_st,   "vec4",   4,  sizeof(float[4]),
    SemanticType::double_st, "double", 1,  sizeof(double),
    SemanticType::dvec2_st,  "dvec2",  2,  sizeof(double[2]),
    SemanticType::dvec3_st,  "dvec3",  3,  sizeof(double[3]),
    SemanticType::dvec4_st,  "dvec4",  4,  sizeof(double[4]),
    SemanticType::mat2_st,   "mat2",   4,  sizeof(float[4]),
    SemanticType::mat3_st,   "mat3",   9,  sizeof(float[9]),
    SemanticType::mat4_st,   "mat4",   16, sizeof(float[16]),
    SemanticType::mat2x3_st, "mat2x3", 6,  sizeof(float[6]),
    SemanticType::mat2x4_st, "mat2x4", 8,  sizeof(float[8]),
    SemanticType::mat3x2_st, "mat3x2", 6,  sizeof(float[6]),
    SemanticType::mat3x4_st, "mat3x4", 12, sizeof(float[12]),
    SemanticType::mat4x2_st, "mat4x2", 8,  sizeof(float[8]),
    SemanticType::mat4x3_st, "mat4x3", 12, sizeof(float[12]),
    SemanticType::atomic_uint_st, "atomic_uint", 1, sizeof(int32_t),

    SemanticType::sampler1D_st,   "sampler1D",   1, sizeof(int32_t),
    SemanticType::sampler2D_st,   "sampler2D",   1, sizeof(int32_t),
    SemanticType::sampler3D_st,   "sampler3D",   1, sizeof(int32_t),
    SemanticType::samplerCube_st, "samplerCube", 1, sizeof(int32_t),

    SemanticType::sampler2DRect_st,    "sampler2DRect", 1, sizeof(int32_t),
    SemanticType::sampler1DArray_st,   "sampler1DArray", 1, sizeof(int32_t),
    SemanticType::sampler2DArray_st,   "sampler2DArray", 1, sizeof(int32_t),
    SemanticType::samplerCubeArray_st, "samplerCubeArray", 1, sizeof(int32_t),

    SemanticType::samplerBuffer_st,    "samplerBuffer", 1, sizeof(int32_t),
    SemanticType::sampler2DMS_st,      "sampler2DMS", 1, sizeof(int32_t),
    SemanticType::sampler2DMSArray_st, "sampler2DMSArray", 1, sizeof(int32_t),

    SemanticType::sampler1DShadow_st,     "sampler1DShadow", 1, sizeof(int32_t),
    SemanticType::sampler2DShadow_st,     "sampler2DShadow", 1, sizeof(int32_t),
    SemanticType::samplerCubeShadow_st,   "samplerCubeShadow", 1, sizeof(int32_t),
    SemanticType::sampler2DRectShadow_st, "sampler2DRectShadow", 1, sizeof(int32_t),

    SemanticType::sampler1DArrayShadow_st,   "Sampler1DArrayShadow", 1, sizeof(int32_t),
    SemanticType::sampler2DArrayShadow_st,   "sampler2DArrayShadow", 1, sizeof(int32_t),
    SemanticType::samplerCubeArrayShadow_st, "samplerCubeArrayShadow", 1, sizeof(int32_t),

    SemanticType::isampler1D_st,   "isampler1D", 1, sizeof(int32_t),
    SemanticType::isampler2D_st,   "isampler2D", 1, sizeof(int32_t),
    SemanticType::isampler3D_st,   "isampler3D", 1, sizeof(int32_t),
    SemanticType::isamplerCube_st, "isamplerCube", 1, sizeof(int32_t),

    SemanticType::isampler2DRect_st,    "isampler2DRect", 1, sizeof(int32_t),
    SemanticType::isampler1DArray_st,   "isampler1DArray", 1, sizeof(int32_t),
    SemanticType::isampler2DArray_st,   "isampler2DArray", 1, sizeof(int32_t),
    SemanticType::isamplerCubeArray_st, "isamplerCubeArray", 1, sizeof(int32_t),

    SemanticType::isamplerBuffer_st,    "isamplerBuffer", 1, sizeof(int32_t),
    SemanticType::isampler2DMS_st,      "isampler2DMS", 1, sizeof(int32_t),
    SemanticType::isampler2DMSArray_st, "isampler2DMSArray", 1, sizeof(int32_t),

    SemanticType::isampler1DShadow_st,     "isampler1DShadow", 1, sizeof(int32_t),
    SemanticType::isampler2DShadow_st,     "isampler2DShadow", 1, sizeof(int32_t),
    SemanticType::isamplerCubeShadow_st,   "isamplerCubeShadow", 1, sizeof(int32_t),
    SemanticType::isampler2DRectShadow_st, "isampler2DRectShadow", 1, sizeof(int32_t),

    SemanticType::isampler1dArrayShadow_st,   "iSampler1dArrayShadow", 1, sizeof(int32_t),
    SemanticType::isampler2DArrayShadow_st,   "isampler2DArrayShadow", 1, sizeof(int32_t),
    SemanticType::isamplerCubeArrayShadow_st, "isamplerCubeArrayShadow", 1, sizeof(int32_t),

    SemanticType::usampler1D_st,   "usampler1D", 1, sizeof(int32_t),
    SemanticType::usampler2D_st,   "usampler2D", 1, sizeof(int32_t),
    SemanticType::usampler3D_st,   "usampler3D", 1, sizeof(int32_t),
    SemanticType::usamplerCube_st, "usamplerCube", 1, sizeof(int32_t),

    SemanticType::usampler2DRect_st,    "usampler2DRect", 1, sizeof(int32_t),
    SemanticType::usampler1DArray_st,   "usampler1DArray", 1, sizeof(int32_t),
    SemanticType::usampler2DArray_st,   "usampler2DArray", 1, sizeof(int32_t),
    SemanticType::usamplerCubeArray_st, "usamplerCubeArray", 1, sizeof(int32_t),

    SemanticType::usamplerBuffer_st,    "usamplerBuffer", 1, sizeof(int32_t),
    SemanticType::usampler2DMS_st,      "usampler2DMS", 1, sizeof(int32_t),
    SemanticType::usampler2DMSArray_st, "usampler2DMSArray", 1, sizeof(int32_t),

    SemanticType::usampler1DShadow_st,     "usampler1DShadow", 1, sizeof(int32_t),
    SemanticType::usampler2DShadow_st,     "usampler2DShadow", 1, sizeof(int32_t),
    SemanticType::usamplerCubeShadow_st,   "usamplerCubeShadow", 1, sizeof(int32_t),
    SemanticType::usampler2DRectShadow_st, "usampler2DRectShadow", 1, sizeof(int32_t),

    SemanticType::usampler1DArrayShadow_st,   "uSampler1dArrayShadow", 1, sizeof(int32_t),
    SemanticType::usampler2DArrayShadow_st,   "usampler2DArrayShadow", 1, sizeof(int32_t),
    SemanticType::usamplerCubeArrayShadow_st, "usamplerCubeArrayShadow", 1, sizeof(int32_t),

    SemanticType::unknown_st, "unknown", 0, sizeof(int32_t),
};

int semanticTypeElementCount(SemanticType t)
{
    if (t >= SemanticType::unknown_st)
        return 1;

    unsigned int index = static_cast<typename std::underlying_type<SemanticType>::type>(t);
    return semanticTypeData[index].count;
}

int semanticTypeStride(SemanticType t)
{
    if (t >= SemanticType::unknown_st)
        return sizeof(int);

    unsigned int index = static_cast<typename std::underlying_type<SemanticType>::type>(t);
    return semanticTypeData[index].stride;
}

const char* semanticTypeName(SemanticType t)
{
    if (t >= SemanticType::unknown_st)
        return semanticTypeData[static_cast<typename std::underlying_type<SemanticType>::type>(SemanticType::unknown_st)].name;

    unsigned int index = static_cast<typename std::underlying_type<SemanticType>::type>(t);
    return semanticTypeData[index].name;
}


std::string semanticTypeToString(SemanticType st)
{
    return std::string(semanticTypeName(st));
}

SemanticType semanticTypeNamed(StrView name)
{
    if (name.sz > 32)
        return SemanticType::unknown_st;

    for (int i = 0; i < sizeof(semanticTypeData) / sizeof(SemanticTypeData); ++i)
        if (!strncasecmp(name.curr, semanticTypeData[i].name, name.sz))
            return semanticTypeData[i].type;

    return SemanticType::unknown_st;
}


lab::Render::DepthTest depth_test_from_str(StrView str)
{
    using lab::Render::DepthTest;
    static StrView tok_less{"less", 4};
    if (str == tok_less) return DepthTest::less;
    static StrView tok_lequal{"lequal", 6};
    if (str == tok_lequal) return DepthTest::lequal;
    static StrView tok_never{"never", 5};
    if (str == tok_never) return DepthTest::never;
    static StrView tok_equal{"equal", 5};
    if (str == tok_equal) return DepthTest::equal;
    static StrView tok_greater{"greater", 7};
    if (str == tok_greater) return DepthTest::greater;
    static StrView tok_notequal{"notequal", 8};
    if (str == tok_notequal) return DepthTest::notequal;
    static StrView tok_gequal{"gequal", 6};
    if (str == tok_gequal) return DepthTest::gequal;
    static StrView tok_always{"always", 6};
    if (str == tok_always) return DepthTest::always;
    return DepthTest::always;
}

pass_draw pass_draw_from_str(StrView str)
{
    static StrView tok_opaque_geometry {"opaque geometry", 15};
    if (str == tok_opaque_geometry) return pass_draw::opaque_geometry;
    static StrView tok_quad {"quad", 4};
    if (str == tok_quad) return pass_draw::quad;
    return pass_draw::none;
}

lab::Render::TextureType TextureType_from_str(StrView str)
{
    using lab::Render::TextureType;
    static StrView tok_f32x1{"f32x1", 5};
    if (str == tok_f32x1) return TextureType::f32x1;
    static StrView tok_f32x2{"f32x2", 5};
    if (str == tok_f32x2) return TextureType::f32x2;
    static StrView tok_f32x3{"f32x3", 5};
    if (str == tok_f32x3) return TextureType::f32x3;
    static StrView tok_f32x4{"f32x4", 5};
    if (str == tok_f32x4) return TextureType::f32x4;
    static StrView tok_f16x1{"f16x1", 5};
    if (str == tok_f16x1) return TextureType::f16x1;
    static StrView tok_f16x2{"f16x2", 5};
    if (str == tok_f16x2) return TextureType::f16x2;
    static StrView tok_f16x3{"f16x3", 5};
    if (str == tok_f16x3) return TextureType::f16x3;
    static StrView tok_f16x4{"f16x4", 5};
    if (str == tok_f16x4) return TextureType::f16x4;
    static StrView tok_u8x1{"u8x1", 4};
    if (str == tok_u8x1) return TextureType::u8x1;
    static StrView tok_u8x2{"u8x2", 4};
    if (str == tok_u8x2) return TextureType::u8x2;
    static StrView tok_u8x3{"u8x3", 4};
    if (str == tok_u8x3) return TextureType::u8x3;
    static StrView tok_u8x4{"u8x4", 4};
    if (str == tok_u8x4) return TextureType::u8x4;
    static StrView tok_s8x1{"s8x1", 4};
    if (str == tok_s8x1) return TextureType::s8x1;
    static StrView tok_s8x2{"s8x2", 4};
    if (str == tok_s8x2) return TextureType::s8x2;
    static StrView tok_s8x3{"s8x3", 4};
    if (str == tok_s8x3) return TextureType::s8x3;
    static StrView tok_s8x4{"s8x4", 4};
    if (str == tok_s8x4) return TextureType::s8x4;
    return TextureType::none;
}

StrView parse_uniforms(StrView curr, std::vector<uniform>& uniforms)
{
    curr = ScanForNonWhiteSpace(Expect(ScanForNonWhiteSpace(curr), StrView{":", 1}));
    if (*curr.curr != '[')
        return curr;
    curr = Expect(curr, StrView{"[", 1});

    while (curr.sz > 0)
    {
        curr = ScanForNonWhiteSpace(curr);
        if (!curr.sz)
            break;

        if (*curr.curr == ']')
        {
            curr = Expect(curr, StrView{ "]", 1 });
            break;
        }

        StrView name;
        curr = ScanForNonWhiteSpace(GetToken(curr, ':', name));
        curr = Expect(curr, StrView{":", 1});
        StrView type;
        curr = ScanForNonWhiteSpace(GetTokenAlphaNumeric(curr, type));
        SemanticType st = semanticTypeNamed(type);

        // automatic uniform marker
        StrView automatic {"", 0};
        if (*curr.curr == '<')
        {
            curr = Expect(curr, StrView{ "<", 1 });
            if (*curr.curr == '-')
            {
                curr = Expect(curr, StrView{ "-", 1 });
                curr = ScanForNonWhiteSpace(GetTokenAlphaNumericExt(curr, "-_", automatic));
            }
        }
        uniforms.push_back(uniform{ std::string(name.curr, name.sz), st, std::string(automatic.curr, automatic.sz) });

        if (*curr.curr == ',')  // comma is optional
        {
            curr = Expect(curr, StrView{ ",", 1 });
        }
    }
    return curr;
}

StrView parse_shader(StrView curr, shader& prg, std::string& source)
{
    curr = Expect(curr, StrView{":", 1});
    while (curr.sz > 0)
    {
        curr = ScanForNonWhiteSpace(curr);
        if (!curr.sz)
            break;

        static StrView tok_source{ "source", 6 };
        static StrView tok_attributes{ "attributes", 10 };

        StrView str_token;
        StrView next = ScanForNonWhiteSpace(GetToken(curr, ':', str_token));
        if (str_token == tok_source)
        {
            next = Expect(next, StrView{":", 1});
            next = ScanForNonWhiteSpace(next);
            if (*next.curr != '`')
                return curr;

            // could parse for slang (```glsl, etc) here
            next = ScanForNonWhiteSpace(ScanForBeginningOfNextLine(next));
            StrView src = next;
            while (next.sz > 0)
            {
                next = ScanForBeginningOfNextLine(next);
                StrView tmp = ScanForNonWhiteSpace(next);
                if (*tmp.curr == '`')
                {
                    // at the end of the source block, assign the source
                    src.sz = tmp.curr - src.curr;
                    source = std::string{ src.curr, src.sz };
                    next = ScanForBeginningOfNextLine(tmp);
                    break;
                }
            }
        }
        else if (str_token == tok_attributes)
        {
            next = parse_uniforms(next, prg.attributes);
        }
        else
            return curr;

        curr = next;
    }
    return curr;
}

StrView parse_pass(StrView start, labfx& fx)
{
    RenderToken token = RenderToken::pass;
    StrView str_token;
    StrView curr = ScanForEndOfLine(start, str_token);
    str_token = ScanForNonWhiteSpace(str_token);
    str_token = Expect(str_token, StrView{":", 1});
    str_token = Strip(ScanForNonWhiteSpace(str_token));
    fx.passes.back().name = std::string(str_token.curr, str_token.sz);

    bool in_pass = true;
    while(in_pass && curr.sz > 0)
    {
        curr = ScanForNonWhiteSpace(curr);
        if (!curr.sz)
            break;

        if (*curr.curr == '-')
        {
            curr = SkipComments(curr);
            continue;
        }

        StrView restore = curr;
        curr = ScanForNonWhiteSpace(GetToken(curr, ':', str_token));
        token = parse_token(str_token);
        if (token == RenderToken::unknown)
        {
            curr = restore;
            break;
        }

        switch(token)
        {
        case RenderToken::use_shader:
            curr = ScanForEndOfLine(curr, str_token);
            str_token = ScanForNonWhiteSpace(str_token);
            str_token = Expect(str_token, StrView{":", 1});
            str_token = Strip(ScanForNonWhiteSpace(str_token));
            fx.passes.back().shader = std::string(str_token.curr, str_token.sz);
            break;

        case RenderToken::draw:
            curr = ScanForEndOfLine(curr, str_token);
            str_token = ScanForNonWhiteSpace(str_token);
            str_token = Expect(str_token, StrView{":", 1});
            str_token = Strip(ScanForNonWhiteSpace(str_token));
            fx.passes.back().draw = pass_draw_from_str(str_token);
            break;

        case RenderToken::clear_depth:
            curr = ScanForEndOfLine(curr, str_token);
            str_token = ScanForNonWhiteSpace(str_token);
            str_token = Expect(str_token, StrView{":", 1});
            str_token = Strip(ScanForNonWhiteSpace(str_token));
            fx.passes.back().clear_depth = str_token == tok_yes || str_token == tok_true;
            break;

        case RenderToken::write_depth:
            curr = ScanForEndOfLine(curr, str_token);
            str_token = ScanForNonWhiteSpace(str_token);
            str_token = Expect(str_token, StrView{":", 1});
            str_token = Strip(ScanForNonWhiteSpace(str_token));
            fx.passes.back().write_depth = str_token == tok_yes || str_token == tok_true;
            break;

        case RenderToken::clear_outputs:
            curr = ScanForEndOfLine(curr, str_token);
            str_token = ScanForNonWhiteSpace(str_token);
            str_token = Expect(str_token, StrView{":", 1});
            str_token = Strip(ScanForNonWhiteSpace(str_token));
            fx.passes.back().clear_outputs = str_token == tok_yes || str_token == tok_true;
            break;

        case RenderToken::depth_test:
            curr = ScanForEndOfLine(curr, str_token);
            str_token = ScanForNonWhiteSpace(str_token);
            str_token = Expect(str_token, StrView{":", 1});
            str_token = Strip(ScanForNonWhiteSpace(str_token));
            fx.passes.back().test = depth_test_from_str(str_token);
            break;

        case RenderToken::inputs:
            {
                curr = ScanForNonWhiteSpace(curr);
                curr = Expect(curr, StrView{":", 1});
                curr = ScanForNonWhiteSpace(curr);
                str_token = Strip(ScanForNonWhiteSpace(str_token));
                StrView advance = Expect(ScanForNonWhiteSpace(curr), StrView{"[", 1});
                if (advance.curr != curr.curr)
                {
                    curr = advance;
                    while (curr.sz > 0)
                    {
                        curr = ScanForNonWhiteSpace(curr);
                        StrView buffer_name;
                        curr = ScanForNonWhiteSpace(GetToken(curr, '.', buffer_name));
                        curr = Expect(curr, StrView{".", 1});
                        StrView texture_name;
                        curr = ScanForNonWhiteSpace(GetTokenAlphaNumeric(curr, texture_name));
                        fx.passes.back().input_textures.push_back({std::string{buffer_name.curr, buffer_name.sz},
                                                                   std::string{texture_name.curr, texture_name.sz}});
                        if (*curr.curr == ',')
                        {
                            curr = Expect(curr, StrView{",", 1});
                        }
                        else if (*curr.curr == ']')
                        {
                            curr = Expect(curr, StrView{"]", 1});
                            break;
                        }
                    }
                }
            }
            break;

        case RenderToken::outputs:
            {
                curr = ScanForNonWhiteSpace(curr);
                curr = Expect(curr, StrView{":", 1});
                StrView buffer_name;
                curr = GetTokenAlphaNumeric(curr, buffer_name);
                fx.passes.back().output_buffer.assign(buffer_name.curr, buffer_name.sz);
                curr = SkipComments(ScanForNonWhiteSpace(curr));
                if (curr.sz && *curr.curr == '[')
                {
                    curr = Expect(curr, StrView{"[", 1});
                    while (curr.sz > 0)
                    {
                        curr = ScanForNonWhiteSpace(curr);
                        StrView texture_name;
                        curr = ScanForNonWhiteSpace(GetTokenAlphaNumeric(curr, texture_name));
                        fx.passes.back().output_textures.push_back(std::string{texture_name.curr, texture_name.sz});
                        if (*curr.curr == ',')
                            curr = Expect(curr, StrView{",", 1});
                        else if (*curr.curr == ']')
                        {
                            curr = Expect(curr, StrView{"]", 1});
                            break;
                        }
                    }
                }
            }
            break;

        default:
            in_pass = false;
            curr = restore;
            break;
        }
    }
    return curr;
}

void Report(char const*const msg, StrView detail)
{
    std::cerr << msg << std::string(detail.curr, detail.sz) << std::endl;
}

} // anon


LRG_API labfx_t* parse_labfx(char const*const input, size_t length)
{
    labfx* fx_ptr = new labfx;
    labfx& fx = *fx_ptr;

    enum class Mode
    {
        root, buffer, pass, shader, texture
    };

    StrView tok_scale{"scale", 5};

    StrView curr{input, length};
    Mode mode = Mode::root;

    bool error_raised = false;
    while(curr.sz > 0 && !error_raised)
    {
        curr = ScanForNonWhiteSpace(curr);
        if (!curr.sz)
            break;

        if (*curr.curr == '-')
        {
            curr = SkipComments(curr);
            continue;
        }

        StrView str_token;
        curr = ScanForNonWhiteSpace(GetToken(curr, ':', str_token));
        RenderToken token = parse_token(str_token);
        if (token == RenderToken::unknown)
        {
            Report("Unknown token: ", str_token);
            error_raised = true;
            break;
        }

        if (token == RenderToken::buffer)
        {
            mode = Mode::buffer;
            fx.buffers.emplace_back(buffer{});
        }
        else if (token == RenderToken::pass)
        {
            mode = Mode::pass;
            fx.passes.emplace_back(pass{});
        }
        else if (token == RenderToken::shader)
        {
            mode = Mode::shader;
            fx.shaders.emplace_back(shader{});
        }
        else if (token == RenderToken::texture)
        {
            mode = Mode::texture;
            fx.textures.emplace_back(texture{});
        }

        switch(mode)
        {
        case Mode::shader:
            switch(token)
            {
            case RenderToken::shader:
                curr = ScanForEndOfLine(curr, str_token);
                str_token = Strip(str_token);
                str_token = Expect(str_token, StrView{":", 1});
                str_token = Strip(str_token);
                fx.shaders.back().name = std::string(str_token.curr, str_token.sz);
                break;

            case RenderToken::vsh:
                curr = parse_shader(curr, fx.shaders.back(), fx.shaders.back().vsh_source);
                break;

            case RenderToken::fsh:
                curr = parse_shader(curr, fx.shaders.back(), fx.shaders.back().fsh_source);
                break;

            case RenderToken::varying:
                curr = parse_uniforms(curr, fx.shaders.back().varyings);
                break;

            case RenderToken::uniforms:
                curr = parse_uniforms(curr, fx.shaders.back().uniforms);
                break;
            }
            break;

        case Mode::pass:
            curr = parse_pass(curr, fx);
            mode = Mode::root;
            break;

        case Mode::texture:
            switch(token)
            {
                case RenderToken::texture:
                    curr = ScanForEndOfLine(curr, str_token);
                    str_token = Strip(str_token);
                    str_token = Expect(str_token, StrView{":", 1});
                    str_token = Strip(str_token);
                    fx.textures.back().name = std::string(str_token.curr, str_token.sz);
                    break;

                case RenderToken::path:
                    curr = ScanForEndOfLine(curr, str_token);
                    str_token = Strip(str_token);
                    str_token = Expect(str_token, StrView{":", 1});
                    str_token = Strip(str_token);
                    fx.textures.back().path = std::string(str_token.curr, str_token.sz);
                    break;
            }
            break;

        case Mode::root:
            switch(token)
            {
            case RenderToken::name:
                curr = ScanForEndOfLine(curr, str_token);
                str_token = ScanForNonWhiteSpace(str_token);
                str_token = Expect(str_token, StrView{":", 1});
                str_token = Strip(str_token);
                fx.name = std::string(str_token.curr, str_token.sz);
                break;

            case RenderToken::version:
                curr = ScanForEndOfLine(curr, str_token);
                str_token = ScanForNonWhiteSpace(str_token);
                str_token = Expect(str_token, StrView{":", 1});
                str_token = Strip(str_token);
                fx.version = std::string(str_token.curr, str_token.sz);
                break;

            case RenderToken::unknown:
                break;
            }
            break;

        case Mode::buffer:
            switch(token)
            {
            case RenderToken::buffer:
                curr = ScanForEndOfLine(curr, str_token);
                str_token = ScanForNonWhiteSpace(str_token);
                str_token = Expect(str_token, StrView{":", 1});
                str_token = Strip(str_token);
                fx.buffers.back().name = std::string(str_token.curr, str_token.sz);
                break;

            case RenderToken::has_depth:
                curr = ScanForEndOfLine(curr, str_token);
                str_token = ScanForNonWhiteSpace(str_token);
                str_token = Expect(str_token, StrView{":", 1});
                str_token = Strip(str_token);
                if (str_token != tok_yes && str_token != tok_no && str_token != tok_true && str_token != tok_false)
                {
                    Report("has_depth invalid argument: ", str_token);
                    error_raised = true;
                }
                else
                    fx.buffers.back().has_depth = str_token == tok_yes || str_token == tok_true;
                break;

            case RenderToken::textures:
            {
                curr = ScanForNonWhiteSpace(curr);
                curr = Expect(curr, StrView{":", 1});
                curr = ScanForNonWhiteSpace(curr);
                if (*curr.curr == '[')
                {
                    curr = Expect(curr, StrView{"[", 1});
                    do
                    {
                        fx.buffers.back().textures.emplace_back(texture{});
                        texture& curr_texture = fx.buffers.back().textures.back();

                        curr = ScanForEndOfLine(curr, str_token);

                        auto crumbs = Split(str_token, ',');
                        for (int i = 0; i < crumbs.size(); ++i)
                            crumbs[i] = Strip(crumbs[i]);

                        if (crumbs.size() > 0)
                        {
                            curr_texture.name = std::string{crumbs[0].curr, crumbs[0].sz};
                        }

                        for (size_t i = 1; i < crumbs.size(); ++i)
                        {
                            auto tx_t = TextureType_from_str(crumbs[i]);
                            if (tx_t != lab::Render::TextureType::none)
                                curr_texture.format = tx_t;
                            else
                            {
                                auto scale_crumbs = Split(crumbs[i], ':');
                                if (scale_crumbs.size() > 1 && scale_crumbs[0] == tok_scale)
                                {
                                    float s;
                                    GetFloat(scale_crumbs[1], s);
                                    curr_texture.scale = s;
                                }
                                else
                                {
                                    Report("unknown texture token: ", crumbs[i]);
                                    error_raised = true;
                                    break;
                                }
                            }
                        }

                        StrView eol = ScanForCharacter(str_token, ']');
                        if (eol.sz > 0)
                            break;
                    }
                    while (curr.sz > 0);
                }
            }
            break;

            case RenderToken::unknown:
                break;
            }
            break;
        }
    }
    if (error_raised)
    {
        delete fx_ptr;
        return nullptr;
    }
    return reinterpret_cast<labfx_t*>(fx_ptr);
}

void free_labfx(labfx_t* fx)
{
    labfx* fx_ptr = reinterpret_cast<labfx*>(fx);
    delete fx_ptr;
}

static std::string compile(const shader& prg, const std::string& source, const std::string& inout)
{
    std::stringstream ss;
    ss << "\
#version 410\n\
#extension GL_ARB_explicit_attrib_location: enable\n\
#extension GL_ARB_separate_shader_objects: enable\n\
#define texture2D texture\n";

    int loc = 0;
    for (const auto& a : prg.attributes)
    {
        ss << "layout(location = " << loc++ << ") in ";
        ss << semanticTypeToString(a.type) << " " << a.name << ";\n";
    }

    for (const auto& a : prg.uniforms)
        ss << "uniform " << semanticTypeToString(a.type) << " " << a.name << ";\n";

    if (prg.varyings.size() > 0)
    {
        ss << inout << " Var {\n";
        for (const auto& a : prg.varyings)
            ss << "   " << semanticTypeToString(a.type) << " " << a.name << ";" << std::endl;
        ss << "} var;\n";
    }
    ss << source << std::endl;
    return ss.str();
}

labfx_gen_t* generate_shaders(labfx_t* fx)
{
    labfx* fx_ptr = reinterpret_cast<labfx*>(fx);
    if (!fx_ptr)
        return nullptr;

    generated_shaders* gen = new generated_shaders();
    for (const auto& sh : fx_ptr->shaders)
    {
        gen->vsh.push_back(compile(sh, sh.vsh_source, "out"));
        gen->fsh.push_back(compile(sh, sh.fsh_source, "in"));
    }
    return reinterpret_cast<labfx_gen_t*>(gen);
}

void free_labfx_gen(labfx_gen_t* t)
{
    generated_shaders* gen = reinterpret_cast<generated_shaders*>(t);;
    delete gen;
}
