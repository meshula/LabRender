//
//  LabRenderDemoApp.h
//  LabRenderExamples
//
//  Created by Nick Porcino on 6/5/15.
//  Copyright (c) 2015 Planet IX. All rights reserved.
//

#pragma once

#ifdef _WIN32
#include <GL/glew.h>
#endif

#include <LabRender/MathTypes.h>
#include <LabRender/PassRenderer.h>

#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <atomic>
#include <mutex>
#include <string>

namespace lab {


    class AppBase {
    protected:
        v2f _mousePos;

    public:

        /**
         AppBase implementation
         */

        virtual ~AppBase() {}

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

        virtual void renderStart(PassRenderer::RenderLock & rl, double globalTime, v2i offset, v2i size) {
            if (!rl.valid() || rl.renderInProgress())
                return;

            rl.setRenderInProgress(true);
            rl.context.renderTime = globalTime;
            doRenderStart(rl, offset, size);
        }
        virtual void doRenderStart(PassRenderer::RenderLock &, v2i offset, v2i size) = 0;

        virtual void renderEnd(PassRenderer::RenderLock& rl) {
            if (!rl.valid() || !rl.renderInProgress())
                return;

            doRenderEnd(rl);
            rl.setRenderInProgress(false);
        }
        virtual void doRenderEnd(PassRenderer::RenderLock &) = 0;

        virtual double renderTime() const = 0;

        // to be called at the end of the update loop to allow system event processing etc.
        virtual void frameEnd() {}
    };


    class GLFWAppBase : public AppBase {

        GLFWwindow* window;
        bool mouseIsDown;
        bool rightMouseIsDown;

    public:

        GLFWAppBase()
        : mouseIsDown(false)
        {
            // window creation
            glfwSetErrorCallback(error_callback);

            if (!glfwInit())
                exit(EXIT_FAILURE);

#           ifdef __APPLE__
                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
                glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
                glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#           endif

            window = glfwCreateWindow(1280, 960, "LabRender", NULL, NULL);
            if (!window) {
                glfwTerminate();
                exit(EXIT_FAILURE);
            }
            glfwMakeContextCurrent(window);
            glfwSwapInterval(1);

            // callback initialization
            glfwSetWindowUserPointer(window, this);
            glfwSetKeyCallback(window, key_callback);
            glfwSetWindowSizeCallback(window, resizeCallback);
            glfwSetMouseButtonCallback(window, mouseButtonCallback);
            glfwSetCursorPosCallback(window, mousePosCallback);

			// start GLEW extension handler
			checkError(ErrorPolicy::onErrorThrow,
				TestConditions::exhaustive, "main loop start");

#           ifdef _WIN32
			    glewExperimental = GL_TRUE;
			    glewInit(); // create GLEW after the context has been created
#           endif

			// get version info
			const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
			const GLubyte* version = glGetString(GL_VERSION); // version as a string
			printf("Renderer: %s\n", renderer);
			printf("OpenGL version supported %s\n", version);

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

            AppBase* appPtr = (AppBase*) glfwGetWindowUserPointer(window);
            if (!appPtr)
                return;

            if (action == GLFW_PRESS && appPtr) {
                appPtr->keyPress(key);
            }
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

        virtual bool isFinished() override {
            return !!glfwWindowShouldClose(window);
        }

        virtual void frameEnd() override {
            glfwPollEvents();
        }

    protected:
        virtual void doRenderStart(PassRenderer::RenderLock & rl, v2i offset, v2i size) override {
            if (!rl.valid())
                return;

            glViewport(offset.x, offset.y, size.x, size.y);
            glClearColor(0.5f,0,0,1);
            glClear(GL_COLOR_BUFFER_BIT);
        }

        virtual void doRenderEnd(PassRenderer::RenderLock & rl) override {
            if (!rl.valid())
                return;

            glfwSwapBuffers(window);
        }



    };

}
