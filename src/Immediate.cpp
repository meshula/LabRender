
#include "LabRender/Immediate.h"
#include "gl4.h"

#define STBRP_ASSERT(x)    IMM_ASSERT(x)
#ifndef IMGUI_DISABLE_STB_RECT_PACK_IMPLEMENTATION
#define STBRP_STATIC
#define STB_RECT_PACK_IMPLEMENTATION
#endif
#include "stb_rect_pack.h"

#include "tinyheaders/tinyspritebatch.h"
#include "tinyheaders/tinypng.h"

#include <algorithm>
#include <atomic>
#include <memory>
#include <unordered_map>
#include <vector>



/*
This file is liberally adapted from ocornut's imgui.
*/

// Define assertion handler.
#ifndef IMM_ASSERT
#include <assert.h>
#define IMM_ASSERT(_EXPR)    assert(_EXPR)
#endif

// All of this is straight out of ImGui. Because it works, and is fast.

namespace lab {

#define IMM_COL32_R_SHIFT    16
#define IMM_COL32_G_SHIFT    8
#define IMM_COL32_B_SHIFT    0
#define IMM_COL32_A_SHIFT    24
#define IMM_COL32_A_MASK     0xFF000000
#define IMM_COL32(R,G,B,A)    (((uint32_t)(A)<<IMM_COL32_A_SHIFT) | ((uint32_t)(B)<<IMM_COL32_B_SHIFT) | ((uint32_t)(G)<<IMM_COL32_G_SHIFT) | ((uint32_t)(R)<<IMM_COL32_R_SHIFT))

#define IMM_OFFSETOF(_TYPE,_ELM)     ((size_t)&(((_TYPE*)0)->_ELM))
#define IMM_ARRAYSIZE(_ARR)          ((int)(sizeof(_ARR)/sizeof(*_ARR)))

inline float ImmInvLength(const v2f& lhs, float fail_value)
{
    float d = lhs.x*lhs.x + lhs.y*lhs.y;
    if (d > 0.0f) return 1.0f / sqrtf(d);
    return fail_value;
}
inline float ImmLengthSqr(const v2f& lhs)
{
    return lhs.x * lhs.x + lhs.y * lhs.y;
}
inline float ImmDot(const v2f& a, const v2f& b)
{
    return a.x * b.x + a.y * b.y;
}
inline v2f ImmMul(const v2f& lhs, const v2f& rhs) { return v2f(lhs.x * rhs.x, lhs.y * rhs.y); }

inline int    ImmLerp(int a, int b, float t) { return (int)(a + (b - a) * t); }
inline float  ImmClamp(float v, float mn, float mx) { return (v < mn) ? mn : (v > mx) ? mx : v; }
inline v2f    ImmClamp(const v2f& f, const v2f& mn, v2f mx) { return v2f(ImmClamp(f.x, mn.x, mx.x), ImmClamp(f.y, mn.y, mx.y)); }

static inline int       ImmUpperPowerOfTwo(int v) { v--; v |= v >> 1; v |= v >> 2; v |= v >> 4; v |= v >> 8; v |= v >> 16; v++; return v; }




struct GLBitsAndBobs
{
    unsigned int vboHandle{ 0 };
    unsigned int elementsHandle{ 0 };
    int vertHandle{ 0 };
    int fragHandle{ 0 };
    int shaderHandle{ 0 };
    int attribLocationTex{ 0 };
    int attribLocationProjMtx{ 0 };
    int attribLocationPosition{ 0 };
    int attribLocationUV{ 0 };
    int attribLocationColor{ 0 };
    unsigned int defaultTexture{ 0 };

    GLBitsAndBobs() {}
    ~GLBitsAndBobs()
    {
        if (vboHandle) glDeleteBuffers(1, &vboHandle);
        if (elementsHandle) glDeleteBuffers(1, &elementsHandle);
        if (shaderHandle && vertHandle) glDetachShader(shaderHandle, vertHandle);
        if (vertHandle) glDeleteShader(vertHandle);
        if (shaderHandle && fragHandle) glDetachShader(shaderHandle, fragHandle);
        if (fragHandle) glDeleteShader(fragHandle);
        if (shaderHandle) glDeleteProgram(shaderHandle);
        if (defaultTexture) glDeleteTextures(1, &defaultTexture);
    }

    void init()
    {
        const GLchar *vertex_shader =
            "#version 150\n"
            "uniform mat4 ProjMtx;\n"
            "in vec2 Position;\n"
            "in vec2 UV;\n"
            "in vec4 Color;\n"
            "out vec2 Frag_UV;\n"
            "out vec4 Frag_Color;\n"
            "void main()\n"
            "{\n"
            "	Frag_UV = UV;\n"
            "	Frag_Color = Color;\n"
            "	gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
            "}\n";

        const GLchar* fragment_shader =
            "#version 150\n"
            "uniform sampler2D Texture;\n"
            "in vec2 Frag_UV;\n"
            "in vec4 Frag_Color;\n"
            "out vec4 Out_Color;\n"
            "void main()\n"
            "{\n"
            "	Out_Color = Frag_Color * texture( Texture, Frag_UV.st);\n"
            "}\n";

        shaderHandle = glCreateProgram();
        vertHandle = glCreateShader(GL_VERTEX_SHADER);
        fragHandle = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(vertHandle, 1, &vertex_shader, 0);
        glShaderSource(fragHandle, 1, &fragment_shader, 0);
        glCompileShader(vertHandle);
        glCompileShader(fragHandle);
        glAttachShader(shaderHandle, vertHandle);
        glAttachShader(shaderHandle, fragHandle);
        glLinkProgram(shaderHandle);

        attribLocationTex = glGetUniformLocation(shaderHandle, "Texture");
        attribLocationProjMtx = glGetUniformLocation(shaderHandle, "ProjMtx");
        attribLocationPosition = glGetAttribLocation(shaderHandle, "Position");
        attribLocationUV = glGetAttribLocation(shaderHandle, "UV");
        if (attribLocationUV < 0)
            printf("No UV attrib bound in shader\n");
        attribLocationColor = glGetAttribLocation(shaderHandle, "Color");
        if (attribLocationColor < 0)
            printf("No Color attrib bound in shader\n");

        glGenBuffers(1, &vboHandle);
        glGenBuffers(1, &elementsHandle);

        // font texture
        // Build texture atlas
        uint32_t pixels[16] = { 0xffffffff,0xffffffff,0xffffffff,0xffffffff,
                                0xffffffff,0xffffffff,0xffffffff,0xffffffff,
                                0xffffffff,0xffffffff,0xffffffff,0xffffffff,
                                0xffffffff,0xffffffff,0xffffffff,0xffffffff };
        int width = 4, height = 4;

        // Upload texture to graphics system
        GLint last_texture;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
        glGenTextures(1, &defaultTexture);
        glBindTexture(GL_TEXTURE_2D, defaultTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

        // Restore state
        glBindTexture(GL_TEXTURE_2D, last_texture);
    }
};
static GLBitsAndBobs gl;













struct ImmDrawCmd;
struct ImmDrawList;
typedef uint32_t ImmDrawIdx;


typedef uint8_t ImmDrawListFlags;

enum ImDmrawListFlags_
{
    ImmDrawListFlags_AntiAliasedLines = 1 << 0,
    ImmDrawListFlags_AntiAliasedFill = 1 << 1
};



struct ImmDrawListSharedData
{
    v2f      TexUvWhitePixel;            // UV of white pixel in the atlas
    float    CurveTessellationTol{ 0.f };
    v4f      ClipRectFullscreen{ -8192.0f, -8192.0f, +8192.0f, +8192.0f }; // Value for PushClipRectFullscreen()

                                                                           // Const data
                                                                           // FIXME: Bake rounded corners fill/borders in atlas
    v2f CircleVtx12[12];

    LR_API ImmDrawListSharedData(); // creates CircleVtx data
};


//-----------------------------------------------------------------------------
// Draw List
// Hold a series of drawing commands. The user provides a renderer for ImmDrawData which essentially contains an array of ImmDrawList.
//-----------------------------------------------------------------------------

// Draw callbacks for advanced uses.
// NB- You most likely do NOT need to use draw callbacks just to create your own widget or customized UI rendering (you can poke into the draw list for that)
// Draw callback may be useful for example, A) Change your GPU render state, B) render a complex 3D scene inside a UI element (without an intermediate texture/render target), etc.
// The expected behavior from your rendering function is 'if (cmd.UserCallback != NULL) cmd.UserCallback(parent_list, cmd); else RenderTriangles()'
typedef void(*ImmDrawCallback)(const ImmDrawList* parent_list, const ImmDrawCmd* cmd);

// Typically, 1 command = 1 GPU draw call (unless command is a callback)
struct ImmDrawCmd
{
    unsigned int    ElemCount = 0;              // Number of indices (multiple of 3) to be rendered as triangles. Vertices are stored in the callee ImmDrawList's vtx_buffer[] array, indices in idx_buffer[].
    v4f          ClipRect{ 0,0,0,0 };               // Clipping rectangle (x1, y1, x2, y2)
    ImmTextureId     TextureId{ 0 };              // User-provided texture ID.
    ImmDrawCallback  UserCallback{ nullptr };           // If != NULL, call the function instead of rendering the vertices. clip_rect and texture_id will be set normally.
    void*           UserCallbackData{ nullptr };       // The draw callback code can access this.
};

// Vertex layout
struct ImmDrawVert
{
    v2f  pos;
    v2f  uv;
    uint32_t   col;
};

// Draw channels are used by the Columns API to "split" the render list into different channels while building, so items of each column can be batched together.
// You can also use them to simulate drawing layers and submit primitives in a different order than how they will be rendered.
struct ImmDrawChannel
{
    PodVector<ImmDrawCmd>     CmdBuffer;
    PodVector<ImmDrawIdx>     IdxBuffer;
};


// Draw command list
// This is the low-level list of polygons that ImGui functions are filling. At the end of the frame, all command lists are passed to your ImGuiIO::RenderDrawListFn function for rendering.
// Each ImGui window contains its own ImmDrawList. You can use ImGui::GetWindowDrawList() to access the current window draw list and draw custom primitives.
// You can interleave normal ImGui:: calls and adding primitives to the current draw list.
// All positions are generally in pixel coordinates (top-left at (0,0), bottom-right at io.DisplaySize), however you are totally free to apply whatever transformation matrix to want to the data (if you apply such transformation you'll want to apply it to ClipRect as well)
// Important: Primitives are always added to the list and not culled (culling is done at higher-level by ImGui:: functions), if you use this API a lot consider coarse culling your drawn objects.
struct ImmDrawList
{
    // This is what you have to render
    PodVector<ImmDrawCmd>     CmdBuffer;          // Draw commands. Typically 1 command = 1 GPU draw call, unless the command is a callback.
    PodVector<ImmDrawIdx>     IdxBuffer;          // Index buffer. Each command consume ImmDrawCmd::ElemCount of those
    PodVector<ImmDrawVert>    VtxBuffer;          // Vertex buffer.

