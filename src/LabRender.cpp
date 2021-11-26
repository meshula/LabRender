
#ifdef _WINDOWS
#include "GL/gl3w.h"
#include <stdio.h>
#endif

extern "C"
int labrender_init()
{
#ifdef _WINDOWS
    if (gl3wInit()) {
        fprintf(stderr, "labrender_init(): failed to initialize OpenGL\n");
        return -1;
    }
    if (!gl3wIsSupported(4, 2)) {
        fprintf(stderr, "labrender_init(): OpenGL 4.2 not supported\n");
        return -1;
    }
    printf("Renderer: %s\n", (const char*)glGetString(GL_RENDERER));
    printf("OpenGL %s, GLSL %s\n", 
        (const char*) glGetString(GL_VERSION),
        (const char*) glGetString(GL_SHADING_LANGUAGE_VERSION)); 
#endif
    return 0;
}

extern "C"
void labrender_shutdown()
{
}
