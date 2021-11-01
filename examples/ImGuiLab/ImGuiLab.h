//
//  ImGuiLab.h
//  LabRenderExamples
//
//  Created by Nick Porcino on 6/5/15.
//  Copyright (c) 2015 Planet IX. All rights reserved.
//

#pragma once

#include "../LabRenderDemoApp.h"


struct GLFWwindow;

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
