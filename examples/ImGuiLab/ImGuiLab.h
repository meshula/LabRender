//
//  ImGuiLab.h
//  LabRenderExamples
//
//  Created by Nick Porcino on 6/5/15.
//  Copyright (c) 2015 Planet IX. All rights reserved.
//

#pragma once

#include "../LabRenderDemoApp.h"

#ifdef _WIN32
#include <GL/glew.h>
#endif

#include <LabMath/LabMath.h>
#include <LabRender/PassRenderer.h>

#include <GLFW/glfw3.h>

#ifdef _WIN32
# undef APIENTRY
# define GLFW_EXPOSE_NATIVE_WIN32
# define GLFW_EXPOSE_NATIVE_WGL
# include <GLFW/glfw3native.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <atomic>
#include <mutex>
#include <string>

#include "imgui.h"


namespace lab {




    class ImGuiIntegrationx : public SupplementalGuiHandler
    {
        ImFont* default_font{};
        ImFont* regular_font{};
        ImFont* mono_font{};
        ImFont* mono_small_font{};
        ImFont* icon_font{};

        bool show_demo_window = true;
        bool show_another_window = false;
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

        // GLFW data
        GLFWwindow*  g_Window = NULL;
        double       g_Time = 0.0f;
        bool         g_MouseJustPressed[3] = { false, false, false };
        GLFWcursor*  g_MouseCursors[ImGuiMouseCursor_COUNT] = { 0 };

        // OpenGL3 data
        GLuint       g_FontTexture = 0;
        int          g_ShaderHandle = 0, g_VertHandle = 0, g_FragHandle = 0;
        int          g_AttribLocationTex = 0, g_AttribLocationProjMtx = 0;
        int          g_AttribLocationPosition = 0, g_AttribLocationUV = 0, g_AttribLocationColor = 0;
        unsigned int g_VboHandle = 0, g_ElementsHandle = 0;


        void    ImGui_ImplGlfwGL3_InvalidateDeviceObjects();
        static void ImGui_ImplGlfwGL3_SetClipboardText(void* user_data, const char* text);
        static const char* ImGui_ImplGlfwGL3_GetClipboardText(void* user_data);

        virtual void mouseButtonCallback(int /*button*/, int /*action*/, int /*mods*/) override;
        virtual void keyCallback(int key, int scancode, int action, int mods) override;

        static void ImGui_ImplGlfw_ScrollCallback(GLFWwindow*, double xoffset, double yoffset);
        static void ImGui_ImplGlfw_KeyCallback(GLFWwindow*, int key, int, int action, int mods);
        static void ImGui_ImplGlfw_CharCallback(GLFWwindow*, unsigned int c);
        bool ImGui_ImplGlfwGL3_CreateFontsTexture();
        bool ImGui_ImplGlfwGL3_CreateDeviceObjects();
        void ImGui_ImplGlfwGL3_RenderDrawData(ImDrawData* draw_data);
        void ImGui_ImplGlfwGL3_NewFrame();

    public:
        ImGuiIntegrationx(GLFWwindow* window);
        ~ImGuiIntegrationx();

        virtual void ui() override;
    };


    class ImGuiIntegration : public SupplementalGuiHandler
    {
    public:
        ImGuiIntegration(GLFWwindow* window);
        ~ImGuiIntegration();

        virtual void ui() override;
    };



}
