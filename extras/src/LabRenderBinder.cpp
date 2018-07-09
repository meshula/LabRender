
#define BUILDING_LABRENDER_MODELLOADER

#include "LabREnderBinder/LabRenderBinder.h"
#include "LabRender/PassRenderer.h"
#include "LabRender/Renderer.h"
#include "LabRenderModelLoader/modelLoader.h"
#include "LabRender/UtilityModel.h"
#include <LabMath/LabMath.h>

#include <vector>

using namespace std;
using namespace lab;

struct lab_render_mgr
{
    vector<unique_ptr<PassRenderer>> renderers;
    vector<unique_ptr<Renderer::RenderLock>> renderlocks;
    vector<shared_ptr<Model>> models;
    vector<shared_ptr<ModelBase>> meshes;
    vector<shared_ptr<Texture>> textures;
    lab_renderer empty;
    v2i frame_buffer_size;

    lab::DrawList drawList;

    lab_render_mgr()
    {
        renderers.emplace_back(nullptr);
        renderlocks.emplace_back(nullptr);
    }

    lab_renderer lab_renderer_create()
    {
        renderers.emplace_back(new PassRenderer());
        lab_renderer r;
        r.handle = (int)(renderers.size() - 1);
        return r;
    }

    void renderer_configure(const lab_renderer& lr, const char* path)
    {
        if (!renderers[lr.handle])
            return;

        renderers[lr.handle]->configure(path);
    }

    void lab_render_release(const lab_renderer& lr)
    {
        renderers[lr.handle].reset();
    }

    lab_render_lock acquire_render_lock(const lab_renderer& lr, double renderTime, v2f mousePosition)
    {
        lab_render_lock l;
        if (!renderers[lr.handle])
            return l;

        renderlocks.emplace_back(new Renderer::RenderLock(renderers[lr.handle].get(), renderTime, mousePosition));
        l.handle = (int)(renderlocks.size() - 1);
        l.lr = lr;
        return l;
    }

    void release_render_lock(const lab_render_lock& l)
    {
        if (!renderlocks[l.handle])
            return;

        if ((size_t)l.handle == renderlocks.size() - 1)
            renderlocks.pop_back();
        else
            renderlocks[l.handle].reset();
    }

    void render_start(const lab_render_lock& l, const v2i& frame_buffer_size_)
    {
        frame_buffer_size = frame_buffer_size_;

        if (!renderlocks[l.handle])
            return;

        Renderer::RenderLock* rl = renderlocks[l.handle].get();
        if (!rl->valid() || rl->renderInProgress())
            return;

        //const double globalTime = 0;

        rl->setRenderInProgress(true);
        //rl.context.renderTime = globalTime;

        drawList.deferredMeshes.clear();
    }

    void render_end(const lab_render_lock& l)
    {
        if (!renderlocks[l.handle])
            return;

        Renderer::RenderLock* rl = renderlocks[l.handle].get();
        if (!rl->valid() || !rl->renderInProgress())
            return;

        if (!renderers[l.lr.handle])
            return;

        renderers[l.lr.handle]->render(*rl, frame_buffer_size, drawList);

        // custom bit
        //glfwSwapBuffers(window);
        // end

        rl->setRenderInProgress(false);
    }

    void render_context_set_camera(const lab_render_lock& l, const m44f& model, const m44f& view, const m44f& proj)
    {
        drawList.modl = model;
        drawList.view = view;
        drawList.proj = proj;
    }

    void render_draw_mesh(const lab_render_lock& l, const lab_mesh& m, const m44f& tx)
    {
        if (!renderlocks[l.handle])
            return;

        Renderer::RenderLock* rl = renderlocks[l.handle].get();
        if (!rl->valid() || !rl->renderInProgress())
            return;

        if (!meshes[m.handle])
            return;

        drawList.deferredMeshes.push_back({ tx, meshes[m.handle] });
    }

    void render_draw_model(const lab_render_lock& l, const lab_model& m, const m44f& tx)
    {
        if (!renderlocks[l.handle])
            return;

        Renderer::RenderLock* rl = renderlocks[l.handle].get();
        if (!rl->valid() || !rl->renderInProgress())
            return;

        auto model = models[m.handle].get();
        if (!model)
            return;

        for (auto& part : model->parts())
            drawList.deferredMeshes.push_back({ tx, part });
    }

