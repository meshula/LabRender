//
//  PassRenderer.cpp
//  LabApp
//
//  Created by Nick Porcino on 2014 01/19.
//  Copyright (c) 2014 Nick Porcino. All rights reserved.
//

#include "LabRender/PassRenderer.h"

#include "LabRender/Camera.h"
#include "LabRender/FrameBuffer.h"
#include "LabRender/Model.h"
#include "LabRender/SemanticType.h"
#include "LabRender/ShaderBuilder.h"
#include "LabRender/Texture.h"
#include "LabRender/Utils.h"
#include "LabRender/UtilityModel.h"
#include "LabRenderGraph/LabRenderGraph.h"
#include "gl4.h"
#include "json/json.h"

#include <fstream>

using namespace std;
using namespace lab::Render;

GLint depthTestToGL[] = {
    GL_LESS, GL_LEQUAL, GL_NEVER, GL_EQUAL, GL_GREATER, GL_NOTEQUAL, GL_GEQUAL, GL_ALWAYS };

namespace lab {

DepthTest stringToDepthTest(const std::string & df)
{
    if (df == "less")          return DepthTest::less;
    else if (df == "lequal")   return DepthTest::lequal;
    else if (df == "never")    return DepthTest::never;
    else if (df == "equal")    return DepthTest::equal;
    else if (df == "greater")  return DepthTest::greater;
    else if (df == "notequal") return DepthTest::notequal;
    else if (df == "gequal")   return DepthTest::equal;
    else if (df == "always")   return DepthTest::always;
    return DepthTest::less;
}

PassRenderer::Pass::Pass(const std::string& name, int passNumber)
: _name(name), _passNumber(passNumber)
, writeDepth(true), depthTest(DepthTest::less)
, clearDepthBuffer(false), clearGbuffer(false)
, isQuadPass(false), drawOpaqueGeometry(false)
{
}

void PassRenderer::Pass::bindInputTextures(RenderLock& rl, const FramebufferSet& fbos)
{
    for (const pair<string, string>& readBuffer : readAttachments)
    {
        std::shared_ptr<FrameBuffer> textureInputs = fbos.fbo(readBuffer.first);
        int texture_unit = rl.context.activeTextureUnit;
        for (int i = 0; i < textureInputs->baseNames.size(); ++i)
        {
            if (textureInputs->baseNames[i] == readBuffer.second)
            {
                textureInputs->textures[i]->bind(texture_unit);
                if (_shader)
                    _shader->uniformInt(textureInputs->uniformNames[i].c_str(), texture_unit);
                ++texture_unit;
            }
        }
        rl.context.activeTextureUnit = texture_unit;
    }
}

void PassRenderer::Pass::prepareFullScreenQuadAndShader(const FramebufferSet & fbos)
{
    if (!isQuadPass)
        return;

    if (!_fullScreenQuadMesh)
    {
        UtilityModel* quad = new UtilityModel();
        quad->createFullScreenQuad();
        _fullScreenQuadMesh.reset(quad);
    }

    if (!_shader)
    {
        std::shared_ptr<FrameBuffer> gbufferAOVs = fbos.fbo(writeBuffer);

        ShaderBuilder sb;
        if (gbufferAOVs)
            sb.setFrameBufferOutputs(*gbufferAOVs, writeAttachments);

        _fullScreenQuadMesh->verts()->uploadVerts();
        sb.setAttributes(* _fullScreenQuadMesh.get());
        sb.setVaryings(shaderSpec);
        sb.setUniforms(shaderSpec);
        sb.setSamplers(shaderSpec);

        shaderSpec.name = name();
        const bool printShader = false;

        _shader = sb.makeShader(shaderSpec, * _fullScreenQuadMesh->verts(), printShader);
        _fullScreenQuadMesh->setShader(_shader);
    }
}


void PassRenderer::Pass::run(RenderLock& rl, const FramebufferSet& fbos)
{
	checkError(ErrorPolicy::onErrorThrow,
               TestConditions::exhaustive, "Pass::run");

	if (isQuadPass)
	{
        glViewport(0, 0, rl.context.framebufferSize.x, rl.context.framebufferSize.y);
        _shader->bind(rl);
		bindInputTextures(rl, fbos);	// binds the textures and the shader uniforms
		_fullScreenQuadMesh->verts()->draw();
        checkError(ErrorPolicy::onErrorThrow,
                   TestConditions::exhaustive, "Pass::bind full screen pass");
    }

    if (drawOpaqueGeometry)
	{
        std::shared_ptr<FrameBuffer> gbufferAOVs = fbos.fbo(writeBuffer);

        for (auto& model : rl.context.drawList->deferredMeshes)
		{
            rl.context.viewMatrices.model = model.first;
            rl.context.viewMatrices.mv = matrix_multiply(rl.context.drawList->view, rl.context.viewMatrices.model);
            rl.context.viewMatrices.mvp = matrix_multiply(rl.context.drawList->proj, rl.context.viewMatrices.mv);
            rl.context.viewMatrices.view = rl.context.drawList->view;
            rl.context.viewMatrices.projection = rl.context.drawList->proj;
            model.second->draw(*gbufferAOVs.get(), writeAttachments, rl);
        }
    }
}

class PassRenderer::Detail {
public:
    Detail() {}

