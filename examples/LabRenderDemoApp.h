//
//  LabRenderDemoApp.h
//  LabRenderExamples
//
//  Created by Nick Porcino on 6/5/15.
//  Copyright (c) 2015 Planet IX. All rights reserved.
//

#pragma once

#define HAVE_DEARIMGUI

#include <LabRender/PassRenderer.h>
#include <LabRender/Utils.h>
#include <LabMath/LabMath.h>
#include <LabCamera/LabCamera.h>

#include <stdlib.h>
#include <stdio.h>
#include <atomic>
#include <mutex>
#include <string>

namespace lab {

class SupplementalGuiHandler
{
public:
    virtual ~SupplementalGuiHandler() = default;
    virtual void mouseButtonCallback(int /*button*/, int /*action*/, int /*mods*/) {}
    virtual void keyCallback(int key, int scancode, int action, int mods) {}
    virtual void ui(int window_width, int window_height) {}
    virtual void render() {}
    bool quit = false;
    bool is_main_clicked = false;
    bool is_main_active = false;
    bool is_main_hovered = false;
};

class LabRenderAppScene {
public:
    LabRenderAppScene() {
        lc_camera_set_defaults(&camera);
    }
    virtual ~LabRenderAppScene() = default;

    virtual void build() = 0;
    virtual void update() {}
    virtual void render(lab::Render::Renderer::RenderLock& rl, v2i fbSize)
    {
        if (dr)
            dr->render(rl, fbSize, drawList);
    }

    lc_camera camera;
    lab::Render::DrawList drawList;
    std::shared_ptr<lab::Render::PassRenderer> dr;
};

class AppBackend
{
protected:
    v2f _mousePos;
    SupplementalGuiHandler* _supplemental = nullptr;
    
public:
    virtual ~AppBackend() {}
    
    virtual v2i frameBufferDimensions() const { return {0,0}; }
    
    virtual void keyPress(int key) {}
    virtual void mouseDown(v2f windowSize, v2f pos) {}
    virtual void mouseUp(v2f windowSize, v2f pos) {}
    virtual void rightMouseDown(v2f windowSize, v2f pos) {}
    virtual void rightMouseUp(v2f windowSize, v2f pos) {}
    virtual void mouseDrag(v2f windowSize, v2f pos) {}
    virtual void mouseMove(v2f windowSize, v2f pos) {}
    
    v2f mousePosition() const { return _mousePos; }
    
    virtual bool isFinished() {
        return false;
    }
    
    virtual void renderStart(Render::PassRenderer::RenderLock & rl, double globalTime, v2i offset, v2i size)
    {
        if (!rl.valid() || rl.renderInProgress())
            return;
        
        rl.setRenderInProgress(true);
        rl.context.renderTime = globalTime;
        doRenderStart(rl, offset, size);
    }
    virtual void doRenderStart(Render::PassRenderer::RenderLock &, v2i offset, v2i size) = 0;
    
    virtual void renderEnd(Render::PassRenderer::RenderLock& rl)
    {
        if (!rl.valid() || !rl.renderInProgress())
            return;
        
        doRenderEnd(rl);
        rl.setRenderInProgress(false);
    }
    virtual void doRenderEnd(Render::PassRenderer::RenderLock &) = 0;
    
    virtual double renderTime() const = 0;
    
    virtual void frameBegin() {}
    virtual void frameEnd() {}
    
    virtual void ui() {}
    virtual bool mainActive() const {
        if (!_supplemental)
            return false;
        return _supplemental->is_main_clicked || _supplemental->is_main_active || _supplemental->is_main_hovered;
    }
};


#ifdef HAVE_GLFW

class GLFWBackend : public AppBackend {
    bool mouseIsDown = false;
    bool rightMouseIsDown = false;

protected:
    GLFWwindow* window = nullptr;

public:
    GLFWAppBase(char const*const appTitle)
    : mouseIsDown(false)
    {
        labrender_init();

        // window creation
        glfwSetErrorCallback(error_callback);

        if (!glfwInit())
            throw std::runtime_error("Could not initialize glfw");

#           ifdef __APPLE__
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#           endif

        window = glfwCreateWindow(1280, 960, appTitle, NULL, NULL);
        if (!window) {
            glfwTerminate();
            throw std::runtime_error("Could not create a window");
        }
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

        // callback initialization
        glfwSetWindowUserPointer(window, this);
        glfwSetKeyCallback(window, key_callback);
        glfwSetWindowSizeCallback(window, resizeCallback);
        glfwSetMouseButtonCallback(window, mouseButtonCallback);
        glfwSetCursorPosCallback(window, mousePosCallback);

        checkError(ErrorPolicy::onErrorThrow,
            TestConditions::exhaustive, "main loop start");

        labrender_init();

        checkError(ErrorPolicy::onErrorThrow,
            TestConditions::exhaustive, "main loop start");
    }