    lab_model model_load(const char* path)
    {
        shared_ptr<lab::Model> model = lab::loadMesh(path);
        models.push_back(model);
        lab_model r;
        r.handle = (int)(models.size() - 1);
        return r;
    }

    lab_texture framebuffer_channel(const lab_renderer& lr, const char* name, const char* channel)
    {
        if (!renderers[lr.handle])
            return {};

        auto fb = renderers[lr.handle]->framebuffer(name);
        int index = 0;
        for (auto& i : fb->baseNames)
        {
            if (i == channel)
            {
                break;
            }
            ++index;
        }

        if (index == fb->baseNames.size())
            return {};

        auto tx = fb->textures[index];
        int idx = 0;
        for (auto& i : textures)
        {
            if (i == tx)
            {
                lab_texture r;
                r.handle = idx;
                r.platform_handle = tx->id;
                return r;
            }
        }

        textures.push_back(tx);
        lab_texture r;
        r.handle = (int)(textures.size() - 1);
        r.platform_handle = tx->id;
        return r;
    }

    int texture_GL_handle(const lab_texture& t)
    {
        return t.platform_handle;
    }

    void texture_save(const lab_texture& t, const char* path)
    {
        if (t.handle >= textures.size())
            return;
        if (!textures[t.handle])
            return;

        textures[t.handle]->save(path);
    }


    lab_mesh mesh_cylinder(float radiusTop, float radiusBottom, float height,
        int radialSegments, int heightSegments, bool openEnded)
    {
        shared_ptr<lab::UtilityModel> model = make_shared<lab::UtilityModel>();
        model->createCylinder(radiusTop, radiusBottom, height, radialSegments, heightSegments, openEnded);
        meshes.push_back(model);
        lab_mesh r;
        r.handle = (int)(meshes.size() - 1);
        return r;
    }
};

static lab_render_mgr mgr;
lab_renderer    lab_renderer_create()                                            
                    { return mgr.lab_renderer_create(); }
void            lab_renderer_configure(const lab_renderer& lr, const char* path) 
                    { mgr.renderer_configure(lr, path); }
void            lab_render_release(const lab_renderer& lr)                       
                    { mgr.lab_render_release(lr); }
lab_render_lock lab_acquire_render_lock(const lab_renderer& lr, double renderTime, float mx, float my) 
                    { return mgr.acquire_render_lock(lr, renderTime, v2f(mx, my)); }
void            lab_release_render_lock(const lab_render_lock& l) 
                    { mgr.release_render_lock(l); }
void            lab_render_start(const lab_render_lock& l, const v2i& frame_buffer_size) 
                    { mgr.render_start(l, frame_buffer_size); }
void            lab_render_end(const lab_render_lock& l) 
                    { mgr.render_end(l); }
void            lab_render_context_set_camera(const lab_render_lock& l, const m44f& model, const m44f& view, const m44f& proj) 
                    { mgr.render_context_set_camera(l, model, view, proj); }
void            lab_render_draw_mesh(const lab_render_lock& l, const lab_mesh& m, const m44f& t)   
                    { mgr.render_draw_mesh(l, m, t); }
void            lab_render_draw_model(const lab_render_lock& l, const lab_model& m, const m44f& t)   
                    { mgr.render_draw_model(l, m, t); }
lab_model       lab_render_model_load(const char* path) 
                    { return mgr.model_load(path); }
lab_texture     lab_render_framebuffer_channel(const lab_renderer& lr, const char* name, const char* channel) 
                    { return mgr.framebuffer_channel(lr, name, channel); }
int             lab_texture_GL_handle(const lab_texture& t) 
                    { return mgr.texture_GL_handle(t); }
void            lab_texture_save(const lab_texture& t, const char* path) 
                    { mgr.texture_save(t, path); }
lab_mesh        lab_render_mesh_cylinder(float radiusTop, float radiusBottom, float height,
                                         int radialSegments, int heightSegments, bool openEnded) 
                    { return mgr.mesh_cylinder(radiusTop, radiusBottom, height, radialSegments, heightSegments, openEnded); }
