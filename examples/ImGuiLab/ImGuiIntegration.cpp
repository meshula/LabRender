
#include "ImGuiLab.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "LabImGui-gl-glfw.h"

namespace lab {

ImVec4 clear_color = { 0.45f, 0.55f, 0.6f, 1.f };
bool show_demo_window = true;
bool show_another_window = false;

ImGuiIntegration::ImGuiIntegration(GLFWwindow* window)
{
    lab_imgui_init_window("ImGuiLab", window);
}

ImGuiIntegration::~ImGuiIntegration()
{
    lab_imgui_shutdown();
}

void ImGuiIntegration::ui(int window_width, int window_height)
{
    lab_imgui_new_docking_frame();

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Lab")) {
            ImGui::MenuItem("Quit", 0, &quit);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    lab_imgui_begin_fullscreen_docking((float) window_width, (float) window_height);

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

    lab_imgui_end_fullscreen_docking();
}

void ImGuiIntegration::render() {
    lab_imgui_render("ImGuiLab");
}

} // lab