    virtual ~GLFWAppBase() {
        if (window) {
            glfwDestroyWindow(window);
            glfwTerminate();
            window = nullptr;
        }
    }

    virtual double renderTime() const override {
        return glfwGetTime();
    }

    static void error_callback(int error, const char* description)
    {
        fputs(description, stderr);
    }


    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GL_TRUE);

        auto appPtr = (GLFWAppBase*) glfwGetWindowUserPointer(window);
        if (!appPtr)
            return;

        if (action == GLFW_PRESS && appPtr) {
            appPtr->keyPress(key);
        }

        if (appPtr->_supplemental)
            appPtr->_supplemental->keyCallback(key, scancode, action, mods);
    }


    static void resizeCallback(GLFWwindow* window, int w, int h)
    {
        printf("%d %d\n", w, h);
    }


    // Handle mouse button events - updates the Mouse structure
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int modifierKeys)
    {
        GLFWAppBase* appPtr = (GLFWAppBase*) glfwGetWindowUserPointer(window);
        if (!appPtr)
            return;

        int width, height;
        glfwGetWindowSize(window, &width, &height);

        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (action == GLFW_PRESS) {
                appPtr->mouseIsDown = true;
                appPtr->mouseDown(V2F(float(width), float(height)), appPtr->_mousePos);
            }
            else {
                appPtr->mouseIsDown = false;
                appPtr->mouseUp(V2F(float(width), float(height)), appPtr->_mousePos);
            }
        }
        else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            if (action == GLFW_PRESS) {
                appPtr->rightMouseIsDown = true;
                appPtr->rightMouseDown(V2F(float(width), float(height)), appPtr->_mousePos);
            }
            else {
                appPtr->rightMouseIsDown = false;
                appPtr->rightMouseUp(V2F(float(width), float(height)), appPtr->_mousePos);
            }
        }

        if (appPtr->_supplemental)
            appPtr->_supplemental->mouseButtonCallback(button, action, modifierKeys);
    }

    // Handle mouse motion - updates the Mouse structure
    static void mousePosCallback(GLFWwindow* window, double x, double y)
    {
        GLFWAppBase* appPtr = (GLFWAppBase*) glfwGetWindowUserPointer(window);
        if (!appPtr)
            return;

        int width, height;
        glfwGetWindowSize(window, &width, &height);

        appPtr->_mousePos = V2F(float(x), float(y));
        if (appPtr->mouseIsDown) {
            appPtr->mouseDrag(V2F(float(width), float(height)), appPtr->_mousePos);
        }
        else {
            appPtr->mouseMove(V2F(float(width), float(height)), appPtr->_mousePos);
        }
    }

    virtual v2i frameBufferDimensions() const override {
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        return {w,h};
    }

    virtual bool isFinished() override
    {
        return glfwWindowShouldClose(window) != 0;
    }

    virtual void frameBegin() override
    {
        glfwPollEvents();
    }

    virtual void frameEnd() override
    {
        glfwSwapBuffers(window);
    }
    
    void ui() override {
        if (_supplemental) {
            int width, height;
            glfwGetWindowSize(window, &width, &height);
            _supplemental->ui(width, height);
        }
    }

protected:
    virtual void doRenderStart(Render::PassRenderer::RenderLock & rl, v2i offset, v2i size) override
    {
        if (!rl.valid())
            return;

      //  glViewport(offset.x, offset.y, size.x, size.y);
        //glClearColor(0.5f,0,0,1);
        //glClear(GL_COLOR_BUFFER_BIT);
    }

    virtual void doRenderEnd(Render::PassRenderer::RenderLock & rl) override
    {
        if (!rl.valid())
            return;

        if (_supplemental) {
            _supplemental->render();
        }
    }
};

