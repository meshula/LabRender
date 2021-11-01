#ifndef LABIMGUI_GL_GLFW_H
#define LABIMGUI_GL_GLFW_H

struct GLFWwindow;

extern "C"
typedef struct {
    bool main_clicked;
    bool main_active;
    bool main_hovered;
} lab_FullScreenMouseState;

extern "C"
void lab_imgui_init_window(const char* window_name, GLFWwindow * window);
extern "C"
void lab_imgui_shutdown();
extern "C"
void lab_imgui_new_docking_frame();
extern "C"
lab_FullScreenMouseState lab_imgui_begin_fullscreen_docking(float window_width, float window_height);
extern "C"
void lab_imgui_end_fullscreen_docking();
extern "C"
void lab_imgui_render(const char* window_name);

#endif
