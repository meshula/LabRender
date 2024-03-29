//
//  main.cpp
//  LabRenderExamples
//
//  Created by Nick Porcino on 5/10/15.
//  Copyright (c) 2015 Planet IX. All rights reserved.
//

#include "../LabRenderDemoApp.h"
#include "InsectAI/InsectAI3.h"
#include "InsectAI/InsectAI.h"
#include "InsectAI/InsectAI_Actuator.h"
#include "InsectAI/InsectAI_Agent.h"
#include "InsectAI/InsectAI_Sensor.h"
#include "InsectAI/InsectAI_Sensors.h"

#include <LabCamera/LabCamera.h>
#include <LabRender/Immediate.h>
#include <LabMath/LabMath.h>
#include <LabRender/PassRenderer.h>
#include <LabRender/UtilityModel.h>
#include <LabRender/Utils.h>
#include <LabRenderModelLoader/modelLoader.h>

#include <LabCmd/FFI.h>

// png loader for this demo

#define TINYALLOC_IMPLEMENTATION
#include "tinyalloc.h"

#define TP_ALLOC(size) TINYALLOC_ALLOC(size, 0)
#define TP_FREE(ptr) TINYALLOC_FREE(ptr, 0)
#define TP_CALLOC(count, element_size) TINYALLOC_CALLOC(count, element_size, 0)
#define TINYPNG_IMPLEMENTATION
#include "tinypng.h"

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
            [](const lab::Command & cmd, const string & path, const vector<Argument> &, lab::Ack & ack) {
                ack.payload.push_back(Argument("pong"));
                cout << "Sending pong" << endl;
        };
    }
    virtual ~PingCommand() {}

    virtual string name() const override { return "ping"; }
};

class LoadMeshCommand : public lab::Command
{
public:
    LoadMeshCommand(lab::Render::Renderer * renderer, lab::Render::DrawList * drawlist)
    : _renderer(renderer), _drawlist(drawlist)
    {
        run = [this](const lab::Command & cmd, const string & path, const vector<Argument> & args, lab::Ack & ack)
        {
            string filepath = args[0].stringArg;
            _renderer->enqueCommand([this, filepath]()
            {
                shared_ptr<lab::Render::Model> model = lab::Render::loadMesh(filepath);
                if (model)
                {
                    _drawlist->deferredMeshes.clear();
                    for (auto& i : model->parts())
                        _drawlist->deferredMeshes.emplace_back(pair<lab::m44f, shared_ptr<lab::Render::ModelBase>>( lab::m44f_identity, i ));
                }
            });
        };

        parameterSpecification.push_back(make_pair<string, SemanticType>("path", SemanticType::string_st));
    }

    virtual ~LoadMeshCommand() {}
    virtual string name() const override { return "loadMesh"; }

    lab::Render::Renderer * _renderer;
    lab::Render::DrawList * _drawlist;
};


// example file/asset i/o system
const char* image_names[] = {
    "rocket.png",
    "rocket-blue_tank.png",
    "rocket-booster.png",
    "rocket-capsule.png",
    "rocket-escape.png",
    "rocket-landing_gear.png",
    "rocket-main_stage.png",
    "rocket-module.png",
    "rocket-red_tank.png"
};


enum class Image : int {
    Rocket = 0,
    Rocket_BlueTank, Rocket_Booster, Rocket_Capsule, Rocket_Escape, Rocket_LandingGear,
    Rocket_MainStage, Rocket_Module, Rocket_RedTank
};

int images_count = sizeof(image_names) / sizeof(*image_names);
tpImage images[sizeof(image_names) / sizeof(*image_names)];
lab::ImmSpriteId sprite_ids[sizeof(image_names) / sizeof(*image_names)];

class ImmediateSceneBuilder : public lab::LabRenderAppScene {
public:
    shared_ptr<InsectAI::Engine> pEngine;
    shared_ptr<InsectAI::Agent> pVehicle;
    float epoch{ 0.f };
    InsectAI3::Agent _agent;
    lab::OSCServer oscServer;
    lab::WebSocketsServer wsServer;

    ImmediateSceneBuilder()
    : lab::LabRenderAppScene() 
    , oscServer("labrender")
    , wsServer("labrender")
    {
    }

    virtual ~ImmediateSceneBuilder()
    {
        wsServer.stop();
        oscServer.stop();

        for (int i = 0; i < images_count; ++i)
            tpFreePNG(images + i);

    }

