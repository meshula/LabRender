//
//  ShaderBuilder.cpp
//  LabApp
//
//  Created by Nick Porcino on 2014 01/28.
//  Copyright (c) 2014 Nick Porcino. All rights reserved.
//

#include "LabRender/ShaderBuilder.h"
#include "LabRender/Model.h"
#include "LabRender/FrameBuffer.h"

#include <set>
#include <map>
#include <string>
#include <sstream>
#include "gl4.h"
#include "LabRender/Utils.h"

namespace lab { namespace Render {

using namespace std;

ShaderBuilder::Cache* _cache()
{
    static std::once_flag once;
    static ShaderBuilder::Cache* _shaderCache = 0;
    std::call_once(once, []() { _shaderCache = new ShaderBuilder::Cache(); });
    return _shaderCache;
}

class ShaderBuilder::Cache::Detail
{
public:
    Detail() { }
    std::map<std::string, std::shared_ptr<Shader>> shaders;
};

ShaderBuilder::Cache::Cache() : _detail(new Detail()) { }
ShaderBuilder::Cache::~Cache() { delete _detail.get(); }

bool ShaderBuilder::Cache::hasShader(const std::string& shader) const
{
    Cache *cache = _cache();
    return cache->_detail->shaders.find(shader) != _detail->shaders.end();
}

void ShaderBuilder::Cache::add(const std::string& name, std::shared_ptr<Shader> shader)
{
    Cache *cache = _cache();
    cache->_detail->shaders[name] = shader;
}

std::shared_ptr<Shader> ShaderBuilder::Cache::shader(const std::string& name) const
{
    if (!hasShader(name))
        return std::shared_ptr<Shader>();

    Cache *cache = _cache();
    return cache->_detail->shaders.find(name)->second;
}

ShaderBuilder::Cache* ShaderBuilder::cache()
{
    return _cache();
}

const char* preamble()
{
    return "\
#version 410\n\
#extension GL_ARB_explicit_attrib_location: enable\n\
#extension GL_ARB_separate_shader_objects: enable\n\
#define texture2D texture\n";
}


std::string generateFragment()
{
    std::stringstream s;
    s << preamble();
    return s.str();
}


ShaderBuilder::~ShaderBuilder()
{
    clear();
}

void ShaderBuilder::clear()
{
    for (auto i : uniforms) delete i;   uniforms.clear();
    for (auto i : attributes) delete i.second; attributes.clear();
    for (auto i : varyings) delete i;   varyings.clear();
    for (auto i : outputs) delete i;    outputs.clear();
}

void ShaderBuilder::setFrameBufferOutputs(const FrameBuffer& fbo, const std::vector<std::string>& output_attachments)
{
    // if no draw buffers specified on the fbo, then the fbo is representing the default gl draw buffer
    if (fbo.baseNames.size() == 0)
        outputs.insert(new Semantic(SemanticType::vec4_st, "color", 0));
    else if (false)
    {
        //&&&
        for (int i = 0; i < fbo.drawBufferNames.size(); ++i)
        {
            outputs.insert(new Semantic(fbo.samplerType[i], fbo.drawBufferNames[i].c_str(), i));
        }
    }
    else
        for (auto& s : output_attachments)
        {
            for (int i = 0; i < fbo.baseNames.size(); ++i)
            {
                std::string n = "o_" + s + "_texture";
                if (n == fbo.drawBufferNames[i])
                {
                    outputs.insert(new Semantic(fbo.samplerType[i], fbo.drawBufferNames[i].c_str(), i));
                    break;
                }
            }
        }
}

void ShaderBuilder::setAttributes(const ModelPart& mesh)
{
    VAO* vao = mesh.verts();
    vao->uploadVerts();
    for (int i = 0; i < vao->attributes.size(); ++i)
        attributes[vao->attributes[i].name] = new Semantic(vao->attributes[i]);
}

void ShaderBuilder::setAttributes(const ShaderSpec& spec)
{
    std::set<Semantic*> u = Semantic::makeSemantics(spec.attributes);
    for (auto i : u)
        attributes[i->name] = i;
}

void ShaderBuilder::setUniforms(const ShaderSpec& spec)
{
    std::set<Semantic*> u = Semantic::makeSemantics(spec.uniforms);
    for (auto i : u)
        uniforms.insert(i);
}

void ShaderBuilder::setVaryings(const ShaderSpec& spec)
{
    std::set<Semantic*> u = Semantic::makeSemantics(spec.varyings);
    for (auto i : u)
        varyings.insert(i);
}

void ShaderBuilder::setSamplers(const ShaderSpec& spec)
{
    std::set<Semantic*> u = Semantic::makeSemantics(spec.samplers);
    for (auto i : u)
        samplers.insert(i);
}

void ShaderBuilder::setUniforms(Semantic const*const semantics, size_t count)
{
    for (size_t i = 0; i < count; ++i)
        uniforms.insert(new Semantic(semantics[i]));
}

void ShaderBuilder::setVaryings(Semantic const*const semantics, size_t count)
{
    for (size_t i = 0; i < count; ++i)
        varyings.insert(new Semantic(semantics[i]));
}

void ShaderBuilder::setSamplers(Semantic const*const semantics, size_t count)
{
    for (size_t i = 0; i < count; ++i)
        samplers.insert(new Semantic(semantics[i]));
}

std::string ShaderBuilder::generateVertexShader(const char* body)
{
    std::stringstream s;
    s << "// Vertex\n";
    s << preamble();

    for (auto a : attributes)
        s << a.second->attributeString() << std::endl;
    for (auto u : uniforms)
        s << u->uniformString() << std::endl;

    if (varyings.size() > 0)
    {
        s << "out Var {\n";
        for (auto v : varyings)
            s << "   " << semanticTypeToString(v->type) << " " << v->name << ";\n";
        s << "} var;\n";
    }
    s << body << std::endl;

    return s.str();
}

std::string ShaderBuilder::generateFragmentShader(const char* body)
{
    std::stringstream s;
    s << "// Fragment\n";
    s << preamble();

    for (auto i : outputs)
        s << i->outputString() << std::endl;
    for (auto i : uniforms)
        s << i->uniformString() << std::endl;
    for (auto i : samplers)
        s << i->uniformString() << std::endl;

    if (varyings.size() > 0)
    {
        s << std::endl << "in Var {" << std::endl;
        for (auto v : varyings)
            s << "   " << semanticTypeToString(v->type) << " " << v->name << ";\n";
        s << "} var;" << std::endl;
    }
    s << body << std::endl;

    return s.str();
}

std::shared_ptr<Shader> ShaderBuilder::makeShader(
    const ShaderSpec& spec, const VAO& vao, bool printShader)
{
    std::vector<std::uint8_t> vrtx;
    if (spec.vtx_path.length())
        vrtx = loadFile(spec.vtx_path.c_str());
    else if (spec.vtx_src.length())
    {
        vrtx.resize(spec.vtx_src.length() + 1);
        strcpy(reinterpret_cast<char*>(&vrtx[0]), &spec.vtx_src[0]);
    }
    if (vrtx.empty())
        return {};

    std::vector<std::uint8_t> fgmt_body;
    if (spec.fgmt_path.length())
        fgmt_body = loadFile(spec.fgmt_path.c_str());
    else if (spec.fgmt_src.length())
    {
        fgmt_body.resize(spec.fgmt_src.length() + 1);
        strcpy(reinterpret_cast<char*>(&fgmt_body[0]), &spec.fgmt_src[0]);
    }
    if (fgmt_body.empty())
        return {};

    std::vector<std::uint8_t> fgmt_postAmble;
    if (spec.fgmt_post_path.length())
        fgmt_postAmble = loadFile(spec.fgmt_post_path.c_str());
    else if (spec.fgmt_post_src.length())
    {
        fgmt_postAmble.resize(spec.fgmt_post_src.length() + 1);
        strcpy(reinterpret_cast<char*>(&fgmt_postAmble[0]), &spec.fgmt_post_src[0]);
    }

    std::vector<std::uint8_t> fgmt;
    fgmt.resize(fgmt_body.size() + fgmt_postAmble.size());
    memcpy(&fgmt[0], &fgmt_body[0], fgmt_body.size() - 1);

    if (fgmt_postAmble.size())
        memcpy(&fgmt[fgmt_body.size() - 1], &fgmt_postAmble[0], fgmt_postAmble.size());

    auto shader = makeShader(spec.name, reinterpret_cast<const char*>(&vrtx[0]), reinterpret_cast<const char*>(&fgmt[0]), vao, printShader);

    for (auto u : spec.uniforms)
        if (u.automatic != AutomaticUniform::none)
            shader->automatics.push_back(u);

    return shader;
}


std::shared_ptr<Shader> ShaderBuilder::makeShader(
    const std::string& name,
    const char* vtxCode, const char* fgmtCode,
    const VAO& vao,
    bool printShader)
{
    std::string vtx = generateVertexShader(vtxCode);
    std::string fgm = generateFragmentShader(fgmtCode);

    if (printShader) {
        printf(  "Vertex Shader  \n__________________________\n%s\n\n\n\n", vtx.c_str());
        printf("\nFragment Shader\n__________________________\n%s\n\n\n\n", fgm.c_str());
    }

    vao.bindVAO();
    std::shared_ptr<Shader>shader = std::make_shared<Shader>();
    shader->shader(name, Shader::ProgramType::Vertex,   false, vtx.c_str()).
            shader(name, Shader::ProgramType::Fragment, false, fgm.c_str()).link();
    vao.unbindVAO();

    return shader;
}


}} // lab::Render
