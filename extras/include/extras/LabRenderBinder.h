
#pragma once

#include <LabRender/MathTypes.h>


#if defined(_MSC_VER) && defined(LABRENDER_BINDER_DLL)
# ifdef BUILDING_LabRenderBinder
#  define LRB_CAPI extern "C" __declspec(dllexport)
#  define LRB_API __declspec(dllexport)
#  define LRB_CLASS __declspec(dllexport)
# else
#  define LRB_CAPI extern "C" __declspec(dllimport)
#  define LRB_API __declspec(dllimport)
#  define LRB_CLASS __declspec(dllimport)
# endif
#else
# define LRB_API
# define LRB_CAPI
# define LRB_CLASS
#endif

// object handles
struct lab_render_mgr;
struct lab_renderer    { private: friend struct lab_render_mgr; int handle{}; };
struct lab_render_lock { private: friend struct lab_render_mgr; int handle{}; lab_renderer lr; };
struct lab_model       { private: friend struct lab_render_mgr; int handle{}; };
struct lab_mesh        { private: friend struct lab_render_mgr; int handle{}; };
struct lab_texture     { private: friend struct lab_render_mgr; int handle{}; int platform_handle; };

// renderer
LRB_API lab_renderer lab_renderer_create();
LRB_API void lab_renderer_configure(const lab_renderer&, const char* path);
LRB_API void lab_render_release(const lab_renderer&);

// rendering
LRB_API lab_render_lock lab_acquire_render_lock(const lab_renderer&, double renderTime, float mousex, float mousey);
LRB_API void lab_release_render_lock(const lab_render_lock&);
LRB_API void lab_render_start(const lab_render_lock&, const lab::v2i& frame_buffer_size);
LRB_API void lab_render_end(const lab_render_lock&);
LRB_API void lab_render_context_set_camera(const lab_render_lock& l, const lab::m44f& model, const lab::m44f& view, const lab::m44f& proj);

LRB_API void lab_render_draw_mesh(const lab_render_lock& l, const lab_mesh&, const lab::m44f& mesh_transform);
LRB_API void lab_render_draw_model(const lab_render_lock& l, const lab_model&, const lab::m44f& mesh_transform);

// buffers and textures
LRB_API lab_texture lab_render_framebuffer_channel(const lab_renderer& lr, const char* framebuffer_name, const char* framebuffer_channel);
LRB_API void lab_texture_save(const lab_texture&, const char* path);
// GL interface... because the GL backing has to interoperate with other systems.
LRB_API int lab_texture_GL_handle(const lab_texture&);

// meshes
LRB_API lab_model lab_render_model_load(const char* path);
LRB_API lab_mesh lab_render_mesh_full_screen_quad();
LRB_API lab_mesh lab_render_mesh_plane(float xHalf, float yHalf, int xSegments, int ySegments);
LRB_API lab_mesh lab_render_mesh_cylinder(float radiusTop, float radiusBottom, float height,
                                          int radialSegments, int heightSegments, bool openEnded);
LRB_API lab_mesh lab_render_mesh_sphere(float radius_, int widthSegments_, int heightSegments_,
                                        float phiStart_, float phiLength_, float thetaStart_, float thetaLength_, bool uvw);
LRB_API lab_mesh lab_render_mesh_icosahedron(float radius);
LRB_API lab_mesh lab_render_mesh_box(float xHalf, float yHalf, float zHalf,
                                     int xSegments_, int ySegments_, int zSegments_, bool insideOut, bool uvw);
LRB_API lab_mesh lab_render_mesh_sky_box(int xSegments_, int ySegments_, int zSegments_);
LRB_API lab_mesh lab_render_mesh_frustum(float x1Half, float y1Half, float x2Half, float y2Half, float znear, float zfar);
LRB_API lab_mesh lab_render_mesh_frustum(float znear, float zfar, float yfov, float aspect);

