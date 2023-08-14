#ifndef GL4_H
#define GL4_H

// This is a small library to make OpenGL 4 easier. All objects are lazily
// initialized so it is safe to declare them at global scope (if this wasn't
// the case, they would fail to be created as the OpenGL context wouldn't exist
// yet). The provided vector libraries attempt to imitate some of GLSL syntax.
// The wrappers are intentionally "leaky" in that they don't use private
// variables so you can implement functionality that they don't have if needed.

#ifdef __APPLE__

#include <TargetConditionals.h>

#  define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#  if TARGET_OS_IPHONE
#    import <OpenGLES/ES2/gl.h>
#    import <OpenGLES/ES2/glext.h>
#    define glBindVertexArray glBindVertexArrayOES
#    define glGenVertexArrays glGenVertexArraysOES
#    define glDeleteVertexArrays glDeleteVertexArraysOES
#  else
#    import <OpenGL/CGLTypes.h>
#    import <OpenGL/CGLCurrent.h>
#    import <OpenGL/gl3.h>
#  endif
#elif defined(_WIN32)
#  ifdef __gl_h_
#    error GL has already been included"
#  endif
#  include "GL/gl3w.h"
#  include <Windows.h>
#  undef near
#  undef far
#else
#  include <GL/gl.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <ostream>
#include <vector>
#include <math.h>

#include "LabRender/ErrorPolicy.h"
#include "LabRender/SemanticType.h"
#include "LabRender/Texture.h"
#include <LabMath/LabMath.h>

// Definitions for new macros in case they aren't defined.
#ifndef GL_R32F
# define GL_R32F 0x822E
#endif
#ifndef GL_RG32F
# define GL_RG32F 0x8230
#endif
#ifndef GL_RGB32F
# define GL_RGB32F 0x8815
#endif
#ifndef GL_RGBA32F
# define GL_RGBA32F 0x8814
#endif
#ifndef GL_R16F
# define GL_R16F 0x822D
#endif
#ifndef GL_RGB16F
# define GL_RGB16F 0x881B
#endif
#ifndef GL_RGBA16F
# define GL_RGBA16F 0x881A
#endif
#ifndef GL_PATCHES
# define GL_PATCHES 0x000E
#endif
#ifndef GL_HALF_FLOAT
# define GL_HALF_FLOAT 0x140B
#endif
#ifndef GL_PATCH_VERTICES
# define GL_PATCH_VERTICES 0x8E72
#endif
#ifndef GL_TESS_CONTROL_SHADER
# define GL_TESS_CONTROL_SHADER 0x8E88
#endif
#ifndef GL_TESS_EVALUATION_SHADER
# define GL_TESS_EVALUATION_SHADER 0x8E87
#endif

#define GL_GENERIC_ERROR 1

namespace LabRender {
    struct Texture;
}

const char* glErrorString(GLenum err);

// Convert a C++ type to an OpenGL type enum using TypeToOpenGL<T>::value
template <typename T> struct TypeToOpenGL {};
template <> struct TypeToOpenGL<bool>           { enum { value = GL_BOOL }; };
template <> struct TypeToOpenGL<float>          { enum { value = GL_FLOAT }; };
template <> struct TypeToOpenGL<double>         { enum { value = GL_DOUBLE }; };
template <> struct TypeToOpenGL<int>            { enum { value = GL_INT }; };
template <> struct TypeToOpenGL<char>           { enum { value = GL_BYTE }; };
template <> struct TypeToOpenGL<short>          { enum { value = GL_SHORT }; };
template <> struct TypeToOpenGL<unsigned int>   { enum { value = GL_UNSIGNED_INT }; };
template <> struct TypeToOpenGL<unsigned char>  { enum { value = GL_UNSIGNED_BYTE }; };
template <> struct TypeToOpenGL<unsigned short> { enum { value = GL_UNSIGNED_SHORT }; };



// ----------- RAII State management objects

class CaptureFrameBuffer {
public:
    CaptureFrameBuffer()  { glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currFramebuffer); }
    ~CaptureFrameBuffer() { glBindFramebuffer(GL_FRAMEBUFFER, currFramebuffer); }
    GLint currFramebuffer;
};
class CaptureTexture2DBinding {
public:
    CaptureTexture2DBinding()  { glGetIntegerv(GL_TEXTURE_BINDING_2D, &currentTextureBinding); }
    ~CaptureTexture2DBinding() { glBindTexture(GL_TEXTURE_2D, currentTextureBinding); }
    GLint currentTextureBinding;
};


#endif // GL4_H
