//
//  main.cpp
//  LabRenderExamples
//
//  Created by Nick Porcino on 5/10/15.
//  Copyright (c) 2015 Planet IX. All rights reserved.
//

#include "../LabRenderDemoApp.h"
#include "RenderMeshes.hpp"

#ifdef HAVE_DEARIMGUI
#include "LabImgui/LabImGui.h"
#include "imgui.h"
#include "implot.h"

void imgui_frame()
{
    lab_WindowState ws;
    lab_imgui_window_state("Hello LabImGui", &ws);
    if (!ws.valid)
        return;

    //------------ start the Dear ImGui portion

    lab_imgui_new_docking_frame(&ws);
    lab_imgui_begin_fullscreen_docking(&ws);

    //------------ custom begin

    ImGui::Begin("Hello LabImgui");
    ImGui::Button("hi!");
    ImGui::End();

    ImGui::Begin("Another Window");
    ImGui::Button("hi!###1");
    ImGui::End();

    static bool demo_window = true;
    ImPlot::ShowDemoWindow(&demo_window);

    //------------ custom end

    lab_imgui_end_fullscreen_docking(&ws);
}

int main(int argc, const char* argv[]) try
{
    @autoreleasepool {
        const char* asset_root = lab_application_resource_path(argv[0], "share/lab_font_demo/");
        lab_imgui_init(argc, argv, asset_root);
        lab_imgui_create_window("Hello LabImGui", 1024, 768, nullptr, imgui_frame);
        lab_imgui_shutdown();
    }
    return EXIT_SUCCESS;
}
catch (std::exception& exc)
{
    std::cerr << exc.what();
    return EXIT_FAILURE;
}

#endif

#ifdef HAVE_GLFW

class ExampleApp : public lab::LabRenderExampleApp {
public:
    shared_ptr<lab::Render::PassRenderer> dr;
    lab::Render::DrawList drawList;

    lc_camera camera;
    lc_interaction* camera_controller = nullptr;
    lc_i_Mode cameraRigMode = lc_i_ModeTurnTableOrbit;
    lc_i_Phase interactionPhase = lc_i_PhaseNone;

    v2f initialMousePosition;
    v2f previousMousePosition;
    v2f previousWindowSize;

    ExampleApp() : lab::LabRenderExampleApp("Example") {
        scene = new ExampleSceneBuilder();
    }

    virtual ~ExampleApp() {
        delete scene;
    }
};

int main(void)
{
    shared_ptr<ExampleApp> appPtr = make_shared<ExampleApp>();

	lab::checkError(lab::ErrorPolicy::onErrorThrow,
		lab::TestConditions::exhaustive, "main loop start");

    ExampleApp* app = appPtr.get();

    app->createScene();

    while (!app->isFinished())
    {
        app->update();
        app->frameBegin();
        app->render();
        app->frameEnd();
    }

    return EXIT_SUCCESS;
}

#endif
