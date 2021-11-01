//
//  main.cpp
//  LabRenderExamples
//
//  Created by Nick Porcino on 5/10/15.
//  Copyright (c) 2015 Planet IX. All rights reserved.
//

#include "../LabRenderDemoApp.h"
#include <LabRenderModelLoader/modelLoader.h>

#include <LabRender/PassRenderer.h>
#include <LabRender/UtilityModel.h>
#include <LabRender/Utils.h>
#include <LabMath/LabMath.h>
#include <LabText/LabText.h>

#include <LabCmd/FFI.h>

#include <Effekseer/Effekseer.h>
#include <EffekseerRendererGL/EffekseerRendererGL.h>

using namespace std;
using lab::v2i;
using lab::v2f;
using lab::v3f;
using lab::v4f;

class LabEffekseer
{
public:
    ::EffekseerRendererGL::RendererRef fxr_renderer;
    ::Effekseer::ManagerRef fxr_manager;
    ::Effekseer::EffectRef fxr_effect;

    ::Effekseer::Handle effect_handle;

    ~LabEffekseer()
    {
        // Dispose the manager
        // マネージャーの破棄
        fxr_manager.Reset();

        // Dispose the renderer
        // レンダラーの破棄
        fxr_renderer.Reset();
    }

    void init()
    {
        // Create a renderer of effects
        // エフェクトのレンダラーの作成
        fxr_renderer = ::EffekseerRendererGL::Renderer::Create(8000, EffekseerRendererGL::OpenGLDeviceType::OpenGL3);

        // Create a manager of effects
        // エフェクトのマネージャーの作成
        fxr_manager = ::Effekseer::Manager::Create(8000);

        // Sprcify rendering modules
        // 描画モジュールの設定
        fxr_manager->SetSpriteRenderer(fxr_renderer->CreateSpriteRenderer());
        fxr_manager->SetRibbonRenderer(fxr_renderer->CreateRibbonRenderer());
        fxr_manager->SetRingRenderer(fxr_renderer->CreateRingRenderer());
        fxr_manager->SetTrackRenderer(fxr_renderer->CreateTrackRenderer());
        fxr_manager->SetModelRenderer(fxr_renderer->CreateModelRenderer());

        // Specify a texture, model, curve and material loader
        // It can be extended by yourself. It is loaded from a file on now.
        // テクスチャ、モデル、カーブ、マテリアルローダーの設定する。
        // ユーザーが独自で拡張できる。現在はファイルから読み込んでいる。
        fxr_manager->SetTextureLoader(fxr_renderer->CreateTextureLoader());
        fxr_manager->SetModelLoader(fxr_renderer->CreateModelLoader());
        fxr_manager->SetMaterialLoader(fxr_renderer->CreateMaterialLoader());
        fxr_manager->SetCurveLoader(Effekseer::MakeRefPtr<Effekseer::CurveLoader>());

        // Load an effect
        // エフェクトの読込
        std::string filename = lab::expandPath(u8"{ASSET_ROOT}/effects/Laser01.efk");
        size_t len = filename.size() * 2 + 2;
        char16_t* buff = (char16_t*)malloc(len);
        tsConvertUtf8ToUtf16(buff, len, filename.c_str());
        fxr_effect = Effekseer::Effect::Create(fxr_manager, buff);
        free(buff);
    }

    void play()
    {
        // Play an effect
        // エフェクトの再生
        effect_handle = fxr_manager->Play(fxr_effect, 0, 0, 0);
    }

    void render()
    {
        // Update the manager
        // マネージャーの更新
        fxr_manager->Update();

        // Begin to rendering effects
        // エフェクトの描画開始処理を行う。
        fxr_renderer->BeginRendering();

        // Render effects
        // エフェクトの描画を行う。
        fxr_manager->Draw();

        // Finish to rendering effects
        // エフェクトの描画終了処理を行う。
        fxr_renderer->EndRendering();
    }
};

class ParticlesSceneBuilder : public lab::LabRenderAppScene {
public:
    LabEffekseer particlePlug;

    virtual ~ParticlesSceneBuilder() = default;