    virtual void build() override {
        // load a pipeline
        //string path = "{ASSET_ROOT}/pipelines/deferred.json";
        //string path = "{ASSET_ROOT}/pipelines/shadertoy.json";
        //string path = "{ASSET_ROOT}/pipelines/deferred_fxaa.json";
        string path = "{ASSET_ROOT}/pipelines/deferred-fxaa.labfx";
        std::cout << "Loading pipeline configuration " << path << std::endl;
        dr = make_shared<lab::Render::PassRenderer>();
        dr->configure(path.c_str());

        // load some sprites
        for (int i = 0; i < images_count; ++i)
        {
            std::string p = std::string("{ASSET_ROOT}/sprites/") + image_names[i];
            images[i] = tpLoadPNG(lab::expandPath(p.c_str()).c_str());
            uint8_t* pixels = reinterpret_cast<uint8_t*>(images[i].pix);
            shared_ptr<uint8_t> pixelPtr(pixels, [](uint8_t*) {});
            sprite_ids[i] = lab::SpriteId(pixelPtr, images[i].w, images[i].h);
        }
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

        pVehicle = std::make_shared<InsectAI::Agent>();
        auto pLightSensor = std::make_shared<InsectAI::LightSensor>(pVehicle.get());
        auto pMotor = std::make_shared<InsectAI::Actuator>(InsectAI::Actuator::kMotor);
        pVehicle->AddSensor(pLightSensor); // storage
        pVehicle->AddActuator(pMotor);     // storage
        pMotor->AddInput(pLightSensor);    // connection

        pEngine = std::make_shared<InsectAI::Engine>();
        pEngine->AddEntity(pVehicle.get());

        shared_ptr<lab::Command> command = make_shared<PingCommand>();
        oscServer.registerCommand(command);
        command = make_shared<LoadMeshCommand>(dr.get(), &drawList);
        oscServer.registerCommand(command);

        const int PORT_NUM = 9109;
        oscServer.start(PORT_NUM);

        wsServer.start(PORT_NUM + 1);
    }

    virtual void render(lab::Render::Renderer::RenderLock& rl, v2i fbSize) override {
        if (dr) {
            dr->render(rl, fbSize, drawList);
            //---------------------------------------
            // demo immediate 2d drawing

            lab::ImmRenderContext irc;

            irc.rectangle({ 20,20 }, { 200, 100 }, 0xff00ffff);
            irc.quad({ 100,100 }, { 150, 200 }, { 100, 175 }, { 50, 200 }, 0xff0080ff);
            //dl.AddText({ 50, 50 }, 0xffffffff, "This is a test");

            static float rotate = -0.5f * M_PI;
            static float scale = 5;

            for (int i = 1; i < int(Image::Rocket_RedTank) + 1; ++i)
                irc.sprite(sprite_ids[i], 0, scale, rotate, 200 + 120 * float(i), 200);

            float x = _agent.body.state0.pos.x;
            float y = _agent.body.state0.pos.y;
            irc.sprite(sprite_ids[0], 0, scale, rotate, x, y);

            //rotate += 0.05f;

            irc.render(fbSize.x, fbSize.y);
            //---------------------------------------
        }
    }

    virtual void update() override {
        float dt = 1.f / 60.f;
        pEngine->UpdateEntities(epoch, dt);
        const float kSteeringSpeed = 0.25f;

        _agent.eval(dt);

#if 0
        auto pState = pVehicle->state();

        // connect actuators to physics
        for (auto& actuator : pVehicle->m_Actuators)
        {
            switch (actuator->GetKind())
            {
            case InsectAI::Actuator::kMotor:
                pState.yaw_pitch_roll[0] += kSteeringSpeed /* * actuator->mSteeringActivation*/;
                //pState->yaw_pitch_roll[0] = 0.25f * kPi;
                if (pState.yaw_pitch_roll[0] < 0.0f) pState.yaw_pitch_roll[0] += 2.0f * float(M_PI);
                else if (pState.yaw_pitch_roll[0] > 2.0f * float(M_PI)) pState.yaw_pitch_roll[0] -= 2.0f * float(M_PI);

                float activation = 0.005f * actuator->mActivation;
                pState.position[0] += /*pVehicle->mMaxSpeed * */ sinf(pState.yaw_pitch_roll[0]) * activation;			// rotation of zero moves forward on y axis
                pState.position[1] += /*pVehicle->mMaxSpeed * */ cosf(pState.yaw_pitch_roll[0]) * activation;
                break;
            }
        }

        Lab::DynamicState& t0 = pVehicle->state();
        Lab::DynamicState t1 = t0;
        t1.velocity += (a * dt) / 2;
        t1.angular_momentum += (t0->torque * dt) / 2;
        t1.position += t0->velocity * dt + (a * dt) / 2;
        t1.angular_velocity = ...
#endif
    }
};

class ImmediateApp : public lab::LabRenderExampleApp {
public:

    ImmediateApp() : lab::LabRenderExampleApp("Immediate Rendering") {
        scene = new ImmediateSceneBuilder();
    }

    virtual ~ImmediateApp() {
        delete scene;
    }
};


int main(void)
{
    shared_ptr<ImmediateApp> appPtr = make_shared<ImmediateApp>();

	lab::checkError(lab::ErrorPolicy::onErrorThrow,
		            lab::TestConditions::exhaustive, "main loop start");

    ImmediateApp*app = appPtr.get();
    app->createScene();

    while (!app->isFinished())
    {
        app->frameBegin();
        app->render();
        app->frameEnd();
        app->update();
    }

    return 0;
}
