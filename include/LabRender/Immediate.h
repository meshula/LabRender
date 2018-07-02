
#pragma once

#include <LabRender/Export.h>
#include <LabRender/MathTypes.h>
#include <LabRender/PodVector.h>

#include <memory>

namespace lab {

typedef int      ImmTextureId;
typedef uint64_t ImmSpriteId;

LR_API ImmSpriteId SpriteId(std::shared_ptr<uint8_t> rgb, int w, int h);

enum ImmDrawCornerFlags_
{
    ImmDrawCornerFlags_TopLeft =  1 << 0,
    ImmDrawCornerFlags_TopRight = 1 << 1,
    ImmDrawCornerFlags_BotLeft =  1 << 2,
    ImmDrawCornerFlags_BotRight = 1 << 3,
    ImmDrawCornerFlags_Top =   ImmDrawCornerFlags_TopLeft  | ImmDrawCornerFlags_TopRight,
    ImmDrawCornerFlags_Bot =   ImmDrawCornerFlags_BotLeft  | ImmDrawCornerFlags_BotRight,
    ImmDrawCornerFlags_Left =  ImmDrawCornerFlags_TopLeft  | ImmDrawCornerFlags_BotLeft,
    ImmDrawCornerFlags_Right = ImmDrawCornerFlags_TopRight | ImmDrawCornerFlags_BotRight,
    ImmDrawCornerFlags_All = 0xF
};

/*

    Current use pattern is when immediate drawing is to begin, create an ImmRenderContext,
    draw stuff, call render, then let ImmRenderContext fall out of scope.

    Profiling may dictate another pattern is preferable.

 */

class ImmRenderContext
{
    struct Detail;
    std::unique_ptr<Detail> _detail;

public:
    LR_API ImmRenderContext();
    LR_API ~ImmRenderContext();

    // Render-level scissoring, not used for culling
    LR_API void push_clip_rect(v2f clip_rect_min, v2f clip_rect_max, bool intersect_with_current_clip_rect = false);
    LR_API void push_clip_rect_fullscreen();
    LR_API void pop_clip_rect();
    LR_API v2f  clip_rect_min() const;
    LR_API v2f  clip_rect_max() const;

    LR_API void push_texture_id(lab::ImmTextureId texture_id);
    LR_API void pop_texture_id();

    // Primitives
    LR_API void line(const v2f& a, const v2f& b, uint32_t col, float thickness = 1.0f);

    LR_API void rectangle(                  const lab::v2f& p0, const lab::v2f& p1, uint32_t c,
                                            float rounding = 0.0f, int rounding_corners_flags = lab::ImmDrawCornerFlags_All,
                                            float thickness = 1.0f);
    LR_API void rectangle_filled(           const lab::v2f& p0, const lab::v2f& p1, uint32_t c,
                                            float rounding = 0.0f, int rounding_corners_flags = lab::ImmDrawCornerFlags_All,
                                            float thickness = 1.0f);
    LR_API void rectangle_filled_multicolor(const lab::v2f& p0, const lab::v2f& p1,
                                            uint32_t a, uint32_t b, uint32_t c, uint32_t d);

    LR_API void quad(       const v2f& a, const v2f& b, const v2f& c, const v2f& d, uint32_t col, float thickness = 1.0f);
    LR_API void quad_filled(const v2f& a, const v2f& b, const v2f& c, const v2f& d, uint32_t col);

    LR_API void triangle(       const v2f& a, const v2f& b, const v2f& c, uint32_t col, float thickness = 1.0f);
    LR_API void triangle_filled(const v2f& a, const v2f& b, const v2f& c, uint32_t col);

    LR_API void circle(const v2f& centre, float radius, uint32_t col, int num_segments = 12, float thickness = 1.0f);
    LR_API void circle_filled(const v2f& centre, float radius, uint32_t col, int num_segments = 12);

    //   LR_API void  AddText(const v2f& pos, uint32_t col, const char* text_begin, const char* text_end = NULL);
    //   LR_API void  AddText(const ImmFont* font, float font_size, 
    //                        const v2f& pos, uint32_t col, const char* text_begin, const char* text_end = NULL, 
    //                        float wrap_width = 0.0f, const v4f* cpu_fine_clip_rect = NULL);

    LR_API void image(        lab::ImmTextureId user_texture_id,
                              const v2f& a, const v2f& b,
                              const v2f& uv_a = v2f(0, 0), const v2f& uv_b = v2f(1, 1), uint32_t col = 0xFFFFFFFF);
    LR_API void image_quad(   lab::ImmTextureId user_texture_id,
                              const v2f& a, const v2f& b, const v2f& c, const v2f& d,
                              const v2f& uv_a = v2f(0, 0), const v2f& uv_b = v2f(1, 0),
                              const v2f& uv_c = v2f(1, 1), const v2f& uv_d = v2f(0, 1), uint32_t col = 0xFFFFFFFF);
    LR_API void image_rounded(lab::ImmTextureId user_texture_id,
                              const v2f& a, const v2f& b,
                              const v2f& uv_a, const v2f& uv_b, uint32_t col,
                              float rounding, int rounding_corners = lab::ImmDrawCornerFlags_All);

    LR_API void poly_line(         const v2f* points, const int num_points, uint32_t col, bool closed, float thickness);
    LR_API void poly_convex_filled(const v2f* points, const int num_points, uint32_t col);

    LR_API void bezier(const v2f& pos0, const v2f& cp0, const v2f& cp1, const v2f& pos1, 
                       uint32_t col, float thickness, int num_segments = 0);

    // Stateful path API, add points then finish with path_fill() or path_stroke()
    LR_API void path_clear();
    LR_API void path_line_to(        const v2f& pos);
    LR_API void path_arc_to(         const v2f& centre, float radius, float a_min, float a_max, int num_segments = 10);
    LR_API void path_arc_to_coarse(  const v2f& centre, float radius, int a_min_of_12, int a_max_of_12);
    LR_API void path_bezier_curve_to(const v2f& p1, const v2f& p2, const v2f& p3, int num_segments = 0);
    LR_API void path_rect(           const v2f& rect_min, const v2f& rect_max,
                                     float rounding = 0.0f, int rounding_corners_flags = lab::ImmDrawCornerFlags_All);

    LR_API void path_fill_convex(uint32_t col);
    LR_API void path_stroke(     uint32_t col, bool closed, float thickness = 1.0f);

    LR_API void sprite(lab::ImmSpriteId id, int depth, float scale, float theta_radians, float x, float y);

    // call render after everything is done to emit draw calls.
    LR_API void render(int w, int h);
};

} //lab
