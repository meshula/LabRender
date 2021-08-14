//
//  main.cpp
//  LabRenderExamples
//
//  Created by Nick Porcino on 5/10/15.
//  Copyright (c) 2015 Planet IX. All rights reserved.
//

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
            [](const lab::Command & cmd, const string & path, const std::vector<Argument> &, lab::Ack & ack) {
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
    LoadMeshCommand(lab::Render::Renderer * renderer, lab::Render::DrawList * drawlist)
    : _renderer(renderer), _drawlist(drawlist)
    {
        run =
            [this](const lab::Command & cmd, const string & path, const std::vector<Argument> & args, lab::Ack & ack)
            {
                string filepath = args[0].stringArg;
                _renderer->enqueCommand([this, filepath]()
                {
                    shared_ptr<lab::Render::Model> model = lab::Render::loadMesh(filepath);
                    if (model)
                    {
                        _drawlist->deferredMeshes.clear();
                        for (auto& i : model->parts())
                            _drawlist->deferredMeshes.emplace_back(std::pair<lab::m44f, std::shared_ptr<lab::Render::ModelBase>>( lab::m44f_identity, i ));
                    }
                });
            };

        parameterSpecification.push_back(make_pair<string, SemanticType>("path", SemanticType::string_st));
    }

    virtual ~LoadMeshCommand() {}
    virtual std::string name() const override { return "loadMesh"; }

    lab::Render::Renderer * _renderer;
    lab::Render::DrawList * _drawlist;
};




class LabRenderExampleApp : public lab::GLFWAppBase {
public:
    shared_ptr<lab::Render::PassRenderer> dr;
    lab::Render::DrawList drawList;

    lab::OSCServer oscServer;
    lab::WebSocketsServer wsServer;

    lc_camera camera;
    lc_interaction* camera_controller = nullptr;
    lc_i_Mode cameraRigMode = lc_i_ModeTurnTableOrbit;
    lc_i_Phase interactionPhase = lc_i_PhaseNone;

    v2f initialMousePosition;
    v2f previousMousePosition;
    v2f previousWindowSize;

    LabRenderExampleApp()
    : GLFWAppBase("ImGui LabRender")
    , oscServer("labrender")
    , wsServer("labrender")
    , previousMousePosition(V2F(0,0))
    {
        lc_camera_set_defaults(&camera);
        camera_controller = lc_i_create_interactive_controller();

        _supplemental = new lab::ImGuiIntegration(window);
        
        const char * env = getenv("ASSET_ROOT");
		if (env)
			lab::addPathVariable("{ASSET_ROOT}", env);
		else
			lab::addPathVariable("{ASSET_ROOT}", ASSET_ROOT);

		std::string path = "{ASSET_ROOT}/pipelines/deferred-fxaa.labfx";
		std::cout << "Loading pipeline configuration " << path << std::endl;
        dr = make_shared<lab::Render::PassRenderer>();
        dr->configure(path.c_str());
    }

    ~LabRenderExampleApp()
    {
        lc_i_free_interactive_controller(camera_controller);
        delete _supplemental;
    }


    void createScene()
	{
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

    void render()
	{
        // run the immediate gui. @TODO: add a ui() method

        if (interactionPhase != lc_i_PhaseNone) {
            InteractionToken it = lc_i_begin_interaction(camera_controller, { previousWindowSize.x, previousWindowSize.y });
            lc_i_ttl_interaction(
                camera_controller,
                &camera,
                it,
                interactionPhase,
                cameraRigMode,
                lc_v2f{ previousMousePosition.x, previousMousePosition.y },
                lc_radians{ 0 },
                1.f / 60.f);
            lc_i_end_interaction(camera_controller, it);

            if (interactionPhase == lc_i_PhaseStart)
                interactionPhase = lc_i_PhaseContinue;
            if (interactionPhase == lc_i_PhaseFinish)
                interactionPhase = lc_i_PhaseNone;
        }

        lab::checkError(lab::ErrorPolicy::onErrorThrow,
                              lab::TestConditions::exhaustive, "main loop start");

        v2i fbSize = frameBufferDimensions();

        lc_m44f modl = lc_mount_rotation_transform(&camera.mount);
        drawList.modl = *reinterpret_cast<lab::m44f*>(&modl);
        lc_m44f view = lc_mount_gl_view_transform(&camera.mount);
        drawList.view = *reinterpret_cast<lab::m44f*>(&view);
        lc_m44f proj = lc_camera_perspective(&camera, float(fbSize.x) / float(fbSize.y));
        drawList.proj = *reinterpret_cast<lab::m44f*>(&proj);

        lab::Render::PassRenderer::RenderLock rl(dr.get(), renderTime(), mousePosition());
		v2i fbOffset = V2I(0, 0);
        renderStart(rl, renderTime(), fbOffset, fbSize);

        dr->render(rl, fbSize, drawList);

        renderEnd(rl);

        lab::checkError(lab::ErrorPolicy::onErrorThrow,
                              lab::TestConditions::exhaustive, "main loop end");
    }

    virtual void keyPress(int key) override {
        switch (key) {
        case GLFW_KEY_C: cameraRigMode = lc_i_ModeCrane; break;
        case GLFW_KEY_D: cameraRigMode = lc_i_ModeDolly; break;
        case GLFW_KEY_T:
        case GLFW_KEY_O: cameraRigMode = lc_i_ModeTurnTableOrbit; break;
        }
    }

    virtual void mouseDown(v2f windowSize, v2f pos) override {
        previousMousePosition = pos;
        initialMousePosition = pos;
        previousWindowSize = windowSize;
        interactionPhase = lc_i_PhaseStart;
    }

    virtual void mouseDrag(v2f windowSize, v2f pos) override {
        previousMousePosition = pos;
        previousWindowSize = { windowSize.x, windowSize.y };
        interactionPhase = lc_i_PhaseContinue;
    }

    virtual void mouseUp(v2f windowSize, v2f pos) override {
        previousMousePosition = pos;
        previousWindowSize = { windowSize.x, windowSize.y };
        interactionPhase = lc_i_PhaseFinish;
    }

    virtual void mouseMove(v2f windowSize, v2f pos) override {
        previousMousePosition = pos;
        previousWindowSize = { windowSize.x, windowSize.y };
    }
};


int main(int argc, char* argv[])
{
    shared_ptr<LabRenderExampleApp> appPtr = make_shared<LabRenderExampleApp>();

	lab::checkError(lab::ErrorPolicy::onErrorThrow,
		lab::TestConditions::exhaustive, "main loop start");

	LabRenderExampleApp* app = appPtr.get();

    app->createScene();

    while (!app->isFinished())
    {
        app->frameBegin();
        app->render();
        app->frameEnd();
    }

    return 1;
}