                                                  // [Internal, used while building lists]
    ImmDrawListFlags         Flags;              // Flags, you may poke into these to adjust anti-aliasing settings per-primitive.
    const ImmDrawListSharedData* _Data;          // Pointer to shared draw data (you can use ImGui::GetDrawListSharedData() to get the one from current ImGui context)
    const char*             _OwnerName;         // Pointer to owner window's name for debugging
    unsigned int            _VtxCurrentIdx;     // [Internal] == VtxBuffer.Size
    ImmDrawVert*             _VtxWritePtr;       // [Internal] point within VtxBuffer.Data after each add command (to avoid using the PodVector<> operators too much)
    ImmDrawIdx*              _IdxWritePtr;       // [Internal] point within IdxBuffer.Data after each add command (to avoid using the PodVector<> operators too much)
    PodVector<v4f>        _ClipRectStack;     // [Internal]
    PodVector<ImmTextureId>   _TextureIdStack;    // [Internal]
    PodVector<v2f>        _Path;              // [Internal] current path building
    int                     _ChannelsCurrent;   // [Internal] current channel number (0)
    int                     _ChannelsCount;     // [Internal] number of active channels (1+)
    PodVector<ImmDrawChannel> _Channels;          // [Internal] draw channels for columns API (not resized down so _ChannelsCount may be smaller than _Channels.Size)



                                                  // If you want to create ImmDrawList instances, pass them ImGui::GetDrawListSharedData() or create and use your own ImmDrawListSharedData (so you can use ImmDrawList without ImGui)
    ImmDrawList(const ImmDrawListSharedData* shared_data) { _Data = shared_data; _OwnerName = NULL; Clear(); }
    ~ImmDrawList() { ClearFreeMemory(); }
    LR_API void  PushClipRect(v2f clip_rect_min, v2f clip_rect_max, bool intersect_with_current_clip_rect = false);  // Render-level scissoring. This is passed down to your render function but not used for CPU-side coarse clipping.
    LR_API void  PushClipRectFullScreen();
    LR_API void  PopClipRect();
    LR_API void  PushTextureID(ImmTextureId texture_id);
    LR_API void  PopTextureID();
    inline v2f   GetClipRectMin() const { const v4f& cr = _ClipRectStack.back(); return v2f(cr.x, cr.y); }
    inline v2f   GetClipRectMax() const { const v4f& cr = _ClipRectStack.back(); return v2f(cr.z, cr.w); }

    // Primitives
    LR_API void  AddLine(const v2f& a, const v2f& b, uint32_t col, float thickness = 1.0f);
    LR_API void  AddRect(const v2f& a, const v2f& b, uint32_t col, float rounding = 0.0f, int rounding_corners_flags = ImmDrawCornerFlags_All, float thickness = 1.0f);   // a: upper-left, b: lower-right, rounding_corners_flags: 4-bits corresponding to which corner to round
    LR_API void  AddRectFilled(const v2f& a, const v2f& b, uint32_t col, float rounding = 0.0f, int rounding_corners_flags = ImmDrawCornerFlags_All);                     // a: upper-left, b: lower-right
    LR_API void  AddRectFilledMultiColor(const v2f& a, const v2f& b, uint32_t col_upr_left, uint32_t col_upr_right, uint32_t col_bot_right, uint32_t col_bot_left);
    LR_API void  AddQuad(const v2f& a, const v2f& b, const v2f& c, const v2f& d, uint32_t col, float thickness = 1.0f);
    LR_API void  AddQuadFilled(const v2f& a, const v2f& b, const v2f& c, const v2f& d, uint32_t col);
    LR_API void  AddTriangle(const v2f& a, const v2f& b, const v2f& c, uint32_t col, float thickness = 1.0f);
    LR_API void  AddTriangleFilled(const v2f& a, const v2f& b, const v2f& c, uint32_t col);
    LR_API void  AddCircle(const v2f& centre, float radius, uint32_t col, int num_segments = 12, float thickness = 1.0f);
    LR_API void  AddCircleFilled(const v2f& centre, float radius, uint32_t col, int num_segments = 12);
    //   LR_API void  AddText(const v2f& pos, uint32_t col, const char* text_begin, const char* text_end = NULL);
    //   LR_API void  AddText(const ImmFont* font, float font_size, const v2f& pos, uint32_t col, const char* text_begin, const char* text_end = NULL, float wrap_width = 0.0f, const v4f* cpu_fine_clip_rect = NULL);
    LR_API void  AddImage(ImmTextureId user_texture_id, const v2f& a, const v2f& b, const v2f& uv_a = v2f(0, 0), const v2f& uv_b = v2f(1, 1), uint32_t col = 0xFFFFFFFF);
    LR_API void  AddImageQuad(ImmTextureId user_texture_id, const v2f& a, const v2f& b, const v2f& c, const v2f& d, const v2f& uv_a = v2f(0, 0), const v2f& uv_b = v2f(1, 0), const v2f& uv_c = v2f(1, 1), const v2f& uv_d = v2f(0, 1), uint32_t col = 0xFFFFFFFF);
    LR_API void  AddImageRounded(ImmTextureId user_texture_id, const v2f& a, const v2f& b, const v2f& uv_a, const v2f& uv_b, uint32_t col, float rounding, int rounding_corners = ImmDrawCornerFlags_All);
    LR_API void  AddPolyline(const v2f* points, const int num_points, uint32_t col, bool closed, float thickness);
    LR_API void  AddConvexPolyFilled(const v2f* points, const int num_points, uint32_t col);
    LR_API void  AddBezierCurve(const v2f& pos0, const v2f& cp0, const v2f& cp1, const v2f& pos1, uint32_t col, float thickness, int num_segments = 0);

    // Stateful path API, add points then finish with PathFill() or PathStroke()
    inline    void  PathClear() { _Path.resize(0); }
    inline    void  PathLineTo(const v2f& pos) { _Path.push_back(pos); }
    inline    void  PathLineToMergeDuplicate(const v2f& pos) { if (_Path.Size == 0 || memcmp(&_Path[_Path.Size - 1], &pos, 8) != 0) _Path.push_back(pos); }
    inline    void  PathFillConvex(uint32_t col) { AddConvexPolyFilled(_Path.Data, _Path.Size, col); PathClear(); }
    inline    void  PathStroke(uint32_t col, bool closed, float thickness = 1.0f) { AddPolyline(_Path.Data, _Path.Size, col, closed, thickness); PathClear(); }
    LR_API void  PathArcTo(const v2f& centre, float radius, float a_min, float a_max, int num_segments = 10);
    LR_API void  PathArcToFast(const v2f& centre, float radius, int a_min_of_12, int a_max_of_12);                                // Use precomputed angles for a 12 steps circle
    LR_API void  PathBezierCurveTo(const v2f& p1, const v2f& p2, const v2f& p3, int num_segments = 0);
    LR_API void  PathRect(const v2f& rect_min, const v2f& rect_max, float rounding = 0.0f, int rounding_corners_flags = ImmDrawCornerFlags_All);

    // Channels
    // - Use to simulate layers. By switching channels to can render out-of-order (e.g. submit foreground primitives before background primitives)
    // - Use to minimize draw calls (e.g. if going back-and-forth between multiple non-overlapping clipping rectangles, prefer to append into separate channels then merge at the end)
    LR_API void  ChannelsSplit(int channels_count);
    LR_API void  ChannelsMerge();
    LR_API void  ChannelsSetCurrent(int channel_index);

    // Advanced
    LR_API void  AddCallback(ImmDrawCallback callback, void* callback_data);  // Your rendering function must check for 'UserCallback' in ImmDrawCmd and call the function instead of rendering triangles.
    LR_API void  AddDrawCmd();                                               // This is useful if you need to forcefully create a new draw call (to allow for dependent rendering / blending). Otherwise primitives are merged into the same draw-call as much as possible

                                                                             // Internal helpers
                                                                             // NB: all primitives needs to be reserved via PrimReserve() beforehand!
    LR_API void  Clear();
    LR_API void  ClearFreeMemory();
    LR_API void  PrimReserve(int idx_count, int vtx_count);
    LR_API void  PrimRect(const v2f& a, const v2f& b, uint32_t col);      // Axis aligned rectangle (composed of two triangles)
    LR_API void  PrimRectUV(const v2f& a, const v2f& b, const v2f& uv_a, const v2f& uv_b, uint32_t col);
    LR_API void  PrimQuadUV(const v2f& a, const v2f& b, const v2f& c, const v2f& d, const v2f& uv_a, const v2f& uv_b, const v2f& uv_c, const v2f& uv_d, uint32_t col);
    inline    void  PrimWriteVtx(const v2f& pos, const v2f& uv, uint32_t col) { _VtxWritePtr->pos = pos; _VtxWritePtr->uv = uv; _VtxWritePtr->col = col; _VtxWritePtr++; _VtxCurrentIdx++; }
    inline    void  PrimWriteIdx(ImmDrawIdx idx) { *_IdxWritePtr = idx; _IdxWritePtr++; }
    inline    void  PrimVtx(const v2f& pos, const v2f& uv, uint32_t col) { PrimWriteIdx((ImmDrawIdx)_VtxCurrentIdx); PrimWriteVtx(pos, uv, col); }
    LR_API void  UpdateClipRect();
    LR_API void  UpdateTextureID();
};

// All draw data to render an ImGui frame
struct ImmDrawData
{
    bool            Valid;                  // Only valid after Render() is called and before the next NewFrame() is called.
    PodVector<ImmDrawList*> CmdLists;
    int             TotalVtxCount;          // For convenience, sum of all cmd_lists vtx_buffer.Size
    int             TotalIdxCount;          // For convenience, sum of all cmd_lists idx_buffer.Size

                                            // Functions
    LR_API ImmDrawData() { Clear(); }
    LR_API void Clear() { CmdLists.clear();  Valid = false; TotalVtxCount = TotalIdxCount = 0; } // Draw lists are owned by the ImGuiContext and only pointed to here.
    LR_API void ScaleClipRects(const v2f& sc);  // Helper to scale the ClipRect field of each ImmDrawCmd. Use if your final output buffer is at a different scale than ImGui expects, or if there is a difference between your window resolution and framebuffer resolution.

