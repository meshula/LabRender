//
//  ErrorPolicy.h
//  LabRender
//
//  Created by Nick Porcino
//  Copyright (c) 2013 Planet IX. All rights reserved.
//

#pragma once

#include <LabRender/LabRender.h>

namespace lab {

    enum class ErrorPolicy { onErrorLog, onErrorExit, onErrorThrow, onErrorLogThrow };
    enum Error { errorNone = 0, errorRaised };

    enum TestConditions {
        creation = 1,
        shaderCompiler = 2,
        exhaustive = 4,

        allFailures = creation | shaderCompiler,
        allErrors = allFailures | exhaustive
    };

    LR_API Error handleError(ErrorPolicy, char const*const error, char const*const source = nullptr);
	LR_API Error handleGLError(ErrorPolicy, int glErr, char const*const error, char const*const source = nullptr);
	LR_API char const*const glEnumString(int err);

	LR_API TestConditions activeConditions();
	LR_API void setActiveConditions(TestConditions);

    inline Error checkError(ErrorPolicy policy, TestConditions conditions, char const*const error, char const*const source = nullptr) {
        if (!(conditions & activeConditions()))
            return Error::errorNone;

        return handleError(policy, error, source);
    }

} // LabRender
