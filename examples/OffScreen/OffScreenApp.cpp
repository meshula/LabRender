//
//  main.cpp
//  LabRenderExamples
//
//  Created by Nick Porcino on 5/10/15.
//  Copyright (c) 2015 Planet IX. All rights reserved.
//

#include "../LabRenderDemoApp.h"
#include <LabRenderModelLoader/modelLoader.h>

#include <LabCamera/LabCamera.h>
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

class OffScreenSceneBuilder : public lab::LabRenderAppScene {
    virtual ~OffScreenSceneBuilder() = default;
    virtual void build() override {
        // load a pipeline
        std::string path = "{ASSET_ROOT}/pipelines/deferred-offscreen.labfx";
        std::cout << "Loading pipeline configuration " << path << std::endl;
        dr = make_shared<lab::Render::PassRenderer>();
        dr->configure(path.c_str());
        auto pass = dr->findPass<lab::Render::PassRenderer::Pass>("blit");
        if (pass)
            pass->active = true;
        auto& meshes = drawList.deferredMeshes;

        //shared_ptr<lab::ModelBase> model = lab::Model::loadMesh("{ASSET_ROOT}/models/starfire.25.obj");
        shared_ptr<lab::Render::Model> model = lab::Render::loadMesh("{ASSET_ROOT}/models/ShaderBall/shaderBallNoCrease/shaderBall.obj");
        if (model) {
            for (auto& i : model->parts())
                meshes.push_back({ lab::m44f_identity, i });
        }

        const float pi = float(M_PI);

        for (int i = 0; i < 10; ++i)
        {
            float th = float(i) / 10.f * 2.f * pi;
            float sx = cos(th);
            float sy = sin(th);

            shared_ptr<lab::Render::UtilityModel> mesh = make_shared<lab::Render::UtilityModel>();
            switch (i % 5)
            {
            case 0: mesh->createBox(75, 75, 75, 2, 3, 4, false, false); break;
            case 1: mesh->createCylinder(75, 100, 200, 20, 1, false);  break;
            case 2: mesh->createSphere(75, 32, 32, 0, 2.f * pi, -pi, 2.f * pi, false);   break;
            case 3: mesh->createIcosahedron(75);   break;
            case 4: mesh->createPlane(75, 75, 3, 3);   break;
            }
            auto m = lab::m44f_identity;
            m[3] = v4f{ sx * 300.f, 100.f, sy * 300.f, 1 };
            meshes.push_back({ m, mesh });
        }

        if (model) {
            static float foo = 0.f;
            camera.mount.transform.position = { foo, 0, -1000 };
            lab::Bounds bounds = model->localBounds();
            //bounds = model->transform.transformBounds(bounds);
            v3f& mn = bounds.first;
            v3f& mx = bounds.second;
            lc_camera_frame(&camera, lc_v3f{ mn.x, mn.y, mn.z }, lc_v3f{ mx.x, mx.y, mx.z });
        }

    }

};

class OffScreenApp : public lab::LabRenderExampleApp {
public:
    OffScreenApp() : lab::LabRenderExampleApp("Deferred Rendering") {
        scene = new OffScreenSceneBuilder();
    }

    virtual ~OffScreenApp() {
        delete scene;
    }

};


int main(void)
{
    shared_ptr<OffScreenApp> appPtr = make_shared<OffScreenApp>();

    lab::checkError(lab::ErrorPolicy::onErrorThrow,
        lab::TestConditions::exhaustive, "main loop start");

    OffScreenApp*app = appPtr.get();

    app->createScene();

    while (!app->isFinished())
    {
        app->update();
        app->frameBegin();
        app->render();
        app->frameEnd();
    }

    return 0;
}
