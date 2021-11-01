//
//  ErrorPolicy.cpp
//  LabRender
//
//  Created by Nick Porcino on 5/14/15.
//  Copyright (c) 2015 Planet IX. All rights reserved.
//

#include "LabRender/ErrorPolicy.h"
#include "gl4.h"

namespace lab {
    
    namespace {
        TestConditions _activeConditions = TestConditions::allErrors;
    }
    
    TestConditions activeConditions() {
        return _activeConditions;
    }
    void setActiveConditions(TestConditions c) {
        _activeConditions = c;
    }
    
    char const*const glEnumString(int err) {
        #define ERR(a) case a: return #a
        switch (err) {
                ERR(GL_NO_ERROR);
                ERR(GL_INVALID_ENUM);
                ERR(GL_INVALID_VALUE);
                ERR(GL_INVALID_OPERATION);
                ERR(GL_OUT_OF_MEMORY);
                ERR(GL_FRAMEBUFFER_COMPLETE);
                ERR(GL_FRAMEBUFFER_UNDEFINED);
                ERR(GL_FRAMEBUFFER_UNSUPPORTED);
                ERR(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT);
                ERR(GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER);
                ERR(GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER);
                ERR(GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE);
                ERR(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT);
                
#ifdef GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT
                ERR(GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT);
#endif
                ERR(GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS);
#ifdef GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT
                ERR(GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT);
#endif
#if defined __gl_h_ || defined __gl3_h_
                ERR(GL_INVALID_FRAMEBUFFER_OPERATION);
#endif
#if defined __gl_h_
                ERR(GL_STACK_OVERFLOW);
                ERR(GL_STACK_UNDERFLOW);
                //ERR(GL_TABLE_TOO_LARGE);
#endif
            default:
                return "Error occurred";
                break;
        }
    }

    
    Error handleGLError(ErrorPolicy errorPolicy, int glErr, char const*const error, char const*const source)
    {
        if (glErr != GL_NO_ERROR) {
            printf("LabRender GL error report");
            printf("----- %s -----\n", glEnumString(glErr));
            if (error) {
                printf("%s\n", error);
            }
            if (source) {
                printf("----- source code -----\n");
                printf("%s\n", source);
            }
            switch (errorPolicy) {
                case ErrorPolicy::onErrorLog:
                    break;
                    
                case ErrorPolicy::onErrorExit:
                    exit(0);
                    
                case ErrorPolicy::onErrorThrow:
                case ErrorPolicy::onErrorLogThrow:
                    throw std::runtime_error(error? error:"error");
            }
        }
        return Error::errorRaised;
    }
    
    Error handleError(ErrorPolicy errorPolicy, char const*const error, char const*const source)
    {
        return handleGLError(errorPolicy, glGetError(), error, source);
    }
    

} //lab
