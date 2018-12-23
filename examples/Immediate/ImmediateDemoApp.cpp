//
//  main.cpp
//  LabRenderExamples
//
//  Created by Nick Porcino on 5/10/15.
//  Copyright (c) 2015 Planet IX. All rights reserved.
//

#include "ImmediateDemoApp.h"
#include "InsectAI/InsectAI3.h"
#include "InsectAI/InsectAI.h"
#include "InsectAI/InsectAI_Actuator.h"
#include "InsectAI/InsectAI_Agent.h"
#include "InsectAI/InsectAI_Sensor.h"
#include "InsectAI/InsectAI_Sensors.h"

#include <LabRender/Camera.h>
#include <LabRender/Immediate.h>
#include <LabMath/LabMath.h>
#include <LabRender/PassRenderer.h>
#include <LabRender/UtilityModel.h>
#include <LabRender/Utils.h>
#include <LabRenderModelLoader/modelLoader.h>

#include <LabCmd/FFI.h>

#include <fmt/format.h>

// ping loader for this demo

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
    "basu.png",
    "bat.png",
    "behemoth.png",
    "crow.png",
    "dragon_zombie.png",
    "fire_whirl.png",
    "giant_pignon.png",
    "night_spirit.png",
    "orangebell.png",
    "petit.png",
    "polish.png",
    "power_critter.png",
};

enum class Image : int {
    Basu = 0,
    Bat, Behemoth, Crow, Dragon, Fire, Giant, Night, OrangeBell, Petit, Polish, Power
};

int images_count = sizeof(image_names) / sizeof(*image_names);
tpImage images[sizeof(image_names) / sizeof(*image_names)];
lab::ImmSpriteId sprite_ids[sizeof(image_names) / sizeof(*image_names)];


class LabRenderExampleApp : public lab::GLFWAppBase {
public:
    shared_ptr<lab::Render::PassRenderer> dr;
    lab::Render::DrawList drawList;
    lab::Camera camera;
	lab::CameraRigMode cameraRigMode;

    lab::OSCServer oscServer;
    lab::WebSocketsServer wsServer;

    v2f initialMousePosition;
    v2f previousMousePosition;

    shared_ptr<InsectAI::Engine> pEngine;
    shared_ptr<InsectAI::Agent> pVehicle;
    float epoch{ 0.f };



    InsectAI3::Agent _agent;


    LabRenderExampleApp()
    : GLFWAppBase("LabRender ImmediateExample")
    , oscServer("labrender")
    , wsServer("labrender")
    , previousMousePosition(V2F(0,0))
    {
		const char * env = getenv("ASSET_ROOT");
		if (env)
			lab::addPathVariable("{ASSET_ROOT}", env);
		else
			lab::addPathVariable("{ASSET_ROOT}", ASSET_ROOT);

		//string path = "{ASSET_ROOT}/pipelines/deferred.json";
		//string path = "{ASSET_ROOT}/pipelines/shadertoy.json";
		//string path = "{ASSET_ROOT}/pipelines/deferred_fxaa.json";
		string path = "{ASSET_ROOT}/pipelines/deferred-fxaa.labfx";
		std::cout << "Loading pipeline configuration " << path << std::endl;
        dr = make_shared<lab::Render::PassRenderer>();
        dr->configure(path.c_str());

        for (int i = 0; i < images_count; ++i)
        {
            auto p = fmt::format("{{ASSET_ROOT}}/sprites/{}", image_names[i]);
            images[i] = tpLoadPNG(lab::expandPath(p.c_str()).c_str());
            uint8_t* pixels = reinterpret_cast<uint8_t*>(images[i].pix);
            shared_ptr<uint8_t> pixelPtr(pixels, [](uint8_t*) {});
            sprite_ids[i] = lab::SpriteId(pixelPtr, images[i].w, images[i].h);
        }
    }

    ~LabRenderExampleApp()
    {
        wsServer.stop();
        oscServer.stop();

        for (int i = 0; i < images_count; ++i)
            tpFreePNG(images + i);
    }

    void createScene()
	{
        auto& meshes = drawList.deferredMeshes;

        //shared_ptr<lab::ModelBase> model = lab::Model::loadMesh("{ASSET_ROOT}/models/starfire.25.obj");
        shared_ptr<lab::Render::Model> model = lab::Render::loadMesh("{ASSET_ROOT}/models/ShaderBall/shaderBallNoCrease/shaderBall.obj");
        for (auto& i : model->parts())
            meshes.push_back({ lab::m44f_identity, i });

		shared_ptr<lab::Render::UtilityModel> cube = make_shared<lab::Render::UtilityModel>();
		cube->createCylinder(0.5f, 0.5f, 2.f, 16, 3, false);
        meshes.push_back({ lab::m44f_identity, cube });

        static float foo = 0.f;
        camera.position = {foo, 0, -1000};
        lab::Bounds bounds = model->localBounds();
        //bounds = model->transform.transformBounds(bounds);
        camera.frame(bounds);


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

    void updateScene()
    {
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
        //---------------------------------------
        // demo immediate 2d drawing

        lab::ImmRenderContext irc;

        irc.rectangle({ 20,20 }, { 200, 100 }, 0xff00ffff);
        irc.quad({ 100,100 }, { 150, 200 }, { 100, 175 }, { 50, 200 }, 0xff0080ff);
        //dl.AddText({ 50, 50 }, 0xffffffff, "This is a test");

        static float rotate = 0;
        static float scale = 3;

//        for (int i = 0; i < 12; ++i)
//            irc.sprite(sprite_ids[i], 0, scale, rotate, 200 + 75 * float(i), 200);

        float x = _agent.body.state0.pos.x;
        float y = _agent.body.state0.pos.y;
        irc.sprite(sprite_ids[0], 0, scale, rotate, x, y);

        //rotate += 0.05f;

        irc.render(fbSize.x, fbSize.y);
        //---------------------------------------

        renderEnd(rl);

        lab::checkError(lab::ErrorPolicy::onErrorLog,
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

    virtual void mouseDrag(v2f windowSize, v2f pos) override {
        v2f delta = pos - previousMousePosition;
        previousMousePosition = pos;

        float distanceX = 20.f * delta.x / windowSize.x;
        float distanceY = -20.f * delta.y / windowSize.y;

		//printf("%f %f\n", pos.x, pos.y);

        lab::cameraRig_interact(camera, cameraRigMode, V2F(distanceX, distanceY));
    }

    virtual void mouseUp(v2f windowSize, v2f pos) override {
        if (lab::vector_length(previousMousePosition - initialMousePosition) < 2) {
            // test for clicked object
            //
        }
    }
    virtual void mouseMove(v2f windowSize, v2f pos) override {
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
        app->updateScene();
    }

    return 0;
}