    LR_API void Render(int fb_width, int fb_height);
};




//-----------------------------------------------------------------------------
// ImmDrawListData
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Shade functions
//-----------------------------------------------------------------------------

// Generic linear color gradient, write to RGB fields, leave A untouched.
void ShadeVertsLinearColorGradientKeepAlpha(ImmDrawVert* vert_start, ImmDrawVert* vert_end, v2f gradient_p0, v2f gradient_p1, uint32_t col0, uint32_t col1)
{
    v2f gradient_extent = gradient_p1 - gradient_p0;
    float gradient_inv_length2 = 1.0f / ImmLengthSqr(gradient_extent);
    for (ImmDrawVert* vert = vert_start; vert < vert_end; vert++)
    {
        float d = ImmDot(vert->pos - gradient_p0, gradient_extent);
        float t = ImmClamp(d * gradient_inv_length2, 0.0f, 1.0f);
        int r = ImmLerp((int)(col0 >> IMM_COL32_R_SHIFT) & 0xFF, (int)(col1 >> IMM_COL32_R_SHIFT) & 0xFF, t);
        int g = ImmLerp((int)(col0 >> IMM_COL32_G_SHIFT) & 0xFF, (int)(col1 >> IMM_COL32_G_SHIFT) & 0xFF, t);
        int b = ImmLerp((int)(col0 >> IMM_COL32_B_SHIFT) & 0xFF, (int)(col1 >> IMM_COL32_B_SHIFT) & 0xFF, t);
        vert->col = (r << IMM_COL32_R_SHIFT) | (g << IMM_COL32_G_SHIFT) | (b << IMM_COL32_B_SHIFT) | (vert->col & IMM_COL32_A_MASK);
    }
}


// Scan and shade backward from the end of given vertices. Assume vertices are text only (= vert_start..vert_end going left to right) so we can break as soon as we are out the gradient bounds.
void ShadeVertsLinearAlphaGradientForLeftToRightText(ImmDrawVert* vert_start, ImmDrawVert* vert_end, float gradient_p0_x, float gradient_p1_x)
{
    float gradient_extent_x = gradient_p1_x - gradient_p0_x;
    float gradient_inv_length2 = 1.0f / (gradient_extent_x * gradient_extent_x);
    int full_alpha_count = 0;
    for (ImmDrawVert* vert = vert_end - 1; vert >= vert_start; vert--)
    {
        float d = (vert->pos.x - gradient_p0_x) * (gradient_extent_x);
        float alpha_mul = 1.0f - ImmClamp(d * gradient_inv_length2, 0.0f, 1.0f);
        if (alpha_mul >= 1.0f && ++full_alpha_count > 2)
            return; // Early out
        int a = (int)(((vert->col >> IMM_COL32_A_SHIFT) & 0xFF) * alpha_mul);
        vert->col = (vert->col & ~IMM_COL32_A_MASK) | (a << IMM_COL32_A_SHIFT);
    }
}

// Distribute UV over (a, b) rectangle
void ShadeVertsLinearUV(ImmDrawVert* vert_start, ImmDrawVert* vert_end, const v2f& a, const v2f& b, const v2f& uv_a, const v2f& uv_b, bool clamp)
{
    const v2f size = b - a;
    const v2f uv_size = uv_b - uv_a;
    const v2f scale = v2f(
        size.x != 0.0f ? (uv_size.x / size.x) : 0.0f,
        size.y != 0.0f ? (uv_size.y / size.y) : 0.0f);

    if (clamp)
    {
        const v2f min = { std::min(uv_a.x, uv_b.x), std::min(uv_a.y, uv_b.y) };
        const v2f max = { std::max(uv_a.x, uv_b.x), std::max(uv_a.y, uv_b.y) };

        for (ImmDrawVert* vertex = vert_start; vertex < vert_end; ++vertex)
            vertex->uv = ImmClamp(uv_a + ImmMul(v2f(vertex->pos.x, vertex->pos.y) - a, scale), min, max);
    }
    else
    {
        for (ImmDrawVert* vertex = vert_start; vertex < vert_end; ++vertex)
            vertex->uv = uv_a + ImmMul(v2f(vertex->pos.x, vertex->pos.y) - a, scale);
    }
}

ImmDrawListSharedData::ImmDrawListSharedData()
{
    // Const data
    for (int i = 0; i < IMM_ARRAYSIZE(CircleVtx12); i++)
    {
        const float a = ((float)i * 2 * (float)M_PI) / (float)IMM_ARRAYSIZE(CircleVtx12);
        CircleVtx12[i] = v2f(cosf(a), sinf(a));
    }
}

//-----------------------------------------------------------------------------
// ImmDrawList
//-----------------------------------------------------------------------------

void ImmDrawList::Clear()
{
    CmdBuffer.resize(0);
    IdxBuffer.resize(0);
    VtxBuffer.resize(0);
    Flags = ImmDrawListFlags_AntiAliasedLines | ImmDrawListFlags_AntiAliasedFill;
    _VtxCurrentIdx = 0;
    _VtxWritePtr = NULL;
    _IdxWritePtr = NULL;
    _ClipRectStack.resize(0);
    _TextureIdStack.resize(0);
    _Path.resize(0);
    _ChannelsCurrent = 0;
    _ChannelsCount = 1;
    // NB: Do not clear channels so our allocations are re-used after the first frame.
}

void ImmDrawList::ClearFreeMemory()
{
    CmdBuffer.clear();
    IdxBuffer.clear();
    VtxBuffer.clear();
    _VtxCurrentIdx = 0;
    _VtxWritePtr = NULL;
    _IdxWritePtr = NULL;
    _ClipRectStack.clear();
    _TextureIdStack.clear();
    _Path.clear();
    _ChannelsCurrent = 0;
    _ChannelsCount = 1;
    for (int i = 0; i < _Channels.Size; i++)
    {
        if (i == 0) memset(&_Channels[0], 0, sizeof(_Channels[0]));  // channel 0 is a copy of CmdBuffer/IdxBuffer, don't destruct again
        _Channels[i].CmdBuffer.clear();
        _Channels[i].IdxBuffer.clear();
    }
    _Channels.clear();
}

// Using macros because C++ is a terrible language, we want guaranteed inline, no code in header, and no overhead in Debug builds
#define GetCurrentClipRect()    (_ClipRectStack.Size ? _ClipRectStack.Data[_ClipRectStack.Size-1]  : _Data->ClipRectFullscreen)
#define GetCurrentTextureId()   (_TextureIdStack.Size ? _TextureIdStack.Data[_TextureIdStack.Size-1] : NULL)

void ImmDrawList::AddDrawCmd()
{
    ImmDrawCmd draw_cmd;
    draw_cmd.ClipRect = GetCurrentClipRect();
    draw_cmd.TextureId = GetCurrentTextureId();

    IMM_ASSERT(draw_cmd.ClipRect.x <= draw_cmd.ClipRect.z && draw_cmd.ClipRect.y <= draw_cmd.ClipRect.w);
    CmdBuffer.push_back(draw_cmd);
}

void ImmDrawList::AddCallback(ImmDrawCallback callback, void* callback_data)
{
    ImmDrawCmd* current_cmd = CmdBuffer.Size ? &CmdBuffer.back() : NULL;
    if (!current_cmd || current_cmd->ElemCount != 0 || current_cmd->UserCallback != NULL)
    {
        AddDrawCmd();
        current_cmd = &CmdBuffer.back();
    }
    current_cmd->UserCallback = callback;
    current_cmd->UserCallbackData = callback_data;

    AddDrawCmd(); // Force a new command after us (see comment below)
}

// Our scheme may appears a bit unusual, basically we want the most-common calls AddLine AddRect etc. to not have to perform any check so we always have a command ready in the stack.
// The cost of figuring out if a new command has to be added or if we can merge is paid in those Update** functions only.
void ImmDrawList::UpdateClipRect()
{
    // If current command is used with different settings we need to add a new command
    const v4f curr_clip_rect = GetCurrentClipRect();
    ImmDrawCmd* curr_cmd = CmdBuffer.Size > 0 ? &CmdBuffer.Data[CmdBuffer.Size-1] : NULL;
    if (!curr_cmd || (curr_cmd->ElemCount != 0 && memcmp(&curr_cmd->ClipRect, &curr_clip_rect, sizeof(v4f)) != 0) || curr_cmd->UserCallback != NULL)
    {
        AddDrawCmd();
        return;
    }

    // Try to merge with previous command if it matches, else use current command
    ImmDrawCmd* prev_cmd = CmdBuffer.Size > 1 ? curr_cmd - 1 : NULL;
    if (curr_cmd->ElemCount == 0 && prev_cmd && memcmp(&prev_cmd->ClipRect, &curr_clip_rect, sizeof(v4f)) == 0 && prev_cmd->TextureId == GetCurrentTextureId() && prev_cmd->UserCallback == NULL)
        CmdBuffer.pop_back();
    else
        curr_cmd->ClipRect = curr_clip_rect;
}

void ImmDrawList::UpdateTextureID()
{
    // If current command is used with different settings we need to add a new command
    const ImmTextureId curr_texture_id = GetCurrentTextureId();
    ImmDrawCmd* curr_cmd = CmdBuffer.Size ? &CmdBuffer.back() : NULL;
    if (!curr_cmd || (curr_cmd->ElemCount != 0 && curr_cmd->TextureId != curr_texture_id) || curr_cmd->UserCallback != NULL)
    {
        AddDrawCmd();
        return;
    }

    // Try to merge with previous command if it matches, else use current command
    ImmDrawCmd* prev_cmd = CmdBuffer.Size > 1 ? curr_cmd - 1 : NULL;
    if (curr_cmd->ElemCount == 0 && prev_cmd && prev_cmd->TextureId == curr_texture_id && memcmp(&prev_cmd->ClipRect, &GetCurrentClipRect(), sizeof(v4f)) == 0 && prev_cmd->UserCallback == NULL)
        CmdBuffer.pop_back();
    else
        curr_cmd->TextureId = curr_texture_id;
}

#undef GetCurrentClipRect
#undef GetCurrentTextureId

// Render-level scissoring. This is passed down to your render function but not used for CPU-side coarse clipping. Prefer using higher-level ImGui::PushClipRect() to affect logic (hit-testing and widget culling)
void ImmDrawList::PushClipRect(v2f cr_min, v2f cr_max, bool intersect_with_current_clip_rect)
{
    v4f cr(cr_min.x, cr_min.y, cr_max.x, cr_max.y);
    if (intersect_with_current_clip_rect && _ClipRectStack.Size)
    {
        v4f current = _ClipRectStack.Data[_ClipRectStack.Size-1];
        if (cr.x < current.x) cr.x = current.x;
        if (cr.y < current.y) cr.y = current.y;
        if (cr.z > current.z) cr.z = current.z;
        if (cr.w > current.w) cr.w = current.w;
    }
    cr.z = std::max(cr.x, cr.z);
    cr.w = std::max(cr.y, cr.w);

    _ClipRectStack.push_back(cr);
    UpdateClipRect();
}

void ImmDrawList::PushClipRectFullScreen()
{
    PushClipRect(v2f(_Data->ClipRectFullscreen.x, _Data->ClipRectFullscreen.y), v2f(_Data->ClipRectFullscreen.z, _Data->ClipRectFullscreen.w));
}

void ImmDrawList::PopClipRect()
{
    IMM_ASSERT(_ClipRectStack.Size > 0);
    _ClipRectStack.pop_back();
    UpdateClipRect();
}

void ImmDrawList::PushTextureID(ImmTextureId texture_id)
{
    _TextureIdStack.push_back(texture_id);
    UpdateTextureID();
}

void ImmDrawList::PopTextureID()
{
    IMM_ASSERT(_TextureIdStack.Size > 0);
    _TextureIdStack.pop_back();
    UpdateTextureID();
}

void ImmDrawList::ChannelsSplit(int channels_count)
{
    IMM_ASSERT(_ChannelsCurrent == 0 && _ChannelsCount == 1);
    int old_channels_count = _Channels.Size;
    if (old_channels_count < channels_count)
        _Channels.resize(channels_count);
    _ChannelsCount = channels_count;

    // _Channels[] (24/32 bytes each) hold storage that we'll swap with this->_CmdBuffer/_IdxBuffer
    // The content of _Channels[0] at this point doesn't matter. We clear it to make state tidy in a debugger but we don't strictly need to.
    // When we switch to the next channel, we'll copy _CmdBuffer/_IdxBuffer into _Channels[0] and then _Channels[1] into _CmdBuffer/_IdxBuffer
    memset(&_Channels[0], 0, sizeof(ImmDrawChannel));
    for (int i = 1; i < channels_count; i++)
    {
        if (i >= old_channels_count)
        {
            new(&_Channels[i]) ImmDrawChannel();
        }
        else
        {
            _Channels[i].CmdBuffer.resize(0);
            _Channels[i].IdxBuffer.resize(0);
        }
        if (_Channels[i].CmdBuffer.Size == 0)
        {
            ImmDrawCmd draw_cmd;
            draw_cmd.ClipRect = _ClipRectStack.back();
            draw_cmd.TextureId = _TextureIdStack.back();
            _Channels[i].CmdBuffer.push_back(draw_cmd);
        }
    }
}

void ImmDrawList::ChannelsMerge()
{
    // Note that we never use or rely on channels.Size because it is merely a buffer that we never shrink back to 0 to keep all sub-buffers ready for use.
    if (_ChannelsCount <= 1)
        return;

    ChannelsSetCurrent(0);
    if (CmdBuffer.Size && CmdBuffer.back().ElemCount == 0)
        CmdBuffer.pop_back();

    int new_cmd_buffer_count = 0, new_idx_buffer_count = 0;
    for (int i = 1; i < _ChannelsCount; i++)
    {
        ImmDrawChannel& ch = _Channels[i];
        if (ch.CmdBuffer.Size && ch.CmdBuffer.back().ElemCount == 0)
            ch.CmdBuffer.pop_back();
        new_cmd_buffer_count += ch.CmdBuffer.Size;
        new_idx_buffer_count += ch.IdxBuffer.Size;
    }
    CmdBuffer.resize(CmdBuffer.Size + new_cmd_buffer_count);
    IdxBuffer.resize(IdxBuffer.Size + new_idx_buffer_count);

    ImmDrawCmd* cmd_write = CmdBuffer.Data + CmdBuffer.Size - new_cmd_buffer_count;
    _IdxWritePtr = IdxBuffer.Data + IdxBuffer.Size - new_idx_buffer_count;
    for (int i = 1; i < _ChannelsCount; i++)
    {
        ImmDrawChannel& ch = _Channels[i];
        if (int sz = ch.CmdBuffer.Size) { memcpy(cmd_write, ch.CmdBuffer.Data, sz * sizeof(ImmDrawCmd)); cmd_write += sz; }
        if (int sz = ch.IdxBuffer.Size) { memcpy(_IdxWritePtr, ch.IdxBuffer.Data, sz * sizeof(ImmDrawIdx)); _IdxWritePtr += sz; }
    }
    UpdateClipRect(); // We call this instead of AddDrawCmd(), so that empty channels won't produce an extra draw call.
    _ChannelsCount = 1;
}

void ImmDrawList::ChannelsSetCurrent(int idx)
{
    IMM_ASSERT(idx < _ChannelsCount);
    if (_ChannelsCurrent == idx) return;
    memcpy(&_Channels.Data[_ChannelsCurrent].CmdBuffer, &CmdBuffer, sizeof(CmdBuffer)); // copy 12 bytes, four times
    memcpy(&_Channels.Data[_ChannelsCurrent].IdxBuffer, &IdxBuffer, sizeof(IdxBuffer));
    _ChannelsCurrent = idx;
    memcpy(&CmdBuffer, &_Channels.Data[_ChannelsCurrent].CmdBuffer, sizeof(CmdBuffer));
    memcpy(&IdxBuffer, &_Channels.Data[_ChannelsCurrent].IdxBuffer, sizeof(IdxBuffer));
    _IdxWritePtr = IdxBuffer.Data + IdxBuffer.Size;
}

// NB: this can be called with negative count for removing primitives (as long as the result does not underflow)
void ImmDrawList::PrimReserve(int idx_count, int vtx_count)
{
    ImmDrawCmd& draw_cmd = CmdBuffer.Data[CmdBuffer.Size-1];
    draw_cmd.ElemCount += idx_count;

    int vtx_buffer_old_size = VtxBuffer.Size;
    VtxBuffer.resize(vtx_buffer_old_size + vtx_count);
    _VtxWritePtr = VtxBuffer.Data + vtx_buffer_old_size;

    int idx_buffer_old_size = IdxBuffer.Size;
    IdxBuffer.resize(idx_buffer_old_size + idx_count);
    _IdxWritePtr = IdxBuffer.Data + idx_buffer_old_size;
}
// Fully unrolled with inline call to keep our debug builds decently fast.
void ImmDrawList::PrimRect(const v2f& a, const v2f& c, uint32_t col)
{
    v2f b(c.x, a.y), d(a.x, c.y), uv(_Data->TexUvWhitePixel);
    ImmDrawIdx idx = (ImmDrawIdx)_VtxCurrentIdx;
    _IdxWritePtr[0] = idx; _IdxWritePtr[1] = (ImmDrawIdx)(idx+1); _IdxWritePtr[2] = (ImmDrawIdx)(idx+2);
    _IdxWritePtr[3] = idx; _IdxWritePtr[4] = (ImmDrawIdx)(idx+2); _IdxWritePtr[5] = (ImmDrawIdx)(idx+3);
    _VtxWritePtr[0].pos = a; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
    _VtxWritePtr[1].pos = b; _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col;
    _VtxWritePtr[2].pos = c; _VtxWritePtr[2].uv = uv; _VtxWritePtr[2].col = col;
    _VtxWritePtr[3].pos = d; _VtxWritePtr[3].uv = uv; _VtxWritePtr[3].col = col;
    _VtxWritePtr += 4;
    _VtxCurrentIdx += 4;
    _IdxWritePtr += 6;
}

void ImmDrawList::PrimRectUV(const v2f& a, const v2f& c, const v2f& uv_a, const v2f& uv_c, uint32_t col)
{
    v2f b(c.x, a.y), d(a.x, c.y), uv_b(uv_c.x, uv_a.y), uv_d(uv_a.x, uv_c.y);
    ImmDrawIdx idx = (ImmDrawIdx)_VtxCurrentIdx;
    _IdxWritePtr[0] = idx; _IdxWritePtr[1] = (ImmDrawIdx)(idx+1); _IdxWritePtr[2] = (ImmDrawIdx)(idx+2);
    _IdxWritePtr[3] = idx; _IdxWritePtr[4] = (ImmDrawIdx)(idx+2); _IdxWritePtr[5] = (ImmDrawIdx)(idx+3);
    _VtxWritePtr[0].pos = a; _VtxWritePtr[0].uv = uv_a; _VtxWritePtr[0].col = col;
    _VtxWritePtr[1].pos = b; _VtxWritePtr[1].uv = uv_b; _VtxWritePtr[1].col = col;
    _VtxWritePtr[2].pos = c; _VtxWritePtr[2].uv = uv_c; _VtxWritePtr[2].col = col;
    _VtxWritePtr[3].pos = d; _VtxWritePtr[3].uv = uv_d; _VtxWritePtr[3].col = col;
    _VtxWritePtr += 4;
    _VtxCurrentIdx += 4;
    _IdxWritePtr += 6;
}

void ImmDrawList::PrimQuadUV(const v2f& a, const v2f& b, const v2f& c, const v2f& d, const v2f& uv_a, const v2f& uv_b, const v2f& uv_c, const v2f& uv_d, uint32_t col)
{
    ImmDrawIdx idx = (ImmDrawIdx)_VtxCurrentIdx;
    _IdxWritePtr[0] = idx; _IdxWritePtr[1] = (ImmDrawIdx)(idx+1); _IdxWritePtr[2] = (ImmDrawIdx)(idx+2);
    _IdxWritePtr[3] = idx; _IdxWritePtr[4] = (ImmDrawIdx)(idx+2); _IdxWritePtr[5] = (ImmDrawIdx)(idx+3);
    _VtxWritePtr[0].pos = a; _VtxWritePtr[0].uv = uv_a; _VtxWritePtr[0].col = col;
    _VtxWritePtr[1].pos = b; _VtxWritePtr[1].uv = uv_b; _VtxWritePtr[1].col = col;
    _VtxWritePtr[2].pos = c; _VtxWritePtr[2].uv = uv_c; _VtxWritePtr[2].col = col;
    _VtxWritePtr[3].pos = d; _VtxWritePtr[3].uv = uv_d; _VtxWritePtr[3].col = col;
    _VtxWritePtr += 4;
    _VtxCurrentIdx += 4;
    _IdxWritePtr += 6;
}

// TODO: Thickness anti-aliased lines cap are missing their AA fringe.
void ImmDrawList::AddPolyline(const v2f* points, const int points_count, uint32_t col, bool closed, float thickness)
{
    if (points_count < 2)
        return;

    const v2f uv = _Data->TexUvWhitePixel;

    int count = points_count;
    if (!closed)
        count = points_count-1;

    const bool thick_line = thickness > 1.0f;
    if (Flags & ImmDrawListFlags_AntiAliasedLines)
    {
        // Anti-aliased stroke
        const float AA_SIZE = 1.0f;
        const uint32_t col_trans = col & ~IMM_COL32_A_MASK;

        const int idx_count = thick_line ? count*18 : count*12;
        const int vtx_count = thick_line ? points_count*4 : points_count*3;
        PrimReserve(idx_count, vtx_count);

        // Temporary buffer
        v2f* temp_normals = (v2f*)alloca(points_count * (thick_line ? 5 : 3) * sizeof(v2f));
        v2f* temp_points = temp_normals + points_count;

        for (int i1 = 0; i1 < count; i1++)
        {
            const int i2 = (i1+1) == points_count ? 0 : i1+1;
            v2f diff = points[i2] - points[i1];
            diff *= ImmInvLength(diff, 1.0f);
            temp_normals[i1].x = diff.y;
            temp_normals[i1].y = -diff.x;
        }
        if (!closed)
            temp_normals[points_count-1] = temp_normals[points_count-2];

        if (!thick_line)
        {
            if (!closed)
            {
                temp_points[0] = points[0] + temp_normals[0] * AA_SIZE;
                temp_points[1] = points[0] - temp_normals[0] * AA_SIZE;
                temp_points[(points_count-1)*2+0] = points[points_count-1] + temp_normals[points_count-1] * AA_SIZE;
                temp_points[(points_count-1)*2+1] = points[points_count-1] - temp_normals[points_count-1] * AA_SIZE;
            }

            // FIXME-OPT: Merge the different loops, possibly remove the temporary buffer.
            unsigned int idx1 = _VtxCurrentIdx;
            for (int i1 = 0; i1 < count; i1++)
            {
                const int i2 = (i1+1) == points_count ? 0 : i1+1;
                unsigned int idx2 = (i1+1) == points_count ? _VtxCurrentIdx : idx1+3;

                // Average normals
                v2f dm = (temp_normals[i1] + temp_normals[i2]) * 0.5f;
                float dmr2 = dm.x*dm.x + dm.y*dm.y;
                if (dmr2 > 0.000001f)
                {
                    float scale = 1.0f / dmr2;
                    if (scale > 100.0f) scale = 100.0f;
                    dm *= scale;
                }
                dm *= AA_SIZE;
                temp_points[i2*2+0] = points[i2] + dm;
                temp_points[i2*2+1] = points[i2] - dm;

                // Add indexes
                _IdxWritePtr[0] = (ImmDrawIdx)(idx2+0); _IdxWritePtr[1] = (ImmDrawIdx)(idx1+0); _IdxWritePtr[2] = (ImmDrawIdx)(idx1+2);
                _IdxWritePtr[3] = (ImmDrawIdx)(idx1+2); _IdxWritePtr[4] = (ImmDrawIdx)(idx2+2); _IdxWritePtr[5] = (ImmDrawIdx)(idx2+0);
                _IdxWritePtr[6] = (ImmDrawIdx)(idx2+1); _IdxWritePtr[7] = (ImmDrawIdx)(idx1+1); _IdxWritePtr[8] = (ImmDrawIdx)(idx1+0);
                _IdxWritePtr[9] = (ImmDrawIdx)(idx1+0); _IdxWritePtr[10]= (ImmDrawIdx)(idx2+0); _IdxWritePtr[11]= (ImmDrawIdx)(idx2+1);
                _IdxWritePtr += 12;

                idx1 = idx2;
            }

            // Add vertexes
            for (int i = 0; i < points_count; i++)
            {
                _VtxWritePtr[0].pos = points[i];          _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
                _VtxWritePtr[1].pos = temp_points[i*2+0]; _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col_trans;
                _VtxWritePtr[2].pos = temp_points[i*2+1]; _VtxWritePtr[2].uv = uv; _VtxWritePtr[2].col = col_trans;
                _VtxWritePtr += 3;
            }
        }
        else
        {
            const float half_inner_thickness = (thickness - AA_SIZE) * 0.5f;
            if (!closed)
            {
                temp_points[0] = points[0] + temp_normals[0] * (half_inner_thickness + AA_SIZE);
                temp_points[1] = points[0] + temp_normals[0] * (half_inner_thickness);
                temp_points[2] = points[0] - temp_normals[0] * (half_inner_thickness);
                temp_points[3] = points[0] - temp_normals[0] * (half_inner_thickness + AA_SIZE);
                temp_points[(points_count-1)*4+0] = points[points_count-1] + temp_normals[points_count-1] * (half_inner_thickness + AA_SIZE);
                temp_points[(points_count-1)*4+1] = points[points_count-1] + temp_normals[points_count-1] * (half_inner_thickness);
                temp_points[(points_count-1)*4+2] = points[points_count-1] - temp_normals[points_count-1] * (half_inner_thickness);
                temp_points[(points_count-1)*4+3] = points[points_count-1] - temp_normals[points_count-1] * (half_inner_thickness + AA_SIZE);
            }

            // FIXME-OPT: Merge the different loops, possibly remove the temporary buffer.
            unsigned int idx1 = _VtxCurrentIdx;
            for (int i1 = 0; i1 < count; i1++)
            {
                const int i2 = (i1+1) == points_count ? 0 : i1+1;
                unsigned int idx2 = (i1+1) == points_count ? _VtxCurrentIdx : idx1+4;

                // Average normals
                v2f dm = (temp_normals[i1] + temp_normals[i2]) * 0.5f;
                float dmr2 = dm.x*dm.x + dm.y*dm.y;
                if (dmr2 > 0.000001f)
                {
                    float scale = 1.0f / dmr2;
                    if (scale > 100.0f) scale = 100.0f;
                    dm *= scale;
                }
                v2f dm_out = dm * (half_inner_thickness + AA_SIZE);
                v2f dm_in = dm * half_inner_thickness;
                temp_points[i2*4+0] = points[i2] + dm_out;
                temp_points[i2*4+1] = points[i2] + dm_in;
                temp_points[i2*4+2] = points[i2] - dm_in;
                temp_points[i2*4+3] = points[i2] - dm_out;

                // Add indexes
                _IdxWritePtr[0]  = (ImmDrawIdx)(idx2+1); _IdxWritePtr[1]  = (ImmDrawIdx)(idx1+1); _IdxWritePtr[2]  = (ImmDrawIdx)(idx1+2);
                _IdxWritePtr[3]  = (ImmDrawIdx)(idx1+2); _IdxWritePtr[4]  = (ImmDrawIdx)(idx2+2); _IdxWritePtr[5]  = (ImmDrawIdx)(idx2+1);
                _IdxWritePtr[6]  = (ImmDrawIdx)(idx2+1); _IdxWritePtr[7]  = (ImmDrawIdx)(idx1+1); _IdxWritePtr[8]  = (ImmDrawIdx)(idx1+0);
                _IdxWritePtr[9]  = (ImmDrawIdx)(idx1+0); _IdxWritePtr[10] = (ImmDrawIdx)(idx2+0); _IdxWritePtr[11] = (ImmDrawIdx)(idx2+1);
                _IdxWritePtr[12] = (ImmDrawIdx)(idx2+2); _IdxWritePtr[13] = (ImmDrawIdx)(idx1+2); _IdxWritePtr[14] = (ImmDrawIdx)(idx1+3);
                _IdxWritePtr[15] = (ImmDrawIdx)(idx1+3); _IdxWritePtr[16] = (ImmDrawIdx)(idx2+3); _IdxWritePtr[17] = (ImmDrawIdx)(idx2+2);
                _IdxWritePtr += 18;

                idx1 = idx2;
            }

            // Add vertexes
            for (int i = 0; i < points_count; i++)
            {
                _VtxWritePtr[0].pos = temp_points[i*4+0]; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col_trans;
                _VtxWritePtr[1].pos = temp_points[i*4+1]; _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col;
                _VtxWritePtr[2].pos = temp_points[i*4+2]; _VtxWritePtr[2].uv = uv; _VtxWritePtr[2].col = col;
                _VtxWritePtr[3].pos = temp_points[i*4+3]; _VtxWritePtr[3].uv = uv; _VtxWritePtr[3].col = col_trans;
                _VtxWritePtr += 4;
            }
        }
        _VtxCurrentIdx += (ImmDrawIdx)vtx_count;
    }
    else
    {
        // Non Anti-aliased Stroke
        const int idx_count = count*6;
        const int vtx_count = count*4;      // FIXME-OPT: Not sharing edges
        PrimReserve(idx_count, vtx_count);

        for (int i1 = 0; i1 < count; i1++)
        {
            const int i2 = (i1+1) == points_count ? 0 : i1+1;
            const v2f& p1 = points[i1];
            const v2f& p2 = points[i2];
            v2f diff = p2 - p1;
            diff *= ImmInvLength(diff, 1.0f);

            const float dx = diff.x * (thickness * 0.5f);
            const float dy = diff.y * (thickness * 0.5f);
            _VtxWritePtr[0].pos.x = p1.x + dy; _VtxWritePtr[0].pos.y = p1.y - dx; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
            _VtxWritePtr[1].pos.x = p2.x + dy; _VtxWritePtr[1].pos.y = p2.y - dx; _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col;
            _VtxWritePtr[2].pos.x = p2.x - dy; _VtxWritePtr[2].pos.y = p2.y + dx; _VtxWritePtr[2].uv = uv; _VtxWritePtr[2].col = col;
            _VtxWritePtr[3].pos.x = p1.x - dy; _VtxWritePtr[3].pos.y = p1.y + dx; _VtxWritePtr[3].uv = uv; _VtxWritePtr[3].col = col;
            _VtxWritePtr += 4;

            _IdxWritePtr[0] = (ImmDrawIdx)(_VtxCurrentIdx); _IdxWritePtr[1] = (ImmDrawIdx)(_VtxCurrentIdx+1); _IdxWritePtr[2] = (ImmDrawIdx)(_VtxCurrentIdx+2);
            _IdxWritePtr[3] = (ImmDrawIdx)(_VtxCurrentIdx); _IdxWritePtr[4] = (ImmDrawIdx)(_VtxCurrentIdx+2); _IdxWritePtr[5] = (ImmDrawIdx)(_VtxCurrentIdx+3);
            _IdxWritePtr += 6;
            _VtxCurrentIdx += 4;
        }
    }
}

void ImmDrawList::AddConvexPolyFilled(const v2f* points, const int points_count, uint32_t col)
{
    const v2f uv = _Data->TexUvWhitePixel;

    if (Flags & ImmDrawListFlags_AntiAliasedFill)
    {
        // Anti-aliased Fill
        const float AA_SIZE = 1.0f;
        const uint32_t col_trans = col & ~IMM_COL32_A_MASK;
        const int idx_count = (points_count-2)*3 + points_count*6;
        const int vtx_count = (points_count*2);
        PrimReserve(idx_count, vtx_count);

        // Add indexes for fill
        unsigned int vtx_inner_idx = _VtxCurrentIdx;
        unsigned int vtx_outer_idx = _VtxCurrentIdx+1;
        for (int i = 2; i < points_count; i++)
        {
            _IdxWritePtr[0] = (ImmDrawIdx)(vtx_inner_idx); _IdxWritePtr[1] = (ImmDrawIdx)(vtx_inner_idx+((i-1)<<1)); _IdxWritePtr[2] = (ImmDrawIdx)(vtx_inner_idx+(i<<1));
            _IdxWritePtr += 3;
        }

        // Compute normals
        v2f* temp_normals = (v2f*)alloca(points_count * sizeof(v2f));
        for (int i0 = points_count-1, i1 = 0; i1 < points_count; i0 = i1++)
        {
            const v2f& p0 = points[i0];
            const v2f& p1 = points[i1];
            v2f diff = p1 - p0;
            diff *= ImmInvLength(diff, 1.0f);
            temp_normals[i0].x = diff.y;
            temp_normals[i0].y = -diff.x;
        }

        for (int i0 = points_count-1, i1 = 0; i1 < points_count; i0 = i1++)
        {
            // Average normals
            const v2f& n0 = temp_normals[i0];
            const v2f& n1 = temp_normals[i1];
            v2f dm = (n0 + n1) * 0.5f;
            float dmr2 = dm.x*dm.x + dm.y*dm.y;
            if (dmr2 > 0.000001f)
            {
                float scale = 1.0f / dmr2;
                if (scale > 100.0f) scale = 100.0f;
                dm *= scale;
            }
            dm *= AA_SIZE * 0.5f;

            // Add vertices
            _VtxWritePtr[0].pos = (points[i1] - dm); _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;        // Inner
            _VtxWritePtr[1].pos = (points[i1] + dm); _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col_trans;  // Outer
            _VtxWritePtr += 2;

            // Add indexes for fringes
            _IdxWritePtr[0] = (ImmDrawIdx)(vtx_inner_idx+(i1<<1)); _IdxWritePtr[1] = (ImmDrawIdx)(vtx_inner_idx+(i0<<1)); _IdxWritePtr[2] = (ImmDrawIdx)(vtx_outer_idx+(i0<<1));
            _IdxWritePtr[3] = (ImmDrawIdx)(vtx_outer_idx+(i0<<1)); _IdxWritePtr[4] = (ImmDrawIdx)(vtx_outer_idx+(i1<<1)); _IdxWritePtr[5] = (ImmDrawIdx)(vtx_inner_idx+(i1<<1));
            _IdxWritePtr += 6;
        }
        _VtxCurrentIdx += (ImmDrawIdx)vtx_count;
    }
    else
    {
        // Non Anti-aliased Fill
        const int idx_count = (points_count-2)*3;
        const int vtx_count = points_count;
        PrimReserve(idx_count, vtx_count);
        for (int i = 0; i < vtx_count; i++)
        {
            _VtxWritePtr[0].pos = points[i]; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
            _VtxWritePtr++;
        }
        for (int i = 2; i < points_count; i++)
        {
            _IdxWritePtr[0] = (ImmDrawIdx)(_VtxCurrentIdx); _IdxWritePtr[1] = (ImmDrawIdx)(_VtxCurrentIdx+i-1); _IdxWritePtr[2] = (ImmDrawIdx)(_VtxCurrentIdx+i);
            _IdxWritePtr += 3;
        }
        _VtxCurrentIdx += (ImmDrawIdx)vtx_count;
    }
}

void ImmDrawList::PathArcToFast(const v2f& centre, float radius, int a_min_of_12, int a_max_of_12)
{
    if (radius == 0.0f || a_min_of_12 > a_max_of_12)
    {
        _Path.push_back(centre);
        return;
    }
    _Path.reserve(_Path.Size + (a_max_of_12 - a_min_of_12 + 1));
    for (int a = a_min_of_12; a <= a_max_of_12; a++)
    {
        const v2f& c = _Data->CircleVtx12[a % IMM_ARRAYSIZE(_Data->CircleVtx12)];
        _Path.push_back(v2f(centre.x + c.x * radius, centre.y + c.y * radius));
    }
}

void ImmDrawList::PathArcTo(const v2f& centre, float radius, float a_min, float a_max, int num_segments)
{
    if (radius == 0.0f)
    {
        _Path.push_back(centre);
        return;
    }
    _Path.reserve(_Path.Size + (num_segments + 1));
    for (int i = 0; i <= num_segments; i++)
    {
        const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
        _Path.push_back(v2f(centre.x + cosf(a) * radius, centre.y + sinf(a) * radius));
    }
}

static void PathBezierToCasteljau(PodVector<v2f>* path, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float tess_tol, int level)
{
    float dx = x4 - x1;
    float dy = y4 - y1;
    float d2 = ((x2 - x4) * dy - (y2 - y4) * dx);
    float d3 = ((x3 - x4) * dy - (y3 - y4) * dx);
    d2 = (d2 >= 0) ? d2 : -d2;
    d3 = (d3 >= 0) ? d3 : -d3;
    if ((d2+d3) * (d2+d3) < tess_tol * (dx*dx + dy*dy))
    {
        path->push_back(v2f(x4, y4));
    }
    else if (level < 10)
    {
        float x12 = (x1+x2)*0.5f,       y12 = (y1+y2)*0.5f;
        float x23 = (x2+x3)*0.5f,       y23 = (y2+y3)*0.5f;
        float x34 = (x3+x4)*0.5f,       y34 = (y3+y4)*0.5f;
        float x123 = (x12+x23)*0.5f,    y123 = (y12+y23)*0.5f;
        float x234 = (x23+x34)*0.5f,    y234 = (y23+y34)*0.5f;
        float x1234 = (x123+x234)*0.5f, y1234 = (y123+y234)*0.5f;

        PathBezierToCasteljau(path, x1,y1,        x12,y12,    x123,y123,  x1234,y1234, tess_tol, level+1);
        PathBezierToCasteljau(path, x1234,y1234,  x234,y234,  x34,y34,    x4,y4,       tess_tol, level+1);
    }
}

void ImmDrawList::PathBezierCurveTo(const v2f& p2, const v2f& p3, const v2f& p4, int num_segments)
{
    v2f p1 = _Path.back();
    if (num_segments == 0)
    {
        // Auto-tessellated
        PathBezierToCasteljau(&_Path, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y, _Data->CurveTessellationTol, 0);
    }
    else
    {
        float t_step = 1.0f / (float)num_segments;
        for (int i_step = 1; i_step <= num_segments; i_step++)
        {
            float t = t_step * i_step;
            float u = 1.0f - t;
            float w1 = u*u*u;
            float w2 = 3*u*u*t;
            float w3 = 3*u*t*t;
            float w4 = t*t*t;
            _Path.push_back(v2f(w1*p1.x + w2*p2.x + w3*p3.x + w4*p4.x, w1*p1.y + w2*p2.y + w3*p3.y + w4*p4.y));
        }
    }
}

void ImmDrawList::PathRect(const v2f& a, const v2f& b, float rounding, int rounding_corners)
{
    rounding = std::min(rounding, fabsf(b.x - a.x) * ( ((rounding_corners & ImmDrawCornerFlags_Top)  == ImmDrawCornerFlags_Top)  || ((rounding_corners & ImmDrawCornerFlags_Bot)   == ImmDrawCornerFlags_Bot)   ? 0.5f : 1.0f ) - 1.0f);
    rounding = std::min(rounding, fabsf(b.y - a.y) * ( ((rounding_corners & ImmDrawCornerFlags_Left) == ImmDrawCornerFlags_Left) || ((rounding_corners & ImmDrawCornerFlags_Right) == ImmDrawCornerFlags_Right) ? 0.5f : 1.0f ) - 1.0f);

    if (rounding <= 0.0f || rounding_corners == 0)
    {
        PathLineTo(a);
        PathLineTo(v2f(b.x, a.y));
        PathLineTo(b);
        PathLineTo(v2f(a.x, b.y));
    }
    else
    {
        const float rounding_tl = (rounding_corners & ImmDrawCornerFlags_TopLeft) ? rounding : 0.0f;
        const float rounding_tr = (rounding_corners & ImmDrawCornerFlags_TopRight) ? rounding : 0.0f;
        const float rounding_br = (rounding_corners & ImmDrawCornerFlags_BotRight) ? rounding : 0.0f;
        const float rounding_bl = (rounding_corners & ImmDrawCornerFlags_BotLeft) ? rounding : 0.0f;
        PathArcToFast(v2f(a.x + rounding_tl, a.y + rounding_tl), rounding_tl, 6, 9);
        PathArcToFast(v2f(b.x - rounding_tr, a.y + rounding_tr), rounding_tr, 9, 12);
        PathArcToFast(v2f(b.x - rounding_br, b.y - rounding_br), rounding_br, 0, 3);
        PathArcToFast(v2f(a.x + rounding_bl, b.y - rounding_bl), rounding_bl, 3, 6);
    }
}

void ImmDrawList::AddLine(const v2f& a, const v2f& b, uint32_t col, float thickness)
{
    if ((col & IMM_COL32_A_MASK) == 0)
        return;
    PathLineTo(a + v2f(0.5f,0.5f));
    PathLineTo(b + v2f(0.5f,0.5f));
    PathStroke(col, false, thickness);
}

// a: upper-left, b: lower-right. we don't render 1 px sized rectangles properly.
void ImmDrawList::AddRect(const v2f& a, const v2f& b, uint32_t col, float rounding, int rounding_corners_flags, float thickness)
{
    if ((col & IMM_COL32_A_MASK) == 0)
        return;
    PathRect(a + v2f(0.5f,0.5f), b - v2f(0.5f,0.5f), rounding, rounding_corners_flags);
    PathStroke(col, true, thickness);
}

void ImmDrawList::AddRectFilled(const v2f& a, const v2f& b, uint32_t col, float rounding, int rounding_corners_flags)
{
    if ((col & IMM_COL32_A_MASK) == 0)
        return;
    if (rounding > 0.0f)
    {
        PathRect(a, b, rounding, rounding_corners_flags);
        PathFillConvex(col);
    }
    else
    {
        PrimReserve(6, 4);
        PrimRect(a, b, col);
    }
}

void ImmDrawList::AddRectFilledMultiColor(const v2f& a, const v2f& c, uint32_t col_upr_left, uint32_t col_upr_right, uint32_t col_bot_right, uint32_t col_bot_left)
{
    if (((col_upr_left | col_upr_right | col_bot_right | col_bot_left) & IMM_COL32_A_MASK) == 0)
        return;

    const v2f uv = _Data->TexUvWhitePixel;
    PrimReserve(6, 4);
    PrimWriteIdx((ImmDrawIdx)(_VtxCurrentIdx)); PrimWriteIdx((ImmDrawIdx)(_VtxCurrentIdx+1)); PrimWriteIdx((ImmDrawIdx)(_VtxCurrentIdx+2));
    PrimWriteIdx((ImmDrawIdx)(_VtxCurrentIdx)); PrimWriteIdx((ImmDrawIdx)(_VtxCurrentIdx+2)); PrimWriteIdx((ImmDrawIdx)(_VtxCurrentIdx+3));
    PrimWriteVtx(a, uv, col_upr_left);
    PrimWriteVtx(v2f(c.x, a.y), uv, col_upr_right);
    PrimWriteVtx(c, uv, col_bot_right);
    PrimWriteVtx(v2f(a.x, c.y), uv, col_bot_left);
}

void ImmDrawList::AddQuad(const v2f& a, const v2f& b, const v2f& c, const v2f& d, uint32_t col, float thickness)
{
    if ((col & IMM_COL32_A_MASK) == 0)
        return;

    PathLineTo(a);
    PathLineTo(b);
    PathLineTo(c);
    PathLineTo(d);
    PathStroke(col, true, thickness);
}

void ImmDrawList::AddQuadFilled(const v2f& a, const v2f& b, const v2f& c, const v2f& d, uint32_t col)
{
    if ((col & IMM_COL32_A_MASK) == 0)
        return;

    PathLineTo(a);
    PathLineTo(b);
    PathLineTo(c);
    PathLineTo(d);
    PathFillConvex(col);
}

void ImmDrawList::AddTriangle(const v2f& a, const v2f& b, const v2f& c, uint32_t col, float thickness)
{
    if ((col & IMM_COL32_A_MASK) == 0)
        return;

    PathLineTo(a);
    PathLineTo(b);
    PathLineTo(c);
    PathStroke(col, true, thickness);
}

void ImmDrawList::AddTriangleFilled(const v2f& a, const v2f& b, const v2f& c, uint32_t col)
{
    if ((col & IMM_COL32_A_MASK) == 0)
        return;

    PathLineTo(a);
    PathLineTo(b);
    PathLineTo(c);
    PathFillConvex(col);
}

void ImmDrawList::AddCircle(const v2f& centre, float radius, uint32_t col, int num_segments, float thickness)
{
    if ((col & IMM_COL32_A_MASK) == 0)
        return;

    const float a_max = (float)M_PI*2.0f * ((float)num_segments - 1.0f) / (float)num_segments;
    PathArcTo(centre, radius-0.5f, 0.0f, a_max, num_segments);
    PathStroke(col, true, thickness);
}

void ImmDrawList::AddCircleFilled(const v2f& centre, float radius, uint32_t col, int num_segments)
{
    if ((col & IMM_COL32_A_MASK) == 0)
        return;

    const float a_max = (float)M_PI*2.0f * ((float)num_segments - 1.0f) / (float)num_segments;
    PathArcTo(centre, radius, 0.0f, a_max, num_segments);
    PathFillConvex(col);
}

void ImmDrawList::AddBezierCurve(const v2f& pos0, const v2f& cp0, const v2f& cp1, const v2f& pos1, uint32_t col, float thickness, int num_segments)
{
    if ((col & IMM_COL32_A_MASK) == 0)
        return;

    PathLineTo(pos0);
    PathBezierCurveTo(cp0, cp1, pos1, num_segments);
    PathStroke(col, false, thickness);
}

void ImmDrawList::AddImage(ImmTextureId user_texture_id, const v2f& a, const v2f& b, const v2f& uv_a, const v2f& uv_b, uint32_t col)
{
    if ((col & IMM_COL32_A_MASK) == 0)
        return;

    const bool push_texture_id = _TextureIdStack.empty() || user_texture_id != _TextureIdStack.back();
    if (push_texture_id)
        PushTextureID(user_texture_id);

    PrimReserve(6, 4);
    PrimRectUV(a, b, uv_a, uv_b, col);

    if (push_texture_id)
        PopTextureID();
}

void ImmDrawList::AddImageQuad(ImmTextureId user_texture_id, const v2f& a, const v2f& b, const v2f& c, const v2f& d, const v2f& uv_a, const v2f& uv_b, const v2f& uv_c, const v2f& uv_d, uint32_t col)
{
    if ((col & IMM_COL32_A_MASK) == 0)
        return;

    const bool push_texture_id = _TextureIdStack.empty() || user_texture_id != _TextureIdStack.back();
    if (push_texture_id)
        PushTextureID(user_texture_id);

    PrimReserve(6, 4);
    PrimQuadUV(a, b, c, d, uv_a, uv_b, uv_c, uv_d, col);

    if (push_texture_id)
        PopTextureID();
}

void ImmDrawList::AddImageRounded(ImmTextureId user_texture_id, const v2f& a, const v2f& b, const v2f& uv_a, const v2f& uv_b, uint32_t col, float rounding, int rounding_corners)
{
    if ((col & IMM_COL32_A_MASK) == 0)
        return;

    if (rounding <= 0.0f || (rounding_corners & ImmDrawCornerFlags_All) == 0)
    {
        AddImage(user_texture_id, a, b, uv_a, uv_b, col);
        return;
    }

    const bool push_texture_id = _TextureIdStack.empty() || user_texture_id != _TextureIdStack.back();
    if (push_texture_id)
        PushTextureID(user_texture_id);

    int vert_start_idx = VtxBuffer.Size;
    PathRect(a, b, rounding, rounding_corners);
    PathFillConvex(col);
    int vert_end_idx = VtxBuffer.Size;
    ShadeVertsLinearUV(VtxBuffer.Data + vert_start_idx, VtxBuffer.Data + vert_end_idx, a, b, uv_a, uv_b, true);

    if (push_texture_id)
        PopTextureID();
}

typedef std::tuple<std::shared_ptr<uint8_t>, int, int> SpriteDesc;
std::unordered_map<lab::ImmSpriteId, SpriteDesc> cached_sprites;
std::unordered_map<uint8_t*, lab::ImmSpriteId> cached_sprites_reverse;


lab::ImmSpriteId SpriteId(std::shared_ptr<uint8_t> rgb, int w, int h)
{
    auto it = cached_sprites_reverse.find(rgb.get());
    if (it != cached_sprites_reverse.end())
    {
        return it->second;
    }

    static std::atomic<int> id(0);
    int new_id = ++id;

    cached_sprites_reverse[rgb.get()] = new_id;
    cached_sprites[new_id] = { rgb, w, h };
    return new_id;
}



//-----------------------------------------------------------------------------
// ImmDrawData
//-----------------------------------------------------------------------------


// Helper to scale the ClipRect field of each ImmDrawCmd. Use if your final output buffer is at a different scale than ImGui expects, or if there is a difference between your window resolution and framebuffer resolution.
void ImmDrawData::ScaleClipRects(const v2f& scale)
{
    for (int i = 0; i < CmdLists.size(); i++)
    {
        ImmDrawList* cmd_list = CmdLists[i];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            ImmDrawCmd* cmd = &cmd_list->CmdBuffer[cmd_i];
            cmd->ClipRect = v4f(cmd->ClipRect.x * scale.x, cmd->ClipRect.y * scale.y, cmd->ClipRect.z * scale.x, cmd->ClipRect.w * scale.y);
        }
    }
}


// OpenGL3 Render function.
// (this used to be set in io.RenderDrawListsFn and called by ImGui::Render(), but you can now call this directly from your main loop)
// Note that this implementation is little overcomplicated because we are saving/setting up/restoring every OpenGL state explicitly, in order to be able to run within any OpenGL engine that doesn't do so.
void ImmDrawData::Render(int fb_width, int fb_height)
{
    const v2f frameBufferScale = { 1.f, 1.f };
    this->ScaleClipRects(frameBufferScale);
    float displaySizeX = (float) fb_width;
    float displaySizeY = (float) fb_height;

    // Backup GL state
    GLenum last_active_texture; glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint*)&last_active_texture);
    glActiveTexture(GL_TEXTURE0);
    GLint last_program; glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
    GLint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    GLint last_sampler; glGetIntegerv(GL_SAMPLER_BINDING, &last_sampler);
    GLint last_array_buffer; glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
    GLint last_element_array_buffer; glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &last_element_array_buffer);
    GLint last_vertex_array; glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
    GLint last_polygon_mode[2]; glGetIntegerv(GL_POLYGON_MODE, last_polygon_mode);
    GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
    GLint last_scissor_box[4]; glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
    GLenum last_blend_src_rgb; glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)&last_blend_src_rgb);
    GLenum last_blend_dst_rgb; glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)&last_blend_dst_rgb);
    GLenum last_blend_src_alpha; glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)&last_blend_src_alpha);
    GLenum last_blend_dst_alpha; glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)&last_blend_dst_alpha);
    GLenum last_blend_equation_rgb; glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint*)&last_blend_equation_rgb);
    GLenum last_blend_equation_alpha; glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint*)&last_blend_equation_alpha);
    GLboolean last_enable_blend = glIsEnabled(GL_BLEND);
    GLboolean last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
    GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
    GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);

    // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, polygon fill
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Setup viewport, orthographic projection matrix
    glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
    const float ortho_projection[4][4] =
    {
        { 2.0f / displaySizeX, 0.0f,                   0.0f, 0.0f },
        { 0.0f,                  2.0f / -displaySizeY, 0.0f, 0.0f },
        { 0.0f,                  0.0f,                  -1.0f, 0.0f },
        { -1.0f,                  1.0f,                   0.0f, 1.0f },
    };

    auto glPtr = &gl;
    static bool once = [glPtr]() -> bool
    {
        glPtr->init();
        return true;
    }();

    glUseProgram(gl.shaderHandle);
    glUniform1i(gl.attribLocationTex, 0);
    glUniformMatrix4fv(gl.attribLocationProjMtx, 1, GL_FALSE, &ortho_projection[0][0]);
    glBindSampler(0, 0); // Rely on combined texture/sampler state.

    // Recreate the VAO every time
    // (This is to easily allow multiple GL contexts. VAO are not shared among GL contexts,
    // and we don't track creation/deletion of windows so we don't have an obvious key to use to cache them.)
    GLuint vao_handle = 0;
    glGenVertexArrays(1, &vao_handle);
    glBindVertexArray(vao_handle);
    glBindBuffer(GL_ARRAY_BUFFER, gl.vboHandle);

    glEnableVertexAttribArray(gl.attribLocationPosition);
    glVertexAttribPointer(gl.attribLocationPosition, 2, GL_FLOAT, GL_FALSE, sizeof(ImmDrawVert), (GLvoid*)IMM_OFFSETOF(ImmDrawVert, pos));

    if (gl.attribLocationUV >= 0)
    {
        glEnableVertexAttribArray(gl.attribLocationUV);
        glVertexAttribPointer(gl.attribLocationUV, 2, GL_FLOAT, GL_FALSE, sizeof(ImmDrawVert), (GLvoid*)IMM_OFFSETOF(ImmDrawVert, uv));
    }
    if (gl.attribLocationColor >= 0)
    {
        glEnableVertexAttribArray(gl.attribLocationColor);
        glVertexAttribPointer(gl.attribLocationColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImmDrawVert), (GLvoid*)IMM_OFFSETOF(ImmDrawVert, col));
    }

    // Draw
    for (int n = 0; n < this->CmdLists.size(); n++)
    {
        const ImmDrawList* cmd_list = this->CmdLists[n];
        const ImmDrawIdx* idx_buffer_offset = 0;

        glBindBuffer(GL_ARRAY_BUFFER, gl.vboHandle);
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImmDrawVert), (const GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl.elementsHandle);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImmDrawIdx), (const GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW);

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImmDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback)
            {
                pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                GLuint tx_id = pcmd->TextureId  ? (GLuint)(intptr_t)pcmd->TextureId : gl.defaultTexture;
                glBindTexture(GL_TEXTURE_2D, tx_id);
                glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
                glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImmDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer_offset);
            }
            idx_buffer_offset += pcmd->ElemCount;
        }
    }
    glDeleteVertexArrays(1, &vao_handle);

    // Restore modified GL state
    glUseProgram(last_program);
    glBindTexture(GL_TEXTURE_2D, last_texture);
    glBindSampler(0, last_sampler);
    glActiveTexture(last_active_texture);
    glBindVertexArray(last_vertex_array);
    glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, last_element_array_buffer);
    glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
    glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb, last_blend_src_alpha, last_blend_dst_alpha);
    if (last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
    if (last_enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
    if (last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
    if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, (GLenum)last_polygon_mode[0]);
    glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
    glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
}




// need one of these global.
static lab::ImmDrawListSharedData dlsd;
static lab::ImmDrawList* drawlist_ptr{ nullptr };    // tiny sprite lacks a user context pointer?


struct ImmRenderContext::Detail
{
    spritebatch_config_t config;
    spritebatch_t* batch{ nullptr };

    lab::ImmDrawData dd;
    lab::ImmDrawList dl;


    static void submit_batch(spritebatch_sprite_t* sprites, int count) 
    {
        if (!count)
            return;


        //printf("begin batch\n");
        //for (int i = 0; i < count; ++i) printf("\t%llu\n", sprites[i].texture_id);
        //printf("end batch\n");

        // NOTE:
        // perform any additional sorting here

        for (int i = 0; i < count; ++i)
        {
            spritebatch_sprite_t* s = sprites + i;
            lab::ImmTextureId texture = (uint32_t)s->texture_id;

            v2f quad[] = {
                { -0.5f,  0.5f },
                {  0.5f,  0.5f },
                {  0.5f, -0.5f },
                { -0.5f, -0.5f },
            };

            for (int j = 0; j < 4; ++j)
            {
                float x = quad[j].x;
                float y = quad[j].y;

                // scale sprite about origin
                x *= s->sx;
                y *= s->sy;

                // rotate sprite about origin
                float x0 = s->c * x - s->s * y;
                float y0 = s->s * x + s->c * y;
                x = x0;
                y = y0;

                // translate sprite into the world
                x += s->x;
                y += s->y;

                quad[j].x = x;
                quad[j].y = y;
            }

            drawlist_ptr->AddImageQuad(texture, quad[0],            quad[1],            quad[2],            quad[3],
                                      {s->minx, s->maxy}, {s->maxx, s->maxy}, {s->maxx, s->miny}, {s->minx, s->miny}, 
                                      0xffffffff);
        }
    }

    // given the user supplied image_id, return the raw pixels for that image
    static void* get_pixels(SPRITEBATCH_U64 image_id)
    {
        auto it = cached_sprites.find(image_id);
        if (it == cached_sprites.end())
            return nullptr;
        return std::get<0>(it->second).get();
    }

    static SPRITEBATCH_U64 generate_texture_handle(void* pixels, int w, int h)
    {
        GLuint location;
        glGenTextures(1, &location);
        glBindTexture(GL_TEXTURE_2D, location);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        glBindTexture(GL_TEXTURE_2D, 0);
        return (SPRITEBATCH_U64)location;
    }

    static void destroy_texture_handle(SPRITEBATCH_U64 texture_id)
    {
        GLuint id = (GLuint)texture_id;
        glDeleteTextures(1, &id);
    }

    void update_sprites()
    {
        // Run tinyspritebatch to find sprite batches.
        // This is the most basic usage of tinypsritebatch, one defrag, tick and flush per game loop.
        // It is also possible to only use defrag once every N frames.
        // tick can also be called at different time intervals (for example, once per game update
        // but not necessarily once per screen render).
        spritebatch_defrag(batch);
        spritebatch_tick(batch);
        spritebatch_flush(batch);
    }


    Detail()
    : dl(&dlsd)
    {
        drawlist_ptr = &dl;

        spritebatch_set_default_config(&config);
        config.batch_callback = submit_batch;                       // report batches of sprites from `spritebatch_flush`
        config.get_pixels_callback = get_pixels;                    // used to retrieve image pixels from `spritebatch_flush` and `spritebatch_defrag`
        config.generate_texture_callback = generate_texture_handle; // used to generate a texture handle from `spritebatch_flush` and `spritebatch_defrag`
        config.delete_texture_callback = destroy_texture_handle;    // used to destroy a texture handle from `spritebatch_defrag`

        batch = spritebatch_create();
        spritebatch_init(batch, &config);

        dl.AddDrawCmd();               // initial command to coalesce verts/indices into
        dd.CmdLists.push_back(&dl);    // just one command list for now.
    }

    ~Detail()
    {
        spritebatch_term(batch);
        spritebatch_destroy(batch);
    }
};



ImmRenderContext::ImmRenderContext()
: _detail(std::make_unique<Detail>())
{
}

ImmRenderContext::~ImmRenderContext() = default;

void ImmRenderContext::sprite(lab::ImmSpriteId id, int depth, float s, float theta_radians, float x, float y) 
{ 
    constexpr uint64_t sortbits = 0;
    constexpr float scale = 1.f;

    auto it = cached_sprites.find(id);
    if (it == cached_sprites.end())
        return;

    int w = std::get<1>(it->second);
    int h = std::get<2>(it->second);

    float sx = float(w) * s;  // premultiply scale by natural size
    float sy = float(h) * s;

    spritebatch_push(_detail->batch, id, w, h, x, y, sx, sy, sinf(theta_radians), cosf(theta_radians), sortbits);
}

// call render after everything is done to emit draw calls.
void ImmRenderContext::render(int w, int h)
{ 
    _detail->update_sprites();
    _detail->dd.Render(w, h);
}

// Render-level scissoring. This is passed down to your render function but not used for CPU-side coarse clipping.
void ImmRenderContext::push_clip_rect(v2f clip_rect_min, v2f clip_rect_max, bool intersect_with_current_clip_rect)
{
    _detail->dl.PushClipRect(clip_rect_min, clip_rect_max, intersect_with_current_clip_rect);
}
void ImmRenderContext::push_clip_rect_fullscreen() { _detail->dl.PushClipRectFullScreen(); }
void ImmRenderContext::pop_clip_rect() { _detail->dl.PopClipRect(); }

v2f  ImmRenderContext::clip_rect_min() const { return _detail->dl.GetClipRectMin(); }
v2f  ImmRenderContext::clip_rect_max() const { return _detail->dl.GetClipRectMax(); }

void ImmRenderContext::push_texture_id(lab::ImmTextureId texture_id) { _detail->dl.PushTextureID(texture_id); }
void ImmRenderContext::pop_texture_id() { _detail->dl.PopTextureID(); }

// Primitives
void ImmRenderContext::line(const v2f& a, const v2f& b, uint32_t col, float thickness)
{
    _detail->dl.AddLine(a, b, col, thickness);
}

void ImmRenderContext::rectangle(const lab::v2f& p0, const lab::v2f& p1, uint32_t c,
    float rounding, int rounding_corners_flags,
    float thickness)
{
    _detail->dl.AddRect(p0, p1, c, rounding, rounding_corners_flags);
}

void ImmRenderContext::rectangle_filled(const lab::v2f& p0, const lab::v2f& p1, uint32_t c,
    float rounding, int rounding_corners_flags,
    float thickness)
{
    _detail->dl.AddRectFilled(p0, p1, c, rounding, rounding_corners_flags);
}

void ImmRenderContext::rectangle_filled_multicolor(const lab::v2f& p0, const lab::v2f& p1,
    uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{
    _detail->dl.AddRectFilledMultiColor(p0, p1, a, b, c, d);
}

void ImmRenderContext::quad(const v2f& a, const v2f& b, const v2f& c, const v2f& d, uint32_t col, float thickness)
{
    _detail->dl.AddQuad(a, b, c, d, col, thickness);
}
void ImmRenderContext::quad_filled(const v2f& a, const v2f& b, const v2f& c, const v2f& d, uint32_t col)
{
    _detail->dl.AddQuadFilled(a, b, c, d, col);
}

void ImmRenderContext::triangle(const v2f& a, const v2f& b, const v2f& c, uint32_t col, float thickness)
{
    _detail->dl.AddTriangle(a, b, c, col, thickness);
}
void ImmRenderContext::triangle_filled(const v2f& a, const v2f& b, const v2f& c, uint32_t col)
{
    _detail->dl.AddTriangleFilled(a, b, c, col);
}

void ImmRenderContext::circle(const v2f& centre, float radius, uint32_t col, int num_segments, float thickness)
{
    _detail->dl.AddCircle(centre, radius, col, num_segments, thickness);
}
void ImmRenderContext::circle_filled(const v2f& centre, float radius, uint32_t col, int num_segments)
{
    _detail->dl.AddCircleFilled(centre, radius, col, num_segments);
}

//   LR_API void  AddText(const v2f& pos, uint32_t col, const char* text_begin, const char* text_end = NULL);
//   LR_API void  AddText(const ImmFont* font, float font_size, const v2f& pos, uint32_t col, const char* text_begin, const char* text_end = NULL, float wrap_width = 0.0f, const v4f* cpu_fine_clip_rect = NULL);

void ImmRenderContext::image(lab::ImmTextureId user_texture_id,
    const v2f& a, const v2f& b,
    const v2f& uv_a, const v2f& uv_b, uint32_t col)
{
    _detail->dl.AddImage(user_texture_id, a, b, uv_a, uv_b, col);
}
void ImmRenderContext::image_quad(lab::ImmTextureId user_texture_id,
    const v2f& a, const v2f& b, const v2f& c, const v2f& d,
    const v2f& uv_a, const v2f& uv_b,
    const v2f& uv_c, const v2f& uv_d, uint32_t col)
{
    _detail->dl.AddImageQuad(user_texture_id, a, b, c, d, uv_a, uv_b, uv_c, uv_d, col);
}
void ImmRenderContext::image_rounded(lab::ImmTextureId user_texture_id,
    const v2f& a, const v2f& b,
    const v2f& uv_a, const v2f& uv_b, uint32_t col,
    float rounding, int rounding_corners)
{
    _detail->dl.AddImageRounded(user_texture_id, a, b, uv_a, uv_b, col, rounding, rounding_corners);
}

void ImmRenderContext::poly_line(const v2f* points, const int num_points, uint32_t col, bool closed, float thickness)
{
    _detail->dl.AddPolyline(points, num_points, col, closed, thickness);
}
void ImmRenderContext::poly_convex_filled(const v2f* points, const int num_points, uint32_t col)
{
    _detail->dl.AddConvexPolyFilled(points, num_points, col);
}
void ImmRenderContext::bezier(const v2f& pos0, const v2f& cp0, const v2f& cp1, const v2f& pos1, uint32_t col,
    float thickness, int num_segments)
{
    _detail->dl.AddBezierCurve(pos0, cp0, cp1, pos1, col, thickness, num_segments);
}

// Stateful path API, add points then finish with PathFill() or PathStroke()
void ImmRenderContext::path_clear() { _detail->dl.PathClear(); }
void ImmRenderContext::path_line_to(const v2f& pos) { _detail->dl.PathLineTo(pos); }
void ImmRenderContext::path_fill_convex(uint32_t col) { _detail->dl.PathFillConvex(col); }
void ImmRenderContext::path_stroke(uint32_t col, bool closed, float thickness) 
{ 
    _detail->dl.PathStroke(col, closed, thickness); 
}

void ImmRenderContext::path_arc_to(const v2f& centre, float radius, float a_min, float a_max, int num_segments)
{
    _detail->dl.PathArcTo(centre, radius, a_min, a_max, num_segments);
}
void ImmRenderContext::path_arc_to_coarse(const v2f& centre, float radius, int a_min_of_12, int a_max_of_12)
{
    _detail->dl.PathArcToFast(centre, radius, a_min_of_12, a_max_of_12);
}
void ImmRenderContext::path_bezier_curve_to(const v2f& p1, const v2f& p2, const v2f& p3, int num_segments)
{
    _detail->dl.PathBezierCurveTo(p1, p2, p3, num_segments);
}
void ImmRenderContext::path_rect(const v2f& rect_min, const v2f& rect_max,
    float rounding, int rounding_corners_flags)
{
    _detail->dl.PathRect(rect_min, rect_max, rounding, rounding_corners_flags);
}


} //lab