    virtual void build() override 
    {
        // load a pipeline
        std::string path = "{ASSET_ROOT}/pipelines/deferred-fxaa.labfx";
        std::cout << "Loading pipeline configuration " << path << std::endl;
        dr = std::make_shared<lab::Render::PassRenderer>();
        dr->configure(path.c_str());

        // create a drawlist

        auto& meshes = drawList.deferredMeshes;

        //shared_ptr<lab::ModelBase> model = lab::Model::loadMesh("{ASSET_ROOT}/models/starfire.25.obj");
        shared_ptr<lab::Render::Model> model = lab::Render::loadMesh("{ASSET_ROOT}/models/ShaderBall/shaderBallNoCrease/shaderBall.obj");
        if (false&&model)
            for (auto& i : model->parts())
                meshes.push_back({ lab::m44f_identity, i });

        const float pi = float(M_PI);
        const float sc = 0.005f;
        const float r = 3;

        for (int i = 0; i < 10; ++i)
        {
            float th = float(i) / 10.f * 2.f * pi;
            float sx = cos(th);
            float sy = sin(th);

            shared_ptr<lab::Render::UtilityModel> mesh = make_shared<lab::Render::UtilityModel>();
            switch (i % 5)
            {
                // box has a bug!!
            case 0: mesh->createBox(sc * 75, sc * 75, sc * 75, 2, 3, 4, false, false); break;
            case 1: mesh->createCylinder(sc * 75, sc * 100, sc * 200, 20, 1, false);  break;
            case 2: mesh->createSphere(sc * 75, 32, 32, 0, 2.f * pi, -pi, 2.f * pi, false);   break;
            case 3: mesh->createIcosahedron(sc * 75);   break;
            case 4: mesh->createPlane(sc * 75, sc * 75, 3, 3);   break;
            }
            auto m = lab::m44f_identity;
            m[3] = v4f{ sx * r, r * 0.25f, sy * r, 1 };
            meshes.push_back({ m, mesh });
        }

        // set the initial camera

        if (model) {
            static float foo = -200.f;
            camera.mount.transform.position = { foo, 200, -1000 };
            lab::Bounds bounds = model->localBounds();
            //bounds = model->transform.transformBounds(bounds);
            v3f& mn = bounds.first;
            v3f& mx = bounds.second;
            lc_camera_frame(&camera, lc_v3f{ mn.x, mn.y, mn.z }, lc_v3f{ mx.x, mx.y, mx.z });
        }
        lc_mount_look_at(&camera.mount,
            lc_v3f{ 20,5,30 },
            lc_v3f{ 0,0,0 }, lc_v3f{ 0,1,0 });

        particlePlug.init();
    }
};

class ParticlesApp : public lab::LabRenderExampleApp {
public:
    ParticlesSceneBuilder* particles_scene = nullptr;
    ParticlesApp() : lab::LabRenderExampleApp("Particles") {
        particles_scene = new ParticlesSceneBuilder();
        scene = particles_scene;
    }

    virtual ~ParticlesApp() {
        delete scene;
    }
};

int main(void)
{
    shared_ptr<ParticlesApp> appPtr = make_shared<ParticlesApp>();

    lab::checkError(lab::ErrorPolicy::onErrorThrow,
        lab::TestConditions::exhaustive, "main loop start");

    ParticlesApp* app = appPtr.get();
    app->createScene();

    // Play an effect
    // エフェクトの再生
    LabEffekseer* plug = &app->particles_scene->particlePlug;
    plug->play();

    while (!app->isFinished())
    {
        app->update();
        app->frameBegin();
        app->render();

        {
            v2i fbSize = app->frameBufferDimensions();


    //            then move all the rest of this into the nascent plug


            if (app->scene) {

                // Specify a projection matrix
                // 投影行列を設定
                lc_m44f proj = lc_camera_perspective(&app->scene->camera, float(fbSize.x) / float(fbSize.y));
                plug->fxr_renderer->SetProjectionMatrix(
                    *reinterpret_cast<::Effekseer::Matrix44*>(&proj));

                // Specify a camera matrix
                // カメラ行列を設定
                lc_m44f view = lc_mount_gl_view_transform(&app->scene->camera.mount);

                plug->fxr_renderer->SetCameraMatrix(
                    *reinterpret_cast<::Effekseer::Matrix44*>(&view));
            }
            else {
                // Specify a position of view
                // 視点位置を確定
                auto g_position = ::Effekseer::Vector3D(10.0f, 5.0f, 20.0f);

                // Specify a projection matrix
                // 投影行列を設定
                plug->fxr_renderer->SetProjectionMatrix(
                    ::Effekseer::Matrix44().PerspectiveFovRH_OpenGL(90.0f / 180.0f * 3.14f,
                        (float)fbSize.x / (float)fbSize.y, 1.0f, 500.0f));

                // Specify a camera matrix
                // カメラ行列を設定
                plug->fxr_renderer->SetCameraMatrix(
                    ::Effekseer::Matrix44().LookAtRH(g_position,
                        ::Effekseer::Vector3D(0.0f, 0.0f, 0.0f),
                        ::Effekseer::Vector3D(0.0f, 1.0f, 0.0f)));
            }


            // Move the effect
            // エフェクトの移動
            plug->fxr_manager->AddLocation(plug->effect_handle, ::Effekseer::Vector3D(0.2f, 0.0f, 0.0f));

            plug->render(); // this function is evolving into the plugMgr plug
        }


        app->frameEnd();
    }


    return 0;
}
