//
//  main.cpp
//  LabRenderExamples
//
//  Created by Nick Porcino on 5/10/15.
//  Copyright (c) 2015 Planet IX. All rights reserved.
//

#include "../LabRenderDemoApp.h"
#include "ImGuiLab.h"
#include <LabRenderModelLoader/modelLoader.h>

#include <LabCamera/LabCamera.h>
#include <LabMath/LabMath.h>
#include <LabRender/PassRenderer.h>
#include <LabRender/UtilityModel.h>
#include <LabRender/Utils.h>

#include <LabCmd/FFI.h>

using namespace std;
using lab::v2i;
using lab::v2f;
using lab::v3f;
using lab::v4f;


class PingCommand : public lab::Command
{
public:
    PingCommand()
    {
        run =
            [](const lab::Command& cmd, const string& path, const std::vector<Argument>&, lab::Ack& ack) {
            ack.payload.push_back(Argument("pong"));
            cout << "Sending pong" << endl;
        };
    }
    virtual ~PingCommand() {}

    virtual std::string name() const override { return "ping"; }
};

class LoadMeshCommand : public lab::Command
{
public:
    LoadMeshCommand(lab::Render::Renderer* renderer, lab::Render::DrawList* drawlist)
        : _renderer(renderer), _drawlist(drawlist)
    {
        run =
            [this](const lab::Command& cmd, const string& path, const std::vector<Argument>& args, lab::Ack& ack)
        {
            string filepath = args[0].stringArg;
            _renderer->enqueCommand([this, filepath]()
                {
                    shared_ptr<lab::Render::Model> model = lab::Render::loadMesh(filepath);
                    if (model)
                    {
                        _drawlist->deferredMeshes.clear();
                        for (auto& i : model->parts())
                            _drawlist->deferredMeshes.emplace_back(std::pair<lab::m44f, std::shared_ptr<lab::Render::ModelBase>>(lab::m44f_identity, i));
                    }
                });
        };

        parameterSpecification.push_back(make_pair<string, SemanticType>("path", SemanticType::string_st));
    }

    virtual ~LoadMeshCommand() {}
    virtual std::string name() const override { return "loadMesh"; }

    lab::Render::Renderer* _renderer;
    lab::Render::DrawList* _drawlist;
};

class ImGUISceneBuilder : public lab::LabRenderAppScene {
public:
    lab::OSCServer oscServer;
    lab::WebSocketsServer wsServer;

    ImGUISceneBuilder()
    : oscServer("labrender")
    , wsServer("labrender")
    {}

    virtual ~ImGUISceneBuilder() = default;

    virtual void build() override {
        // load a pipeline

        std::string path = "{ASSET_ROOT}/pipelines/deferred-fxaa.labfx";
        std::cout << "Loading pipeline configuration " << path << std::endl;
        dr = make_shared<lab::Render::PassRenderer>();
        dr->configure(path.c_str());

        auto& meshes = drawList.deferredMeshes;

        //shared_ptr<lab::ModelBase> model = lab::Model::loadMesh("{ASSET_ROOT}/models/starfire.25.obj");
        shared_ptr<lab::Render::Model> model = lab::Render::loadMesh("{ASSET_ROOT}/models/ShaderBall/shaderBallNoCrease/shaderBall.obj");
        if (model)
            for (auto& i : model->parts())
                meshes.push_back({ lab::m44f_identity, i });

        shared_ptr<lab::Render::UtilityModel> cube = make_shared<lab::Render::UtilityModel>();
        cube->createCylinder(0.5f, 0.5f, 2.f, 16, 3, false);
        meshes.push_back({ lab::m44f_identity, cube });

        if (model) {
            static float foo = 0.f;
            camera.mount.transform.position = { foo, 0, -1000 };
            lab::Bounds bounds = model->localBounds();
            //bounds = model->transform.transformBounds(bounds);
            v3f& mn = bounds.first;
            v3f& mx = bounds.second;
            lc_camera_frame(&camera, lc_v3f{ mn.x, mn.y, mn.z }, lc_v3f{ mx.x, mx.y, mx.z });
        }

        shared_ptr<lab::Command> command = make_shared<PingCommand>();
        oscServer.registerCommand(command);
        command = make_shared<LoadMeshCommand>(dr.get(), &drawList);
        oscServer.registerCommand(command);

        const int PORT_NUM = 9109;
        oscServer.start(PORT_NUM);

        wsServer.start(PORT_NUM + 1);

    }
};

class ImGuiApp : public lab::LabRenderExampleApp {
public:
    lab::ImGuiIntegration* imgui = nullptr;

    ImGuiApp()
    : lab::LabRenderExampleApp("Dear ImGui")
    {
        scene = new ImGUISceneBuilder();
        imgui = new lab::ImGuiIntegration(window);
        _supplemental = imgui;
        
        const char * env = getenv("ASSET_ROOT");
		if (env)
			lab::addPathVariable("{ASSET_ROOT}", env);
		else
			lab::addPathVariable("{ASSET_ROOT}", ASSET_ROOT);
    }

    ~ImGuiApp()
    {
        delete scene;
        delete _supplemental;
    }

    virtual bool isFinished() override
    {
        return glfwWindowShouldClose(window) != 0 || imgui->quit;
    }
};


int main(int argc, char* argv[])
{
    shared_ptr<ImGuiApp> appPtr = make_shared<ImGuiApp>();

	lab::checkError(lab::ErrorPolicy::onErrorThrow,
		lab::TestConditions::exhaustive, "main loop start");

    ImGuiApp* app = appPtr.get();

    app->createScene();

    while (!app->isFinished())
    {
        app->update();
        app->frameBegin();
        app->render();
        app->frameEnd();
    }

    return 1;
}
