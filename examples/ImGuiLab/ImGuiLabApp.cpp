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


#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "LabImGui/LabImGui.h"


using namespace std;
using lab::v2i;
using lab::v2f;
using lab::v3f;
using lab::v4f;

namespace lab {

    class ImGuiIntegration : public SupplementalGuiHandler
    {
        lab_WindowState _ws;
    public:
        ImGuiIntegration(GLFWwindow*);
        ~ImGuiIntegration();

        virtual void ui(int window_width, int widnow_height) override;
        virtual void render() override;
    };


    ImVec4 clear_color = { 0.45f, 0.55f, 0.6f, 1.f };
    bool show_demo_window = true;
    bool show_another_window = false;

    ImGuiIntegration::ImGuiIntegration(GLFWwindow* window)
    {
//        lab_imgui_init();
        //lab_imgui_init_window("ImGuiLab", window);
    // Setup Dear ImGui binding
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 410");

        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        // Setup style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsClassic();

        // Load Fonts
        // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
        // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
        // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
        // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
        // - Read 'misc/fonts/README.txt' for more instructions and details.
        // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
        //io.Fonts->AddFontDefault();
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
        //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
        //IM_ASSERT(font != NULL);

        /// @TODO - WindowState should be in the map, and it should include the dpi scale factor
        ImGuiStyle& style = ImGui::GetStyle();
        style.ScaleAllSizes(2);
    }

    ImGuiIntegration::~ImGuiIntegration()
    {
        // lab_imgui_shutdown();
    }

    void ImGuiIntegration::ui(int window_width, int window_height)
    {
        /// @TODO retina fb scaling
        _ws = { window_width, window_height, window_width, window_height, true };
        lab_imgui_new_docking_frame(&_ws);

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("Lab")) {
                ImGui::MenuItem("Quit", 0, &quit);
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        lab_imgui_begin_fullscreen_docking(&_ws);

        // 1. Show a simple window.
        {
            ImGui::Begin("Hello World");
            static float f = 0.0f;
            static int counter = 0;
            ImGui::Text("Hello, world!");                           // Display some text (you can use a format string too)
            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our windows open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (NB: most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // 2. Show another simple window. In most cases you will use an explicit Begin/End pair to name your windows.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        // 3. Show the ImGui demo window. Most of the sample code is in ImGui::ShowDemoWindow(). Read its code to learn more about Dear ImGui!
        if (show_demo_window)
        {
            ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver); // Normally user code doesn't need/want to call this because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
            ImGui::ShowDemoWindow(&show_demo_window);
        }

        lab_imgui_end_fullscreen_docking(&_ws);
    }

    void ImGuiIntegration::render() {
        lab_imgui_render(&_ws);
    }
}

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


class ExampleSceneBuilder : public lab::LabRenderAppScene {
public:
    lab::OSCServer oscServer;
    lab::WebSocketsServer wsServer;

    ExampleSceneBuilder()
        : oscServer("labrender")
        , wsServer("labrender")
    {}

    virtual ~ExampleSceneBuilder() = default;

    virtual void build() override {
        // pipeline
        std::string path = "{ASSET_ROOT}/pipelines/deferred-fxaa.labfx";
        std::cout << "Loading pipeline configuration " << path << std::endl;
        dr = make_shared<lab::Render::PassRenderer>();
        dr->configure(path.c_str());

        // drawlist
        auto& meshes = drawList.deferredMeshes;

        //shared_ptr<lab::ModelBase> model = lab::Model::loadMesh("{ASSET_ROOT}/models/starfire.25.obj");
        shared_ptr<lab::Render::Model> model = lab::Render::loadMesh("{ASSET_ROOT}/models/ShaderBall/shaderBallNoCrease/shaderBall.obj");
        if (model) {
            for (auto& i : model->parts())
                meshes.push_back({ lab::m44f_identity, i });
        }

        shared_ptr<lab::Render::UtilityModel> cube = make_shared<lab::Render::UtilityModel>();
        cube->createCylinder(0.5f, 0.5f, 2.f, 16, 3, false);
        meshes.push_back({ lab::m44f_identity, cube });

        // set the initial camera

        if (model) {
            static float foo = 0.f;
            camera.mount.transform.position = { foo, 0, -1000 };
            lab::Bounds bounds = model->localBounds();
            //bounds = model->transform.transformBounds(bounds);
            v3f& mn = bounds.first;
            v3f& mx = bounds.second;
            lc_camera_frame(&camera, lc_v3f{ mn.x, mn.y, mn.z }, lc_v3f{ mx.x, mx.y, mx.z });
        }

        // servers

        shared_ptr<lab::Command> command = make_shared<PingCommand>();
        oscServer.registerCommand(command);
        command = make_shared<LoadMeshCommand>(dr.get(), &drawList);
        oscServer.registerCommand(command);

        const int PORT_NUM = 9109;
        oscServer.start(PORT_NUM);

        wsServer.start(PORT_NUM + 1);
    }
};


class ExampleApp : public lab::LabRenderExampleApp {
public:
    shared_ptr<lab::Render::PassRenderer> dr;
    shared_ptr<lab::ImGuiIntegration> imgui;
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
        imgui = shared_ptr<lab::ImGuiIntegration>(new lab::ImGuiIntegration(window));
        _supplemental = imgui.get();
    }

    virtual ~ExampleApp() {
        delete scene;
    }


};


/// @TODO turn the imgui layer into layer plug like effekseer

int main(void)
{
    shared_ptr<ExampleApp> appPtr = make_shared<ExampleApp>();

    lab::checkError(lab::ErrorPolicy::onErrorThrow,
        lab::TestConditions::exhaustive, "main loop start");

    ExampleApp* app = appPtr.get();

    app->createScene();

    while (!app->isFinished())
    {
        app->frameBegin();
        app->update();      // UI requires a bound context
        app->render();
        app->frameEnd();
    }

    exit(EXIT_SUCCESS);
}

