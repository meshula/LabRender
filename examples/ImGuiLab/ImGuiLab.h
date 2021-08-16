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

    class ImGuiIntegration : public SupplementalGuiHandler
    {
    public:
        ImGuiIntegration(GLFWwindow* window);
        ~ImGuiIntegration();

        virtual void ui(int window_width, int widnow_height) override;
        virtual void render() override;
    };

}
