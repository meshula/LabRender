//
//  main.cpp
//  LabRenderExamples
//
//  Created by Nick Porcino on 5/10/15.
//  Copyright (c) 2015 Planet IX. All rights reserved.
//

#include "OffScreenApp.h"
#include <LabRenderModelLoader/modelLoader.h>

#include <LabRender/Camera.h>
#include <LabRender/PassRenderer.h>
#include <LabRender/UtilityModel.h>
#include <LabRender/Utils.h>
#include <LabMath/LabMath.h>

#include <LabCmd/FFI.h>

using namespace std;
using lab::v2i;
using lab::v2f;
using lab::v3f;
using lab::v4f;

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
    lab::Camera camera;
    lab::CameraRigMode cameraRigMode;

    v2f initialMousePosition;
    v2f previousMousePosition;

    LabRenderExampleApp()
    : GLFWAppBase("LabRender Offscreen")
    , previousMousePosition(V2F(0,0))
    {
        const char * env = getenv("ASSET_ROOT");
        if (env)
        {
            lab::addPathVariable("{ASSET_ROOT}", env);
            lab::addPathVariable("{RESOURCE_ROOT}", env);
        }
        else
        {
            lab::addPathVariable("{ASSET_ROOT}", ASSET_ROOT);
            lab::addPathVariable("{RESOURCE_ROOT}", ASSET_ROOT);
        }

        //std::string path = "{ASSET_ROOT}/pipelines/deferred.json";
        //std::string path = "{ASSET_ROOT}/pipelines/shadertoy.json";
        std::string path = "{ASSET_ROOT}/pipelines/deferred-offscreen.json";
        std::cout << "Loading pipeline configuration " << path << std::endl;
        dr = make_shared<lab::Render::PassRenderer>();
        dr->configure(path.c_str());
        cameraRigMode = lab::CameraRigMode::TurnTableOrbit;
    }

    void createScene()
    {
        auto& meshes = drawList.deferredMeshes;

        //shared_ptr<lab::ModelBase> model = lab::Model::loadMesh("{ASSET_ROOT}/models/starfire.25.obj");
        shared_ptr<lab::Render::Model> model = lab::Render::loadMesh("{ASSET_ROOT}/models/ShaderBall/shaderBallNoCrease/shaderBall.obj");
        for (auto& i : model->parts())
            meshes.push_back({ lab::m44f_identity, i });

        const float pi = float(M_PI);

        for (int i = 0; i < 10; ++i)
        {
            float th = float(i)/10.f * 2.f * pi;
            float sx = cos(th);
            float sy = sin(th);

            shared_ptr<lab::Render::UtilityModel> mesh = make_shared<lab::Render::UtilityModel>();
            switch (i % 5)
            {
            case 0: mesh->createBox(75, 75, 75, 2,3,4, false, false); break;
            case 1: mesh->createCylinder(75, 100, 200, 20, 1, false);  break;
            case 2: mesh->createSphere(75, 32, 32, 0, 2.f*pi, -pi, 2.f*pi, false);   break;
            case 3: mesh->createIcosahedron(75);   break;
            case 4: mesh->createPlane(75, 75, 3, 3);   break;
            }
            auto m = lab::m44f_identity;
            m[3] = v4f{sx * 300.f, 100.f, sy * 300.f, 1};
            meshes.push_back({m, mesh});
        }

        static float foo = 0.f;
        camera.position = {foo, 0, -1000};
        lab::Bounds bounds = model->localBounds();
        //bounds = model->transform.transformBounds(bounds);
        camera.frame(bounds);
    }

    void render()
    {
        lab::checkError(lab::ErrorPolicy::onErrorThrow,
                        lab::TestConditions::exhaustive, "main loop start");

        v2i fbSize = frameBufferDimensions();

        drawList.modl = camera.mount.rotationTransform();
        drawList.view = camera.mount.viewTransform();
        drawList.proj = lab::perspective(camera.sensor, camera.optics, float(fbSize.x) / float(fbSize.y));

        lab::Render::PassRenderer::RenderLock rl(dr.get(), renderTime(), mousePosition());
        v2i fbOffset = V2I(0, 0);
        renderStart(rl, renderTime(), fbOffset, fbSize);

        dr->render(rl, fbSize, drawList);

        renderEnd(rl);

//        auto fb = dr->framebuffer("gbuffer");
	//    if (fb && fb->textures.size())
	  //      fb->textures[0]->save("C:\\Projects\\foo.png");

        lab::checkError(lab::ErrorPolicy::onErrorThrow,
                        lab::TestConditions::exhaustive, "main loop end");
    }

    virtual void keyPress(int key) override 
    {
        switch (key) {
            case GLFW_KEY_C: cameraRigMode = lab::CameraRigMode::Crane; break;
            case GLFW_KEY_D: cameraRigMode = lab::CameraRigMode::Dolly; break;
            case GLFW_KEY_T:
            case GLFW_KEY_O: cameraRigMode = lab::CameraRigMode::TurnTableOrbit; break;
        }
    }

    virtual void mouseDown(v2f windowSize, v2f pos) override 
    {
        previousMousePosition = pos;
        initialMousePosition = pos;
    }

    virtual void mouseDrag(v2f windowSize, v2f pos) override 
    {
        v2f delta = pos - previousMousePosition;
        previousMousePosition = pos;

        float distanceX = 20.f * delta.x / windowSize.x;
        float distanceY = -20.f * delta.y / windowSize.y;

        //printf("%f %f\n", pos.x, pos.y);

        lab::cameraRig_interact(camera, cameraRigMode, V2F(distanceX, distanceY));
    }

    virtual void mouseUp(v2f windowSize, v2f pos) override 
    {
        if (lab::vector_length(previousMousePosition - initialMousePosition) < 2) 
        {
            // test for clicked object
            //
        }
    }

    virtual void mouseMove(v2f windowSize, v2f pos) override 
    {
    }
};


int main(void)
{
    shared_ptr<LabRenderExampleApp> appPtr = make_shared<LabRenderExampleApp>();

    lab::checkError(lab::ErrorPolicy::onErrorThrow,
        lab::TestConditions::exhaustive, "main loop start");

    LabRenderExampleApp *app = appPtr.get();

    app->createScene();

    while (!app->isFinished())
    {
        app->frameBegin();
        app->render();
        app->frameEnd();
    }

    return 0;
}
