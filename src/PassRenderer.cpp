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
#include "LabRender/gl4.h"
#include "json/json.h"

#include <fstream>

using namespace lab;
using namespace std;

GLint depthTestToGL[] = {
    GL_LESS, GL_LEQUAL, GL_NEVER, GL_EQUAL, GL_GREATER, GL_NOTEQUAL, GL_GEQUAL, GL_ALWAYS };

PassRenderer::Pass::Pass(const std::string& name, int passNumber)
: _name(name), _passNumber(passNumber)
, writeDepth(true), depthTest(DepthTest::less)
, clearDepthBuffer(false), clearGbuffer(false)
, isQuadPass(false), drawOpaqueGeometry(false)
{
}

void PassRenderer::Pass::bindInputTextures(RenderLock & rl, const FramebufferSet & fbos)
{
    for (const pair<string, vector<string>>& readBuffer : readAttachments) {
        std::shared_ptr<FrameBuffer> gbufferAOVs = fbos.fbo(readBuffer.first);
        int bindBase = rl.context.activeTextureUnit;
        for (const string& a : readBuffer.second) {
            for (int i = 0; i < gbufferAOVs->drawBuffers.size(); ++i) {
                if (gbufferAOVs->baseNames[i] == a) {
                    gbufferAOVs->textures[i]->bind(bindBase);
                    if (_shader)
                        _shader->uniformInt(gbufferAOVs->uniformNames[i].c_str(), bindBase);
                    ++bindBase;
                }
            }
        }
        rl.context.activeTextureUnit = bindBase;
    }
}

void PassRenderer::Pass::prepareFullScreenQuadAndShader(const FramebufferSet & fbos)
{
    if (!isQuadPass)
        return;

    if (!_fullScreenQuadMesh) {
        UtilityModel* quad = new UtilityModel();
        quad->createFullScreenQuad();
        _fullScreenQuadMesh.reset(quad);
    }

    if (!_shader) {
        std::shared_ptr<FrameBuffer> gbufferAOVs = fbos.fbo(writeBuffer);

        ShaderBuilder sb;
        if (gbufferAOVs)
            sb.setGbuffer(*gbufferAOVs);

        _fullScreenQuadMesh->verts()->uploadVerts();
        sb.setAttributes(* _fullScreenQuadMesh.get());
        sb.setVaryings(shaderSpec);
        sb.setUniforms(shaderSpec);

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

	checkError(ErrorPolicy::onErrorThrow,
		TestConditions::exhaustive, "Pass::run bind input textures");

	if (isQuadPass) 
	{
        glViewport(0, 0, rl.context.framebufferSize.x, rl.context.framebufferSize.y);
        _shader->bind(rl);
		bindInputTextures(rl, fbos);	// binds the textures and the shader uniforms
		_fullScreenQuadMesh->verts()->draw();
    }
    if (drawOpaqueGeometry) 
	{
        std::shared_ptr<FrameBuffer> gbufferAOVs = fbos.fbo(writeBuffer);

        for (auto model : rl.context.drawList->deferredMeshes) 
		{
            rl.context.viewMatrices.model = model->transform.transform();
            rl.context.viewMatrices.mv = matrix_multiply(rl.context.drawList->view, rl.context.viewMatrices.model);
            rl.context.viewMatrices.mvp = matrix_multiply(rl.context.drawList->proj, rl.context.viewMatrices.mv);
            rl.context.viewMatrices.view = rl.context.drawList->view;
            rl.context.viewMatrices.projection = rl.context.drawList->proj;
            model->draw(*gbufferAOVs.get(), rl);
        }
    }
}

class PassRenderer::Detail {
public:
    Detail() {}

    FramebufferSet fbos;
    TextureSet textures;

    vector<shared_ptr<Pass>> passes;
};

PassRenderer::PassRenderer() : _detail(new Detail()) {
}

PassRenderer::~PassRenderer() {
    delete _detail;
}

std::shared_ptr<Texture> PassRenderer::texture(const std::string & name)
{
    return _detail->textures.texture(name);
}

std::shared_ptr<FrameBuffer> PassRenderer::framebuffer(const std::string & name)
{
    return _detail->fbos.fbo(name);
}


void PassRenderer::configure(const char *const path)
{
    string p = expandPath(path);
    std::ifstream in(p);
    Json::Value conf;
    in >> conf;

    printf("\nTextures:\n");
    for (Json::Value::iterator it = conf["textures"].begin(); it != conf["textures"].end(); ++it)
	{
        // { "id": "tex16", "path": "$(ASSET_ROOT)/textures/shadertoy/tex16.png" }
        string id = (*it)["id"].asString();
        string path = (*it)["path"].asString();
        FileTextureProvider provider(path);
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

            string outputName = "o_" + name + "Texture";
            string uniformName = "u_" + name + "Texture";

			TextureType textureType = TextureType::u8x4;	// default to RGBA8
            if (typeStr == "u8x4") textureType = TextureType::u8x4;
            else if (typeStr == "f16x4") textureType = TextureType::f16x4;
            else if (typeStr == "f32x4") textureType = TextureType::f32x4;

            spec.attachments.push_back(FrameBuffer::FrameBufferSpec::AttachmentSpec(name, outputName, uniformName, textureType));
        }

        _detail->fbos.add_fbo(bufferName, spec);
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
            if (uniforms.type() != Json::nullValue) {
                for (Json::Value::iterator uniform = uniforms.begin(); uniform != uniforms.end(); ++uniform) {
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
                    if (v.type() != Json::nullValue) {
                        texture = v.asString();
                    }

                    pass->shaderSpec.uniforms.push_back(Uniform(name, type, automatic, texture));
                }
            }
            Json::Value varyings = shader["varyings"];
            if (varyings.type() != Json::nullValue ) {
                for (Json::Value::iterator varying = varyings.begin(); varying != varyings.end(); ++varying) {
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
                                      attachment != writeBuffer["render_textures"].end(); ++attachment) {
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
                for (Json::Value::iterator attachment = attachments.begin(); attachment != attachments.end(); ++attachment) {
                    string a = (*attachment).asString();
                    buffers.push_back(a);
                }
                pass->readAttachments.push_back(make_pair(name, buffers));
            }
        }
    }
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

        if (true || (bound_frame_buffer != pass->writeBuffer))
        {
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // unbind previous framebuffer

			if (pass->writeBuffer == "" || pass->writeBuffer == "visible")
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, rl.context.rootFramebuffer);
            else
                _detail->fbos.fbo(pass->writeBuffer)->bindForWrite(pass->writeAttachments);

            // because I need to take into account active writebuffers, I've
            // commented out to force rebinding: 
			bound_frame_buffer = pass->writeBuffer;
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