#endif


    class LabRenderExampleApp {
        AppBackend* _backend;
        
    public:
        LabRenderAppScene* scene = nullptr;
        lc_interaction* camera_controller = nullptr;
        lc_i_Mode cameraRigMode = lc_i_ModeTurnTableOrbit;
        lc_i_Phase interactionPhase = lc_i_PhaseNone;

        v2f initialMousePosition;
        v2f previousMousePosition;
        v2f previousWindowSize;

        explicit LabRenderExampleApp(const char* name, AppBackend* backend)
        : _backend(backend)
        , previousMousePosition(V2F(0, 0))
        {
            camera_controller = lc_i_create_interactive_controller();

            const char* env = getenv("ASSET_ROOT");
            if (env)
                lab::addPathVariable("{ASSET_ROOT}", env);
            else
                lab::addPathVariable("{ASSET_ROOT}", ASSET_ROOT);
        }

        ~LabRenderExampleApp()
        {
            lc_i_free_interactive_controller(camera_controller);
        }

        void createScene()
        {
            if (scene)
                scene->build();
        }

        void update() {
            if (_backend) {
                _backend->ui();
                
                if (!_backend->mainActive()) {
                    // if the main viewport is not active, stop camera interaction
                    if (interactionPhase != lc_i_PhaseNone)
                        interactionPhase = lc_i_PhaseFinish;
                }
            }

            if (scene)
                scene->update();

            if (interactionPhase != lc_i_PhaseNone) {
                InteractionToken it = lc_i_begin_interaction(camera_controller, 
                                                             { previousWindowSize.x, previousWindowSize.y });
                lc_i_ttl_interaction(
                                        camera_controller,
                                        &scene->camera,
                                        it,
                                        interactionPhase,
                                        cameraRigMode,
                                        lc_v2f{ previousMousePosition.x, previousMousePosition.y },
                                        lc_radians{ 0 },
                                        1.f / 60.f);
                
                lc_i_end_interaction(camera_controller, it);
                
                if (interactionPhase == lc_i_PhaseStart)
                    interactionPhase = lc_i_PhaseContinue;
                if (interactionPhase == lc_i_PhaseFinish)
                    interactionPhase = lc_i_PhaseNone;
            }
        }

        void render()
        {
            if (!scene)
                return;

            // run the immediate gui. @TODO: add a ui() method
            // now render
            lab::checkError(lab::ErrorPolicy::onErrorThrow,
                lab::TestConditions::exhaustive, "main loop start");

            v2i fbSize = _backend->frameBufferDimensions();

            lc_m44f modl = lc_mount_rotation_transform(&scene->camera.mount);
            scene->drawList.modl = *reinterpret_cast<lab::m44f*>(&modl);
            lc_m44f view = lc_mount_gl_view_transform(&scene->camera.mount);
            scene->drawList.view = *reinterpret_cast<lab::m44f*>(&view);
            lc_m44f proj = lc_camera_perspective(&scene->camera, float(fbSize.x) / float(fbSize.y));
            scene->drawList.proj = *reinterpret_cast<lab::m44f*>(&proj);
            lab::Render::PassRenderer::RenderLock rl(scene->dr.get(), _backend->renderTime(), _backend->mousePosition());
            v2i fbOffset = V2I(0, 0);
            _backend->renderStart(rl, _backend->renderTime(), fbOffset, fbSize);

            scene->render(rl, fbSize);

            if (_backend)
                _backend->renderEnd(rl);

            lab::checkError(lab::ErrorPolicy::onErrorThrow,
                lab::TestConditions::exhaustive, "main loop end");

            static bool saving = false;
            if (saving)
            {
                auto fb = scene->dr->framebuffer("gbuffer");
                if (fb && fb->textures.size())
                    for (int i = 0; i < fb->baseNames.size(); ++i)
                    {
                        std::string name = "C:\\Projects\\foo_" + fb->baseNames[i] + ".png";
                        fb->textures[i]->save(name.c_str());
                    }
            }
        }

#ifdef HAVE_GLFW
        virtual void keyPress(int key) override {
            switch (key) {
            case GLFW_KEY_C: cameraRigMode = lc_i_ModeCrane; break;
            case GLFW_KEY_D: cameraRigMode = lc_i_ModeDolly; break;
            case GLFW_KEY_T:
            case GLFW_KEY_O: cameraRigMode = lc_i_ModeTurnTableOrbit; break;
            case GLFW_KEY_P: cameraRigMode = lc_i_ModePanTilt; break;
            case GLFW_KEY_A: cameraRigMode = lc_i_ModeArcball; break;
            }
        }

        virtual void mouseDown(v2f windowSize, v2f pos) override {
            bool accept = true;
            if (_supplemental) 
                accept = _supplemental->is_main_clicked || _supplemental->is_main_active || _supplemental->is_main_hovered;
            if (accept) {
                previousMousePosition = pos;
                initialMousePosition = pos;
                previousWindowSize = windowSize;
                interactionPhase = lc_i_PhaseStart;
            }
        }

        virtual void mouseDrag(v2f windowSize, v2f pos) override {
            bool accept = true;
            if (_supplemental)
                accept = _supplemental->is_main_clicked || _supplemental->is_main_active || _supplemental->is_main_hovered;
            if (accept) {
                previousMousePosition = pos;
                previousWindowSize = { windowSize.x, windowSize.y };
                interactionPhase = lc_i_PhaseContinue;
            }
        }

        virtual void mouseUp(v2f windowSize, v2f pos) override {
            bool accept = true;
            if (_supplemental)
                accept = _supplemental->is_main_clicked || _supplemental->is_main_active || _supplemental->is_main_hovered;
            if (accept) {
                previousMousePosition = pos;
                previousWindowSize = { windowSize.x, windowSize.y };
                interactionPhase = lc_i_PhaseFinish;
            }
        }

        virtual void mouseMove(v2f windowSize, v2f pos) override {
            bool accept = true;
            if (_supplemental)
                accept = _supplemental->is_main_clicked || _supplemental->is_main_active || _supplemental->is_main_hovered;
            if (accept) {
                previousMousePosition = pos;
                previousWindowSize = { windowSize.x, windowSize.y };
            }
        }
        #endif
    };



} // lab