    FramebufferSet fbos;
    Render::TextureSet textures;

    vector<shared_ptr<Pass>> passes;
};

PassRenderer::PassRenderer() : _detail(new Detail()) {
}

PassRenderer::~PassRenderer() {
    delete _detail;
}

std::shared_ptr<Render::Texture> PassRenderer::texture(const std::string & name)
{
    return _detail->textures.texture(name);
}

std::shared_ptr<FrameBuffer> PassRenderer::framebuffer(const std::string & name)
{
    return _detail->fbos.fbo(name);
}


void PassRenderer::configure(char const*const path)
{
    string p = expandPath(path);
#if 1
    struct stat st;
    if (stat(p.c_str(), &st) != 0)
    {
        return;
    }
    size_t sz = st.st_size;

    FILE* f = fopen(p.c_str(), "rb");
    std::vector<char> buff(sz);
    fread(&buff[0], 1, sz, f);
    fclose(f);

    labfx_t* fx_ptr = parse_labfx(&buff[0], sz);
    labfx_gen_t* sh_ptr = generate_shaders(fx_ptr);

    auto fx = reinterpret_cast<lab::fx::labfx*>(fx_ptr);
    auto sh = reinterpret_cast<lab::fx::shader*>(sh_ptr);

    for (const auto& tx : fx->textures)
	{
        Render::FileTextureProvider provider(tx.path);
        _detail->textures.add_texture(tx.name, provider.texture());
    }

    for (const auto& bf : fx->buffers)
	{
        FrameBuffer::FrameBufferSpec spec;
        spec.hasDepth = bf.has_depth;
        for (const auto& tx: bf.textures)
        {
            string name = tx.name;
            string outputName = "o_" + name + "_texture";
            string uniformName = "u_" + name + "_texture";
            spec.attachments.push_back(FrameBuffer::FrameBufferSpec::AttachmentSpec(name, outputName, uniformName, tx.format));
        }

        _detail->fbos.addFbo(bf.name, spec);
    }

    int passNumber = 0;
    for (const auto& ps : fx->passes)
	{
        std::shared_ptr<Pass> pass = addPass(std::make_shared<Pass>(ps.name, passNumber++));

        lab::fx::shader const* shader = nullptr;
        for (const auto& s : fx->shaders)
            if (s.name == ps.shader)
            {
                shader = &s;
                break;
            }

        if (shader)
        {
            pass->shaderSpec.vtx_src = shader->vsh_source;
            pass->shaderSpec.fgmt_src = shader->fsh_source;
            //pass->shaderSpec.fgmt_post_src = fragment_shader_postamble_path.asString();

            for (const auto& uniform : shader->uniforms)
            {
                string texture;
                AutomaticUniform automatic = AutomaticUniform::none;
                if (uniform.automatic.length() > 0)
				{
                    if ((uniform.automatic == "resolution") || (uniform.automatic == "auto-resolution"))
                        automatic = AutomaticUniform::frameBufferResolution;
                    else if ((uniform.automatic == "sky_matrix") || (uniform.automatic == "auto-sky-matrix"))
                        automatic = AutomaticUniform::skyMatrix;
                    else if (uniform.automatic == "render_time")
                        automatic = AutomaticUniform::renderTime;
                    else if (uniform.automatic == "mouse_position")
                        automatic = AutomaticUniform::mousePosition;
                    else
                        texture = uniform.automatic;
                }

                if (semanticTypeIsSampler(uniform.type))
                    pass->shaderSpec.samplers.push_back(Uniform(uniform.name, uniform.type, automatic, texture));
                else
                    pass->shaderSpec.uniforms.push_back(Uniform(uniform.name, uniform.type, automatic, texture));
            }

            for (const auto& varying : shader->varyings)
            {
                pass->shaderSpec.varyings.push_back(make_pair(varying.name, varying.type));
            }
        }

        pass->isQuadPass = ps.draw == lab::fx::pass_draw::quad;
        pass->drawOpaqueGeometry = ps.draw == lab::fx::pass_draw::opaque_geometry;
        pass->writeDepth = ps.write_depth;
        pass->depthTest = ps.test;
        pass->clearDepthBuffer = ps.clear_depth;
        pass->clearGbuffer = ps.clear_outputs;
        pass->writeBuffer = ps.output_buffer;

        for (const auto& tx : ps.output_textures)
            pass->writeAttachments.push_back(tx);

        for (const auto& tx: ps.input_textures)
        {
            pass->readAttachments.push_back({tx.name, tx.texture});
            std::string sampler_name = "o_" + tx.texture;
            pass->shaderSpec.samplers.push_back(Uniform(sampler_name,
                                                SemanticType::sampler2D_st,
                                                AutomaticUniform::none,
                                                tx.name));
        }
    }

    free_labfx_gen(sh_ptr);
    free_labfx(fx_ptr);
    return;
#else

    std::ifstream in(p);
    Json::Value conf;
    try
    {
        in >> conf;
    }
    catch(std::exception& exc)
    {
        printf("%s\n", exc.what());
        throw;
    }

    printf("\nTextures:\n");
    for (Json::Value::iterator it = conf["textures"].begin(); it != conf["textures"].end(); ++it)
	{
        // { "id": "tex16", "path": "{ASSET_ROOT}/textures/shadertoy/tex16.png" }
        string id = (*it)["id"].asString();
        printf(" %s\n", id.c_str());
        string path = (*it)["path"].asString();
        Render::FileTextureProvider provider(path);
        _detail->textures.add_texture(id, provider.texture());
    }

    printf("\nBuffers:\n");
    for (Json::Value::iterator it = conf["buffers"].begin(); it != conf["buffers"].end(); ++it)
	{
        string bufferName = (*it)["name"].asString();
        printf(" %s\n", bufferName.c_str());

        FrameBuffer::FrameBufferSpec spec;
        spec.hasDepth = (*it)["depth"].asString() == "yes";

        for (Json::Value::iterator it2 = (*it)["render_textures"].begin(); it2 != (*it)["render_textures"].end(); ++it2) {

            string name = (*it2)["name"].asString();
            string typeStr = (*it2)["type"].asString();

            printf("  %s %s\n", name.c_str(), typeStr.c_str());

            string outputName = "o_" + name + "_texture";
            string uniformName = "u_" + name + _tTexture";

			Render::TextureType textureType = TextureType::u8x4;	// default to RGBA8
            if (typeStr == "u8x4") textureType = TextureType::u8x4;
            else if (typeStr == "f16x4") textureType = TextureType::f16x4;
            else if (typeStr == "f32x4") textureType = TextureType::f32x4;

            spec.attachments.push_back(FrameBuffer::FrameBufferSpec::AttachmentSpec(name, outputName, uniformName, textureType));
        }

        _detail->fbos.addFbo(bufferName, spec);
    }

    printf("\nPasses:\n");
    int passNumber = 0;
    for (Json::Value::iterator it = conf["passes"].begin(); it != conf["passes"].end(); ++it)
	{
        Json::Value passVal = (*it)["type"];
        string passType = passVal["run"].asString();
        string passName = (*it)["name"].asCString();
        printf(" %s %s\n", passName.c_str(), passType.c_str());

        std::shared_ptr<Pass> pass = addPass(std::make_shared<Pass>(passName, passNumber++));
        Json::Value shader = (*it)["shader"];
        if (shader.type() != Json::nullValue) {
            Json::Value vertex_shader_path = shader["vertex_shader_path"];
            if (vertex_shader_path.type() != Json::nullValue)
                pass->shaderSpec.vertexShaderPath = vertex_shader_path.asString();

            Json::Value fragment_shader_path = shader["fragment_shader_path"];
            if (fragment_shader_path.type() != Json::nullValue)
                pass->shaderSpec.fragmentShaderPath = fragment_shader_path.asString();

            Json::Value fragment_shader_postamble_path = shader["fragment_shader_postamble_path"];
            if (fragment_shader_postamble_path.type() != Json::nullValue)
                pass->shaderSpec.fragmentShaderPostamblePath = fragment_shader_postamble_path.asString();

            Json::Value uniforms = shader["uniforms"];
            if (uniforms.type() != Json::nullValue)
            {
                for (Json::Value::iterator uniform = uniforms.begin(); uniform != uniforms.end(); ++uniform)
                {
                    string name = (*uniform)["name"].asString();
                    string typeStr = (*uniform)["type"].asString();
                    string texture;
                    SemanticType type = semanticTypeNamed(typeStr.c_str());

                    AutomaticUniform automatic = AutomaticUniform::none;
                    Json::Value automaticValue = (*uniform)["auto"];
                    if (automaticValue.type() != Json::nullValue)
					{
                        auto s = automaticValue.asCString();
                        if (!strcmp(s, "resolution"))
                            automatic = AutomaticUniform::frameBufferResolution;
                        else if (!strcmp(s, "sky_matrix"))
                            automatic = AutomaticUniform::skyMatrix;
                        else if (!strcmp(s, "render_time"))
                            automatic = AutomaticUniform::renderTime;
                        else if (!strcmp(s, "mouse_position"))
                            automatic = AutomaticUniform::mousePosition;
                    }

                    Json::Value v = (*uniform)["texture"];
                    if (v.type() != Json::nullValue)
                        texture = v.asString();

                    if (semanticTypeIsSampler(type))
                        pass->shaderSpec.samplers.push_back(Uniform(name, type, automatic, texture));
                    else
                        pass->shaderSpec.uniforms.push_back(Uniform(name, type, automatic, texture));
                }
            }
            Json::Value varyings = shader["varyings"];
            if (varyings.type() != Json::nullValue )
            {
                for (Json::Value::iterator varying = varyings.begin(); varying != varyings.end(); ++varying)
                {
                    string name = (*varying)["name"].asString();
                    string typeStr = (*varying)["type"].asString();
                    SemanticType type = semanticTypeNamed(typeStr.c_str());
                    pass->shaderSpec.varyings.push_back(make_pair(name, type));
                }
            }
        }

        string drawType = passVal["draw"].asString();
        pass->isQuadPass = drawType == "quad";
        pass->drawOpaqueGeometry = drawType == "opaque-geometry";

        bool writeDepth = pass->writeDepth;
        DepthTest dfunc = pass->depthTest;

        bool clearDepthBuffer = pass->clearDepthBuffer;
        bool clearGbuffer = pass->clearGbuffer;

        Json::Value depth = (*it)["depth"];
        if (depth.type() != Json::nullValue)
		{
            Json::Value val = depth["write"];
            if (val.type() != Json::nullValue)
                writeDepth = !strcmp(val.asCString(), "yes");
            val = depth["test"];
            if (val.type() != Json::nullValue) {
                string df = val.asString();
				dfunc = stringToDepthTest(df);
            }
            val = depth["clear_buffer"];
            if (val.type() != Json::nullValue)
                clearDepthBuffer = val.asString() == "yes";
        }
        pass->writeDepth = writeDepth;
        pass->depthTest = dfunc;
        pass->clearDepthBuffer = clearDepthBuffer;

        Json::Value writeBuffer = (*it)["outputs"];
        if (writeBuffer.type() != Json::nullValue)
		{
            string writeBufferName = writeBuffer["buffer"].asString();
            pass->writeBuffer = writeBufferName;
            for (Json::Value::iterator attachment = writeBuffer["render_textures"].begin();
                                      attachment != writeBuffer["render_textures"].end(); ++attachment)
            {
                pass->writeAttachments.push_back((*attachment).asString());
            }

            Json::Value val = writeBuffer["clear_buffer"];
            if (val.type() != Json::nullValue)
                clearGbuffer = val.asString() == "yes";
        }
        pass->clearGbuffer = clearGbuffer;

        Json::Value readBuffers = (*it)["inputs"];
        if (readBuffers.type() != Json::nullValue)
		{
            for (Json::Value::iterator buffer = readBuffers.begin(); buffer != readBuffers.end(); ++buffer)
			{
                string name = (*buffer)["buffer"].asString();
                Json::Value attachments = (*buffer)["render_textures"];
                vector<string> buffers;
                for (Json::Value::iterator attachment = attachments.begin(); attachment != attachments.end(); ++attachment)
                {
                    string a = (*attachment).asString();
                    buffers.push_back(a);
                    pass->shaderSpec.samplers.push_back(Uniform(name, SemanticType::sampler2D_st, AutomaticUniform::none, a));
                }
                pass->readAttachments.push_back(make_pair(name, buffers));
            }
        }
    }
#endif
}

std::shared_ptr<PassRenderer::Pass> PassRenderer::addPass(std::shared_ptr<Pass> pass)
{
    _detail->passes.push_back(pass);
    sort(_detail->passes.begin(), _detail->passes.end(),
         [](const shared_ptr<Pass>& a, const shared_ptr<Pass>& b)
         {
             return a->passNumber() < b->passNumber();
         });

    return pass; // for chaining
}

PassRenderer::Pass* PassRenderer::_findPass(const std::string& name) const
{
    for (auto i : _detail->passes)
        if (name == i->name())
            return i.get();

    return nullptr;
}

void PassRenderer::render(RenderLock& rl, v2i fbSize, DrawList& drawList)
{
    if (!rl.valid())
        return;

	GLenum framebuffer_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (framebuffer_status != GL_FRAMEBUFFER_COMPLETE)
		return;

    checkError(ErrorPolicy::onErrorThrow,
               TestConditions::exhaustive, "runPasses");

    CaptureFrameBuffer current_frame_buffer;

    _detail->fbos.setSize(fbSize.x, fbSize.y);

    rl.context.drawList = &drawList;
    rl.context.framebufferSize = fbSize;
    rl.context.rootFramebuffer = current_frame_buffer.currFramebuffer;

    string bound_frame_buffer = "*";
    vector<string> bound_attachments;

    glClearColor(0, 0, 0, 0);
    glClearDepthf(1.0f);

	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_STENCIL_TEST);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    for (auto pass : _detail->passes)
	{
        checkError(ErrorPolicy::onErrorThrow, TestConditions::exhaustive, "render, before pass");

        //&&&if (bound_frame_buffer != pass->writeBuffer || bound_attachments != pass->writeAttachments)
        {
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // unbind previous framebuffer

			if (pass->writeBuffer == "" || pass->writeBuffer == "visible")
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, rl.context.rootFramebuffer);
            else
                _detail->fbos.fbo(pass->writeBuffer)->bindForWrite(pass->writeAttachments);

			bound_frame_buffer = pass->writeBuffer;
            bound_attachments = pass->writeAttachments;
        }
		checkError(ErrorPolicy::onErrorThrow, TestConditions::exhaustive, "render, bind for write");

		if (pass->isQuadPass)
	        pass->prepareFullScreenQuadAndShader(_detail->fbos);

        if (pass->depthTest == DepthTest::never)
            glDisable(GL_DEPTH_TEST);
        else
        {
            glEnable(GL_DEPTH_TEST);
            int itype = static_cast< typename std::underlying_type<DepthTest>::type >(pass->depthTest);
            glDepthFunc(depthTestToGL[itype]);
        }

        uint32_t clearbits = pass->clearDepthBuffer? GL_DEPTH_BUFFER_BIT : 0;
        clearbits |= pass->clearGbuffer? GL_COLOR_BUFFER_BIT : 0;
        if (clearbits)
		{
            glDepthMask(GL_TRUE);
            glClear(clearbits);
        }

        glDepthMask(pass->writeDepth? GL_TRUE:GL_FALSE);
        glDisable(GL_BLEND);

        pass->run(rl, _detail->fbos);

        checkError(ErrorPolicy::onErrorThrow, TestConditions::exhaustive, "render, after pass");
    }

    glUseProgram(0);
}

} // lab
