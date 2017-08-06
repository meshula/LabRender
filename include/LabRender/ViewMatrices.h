//
//  ViewMatrices.h
//  LabRender
//
//  Copyright (c) 2015 Planet IX. All rights reserved.
//

#pragma once

#include "LabRender/MathTypes.h"

namespace lab {
    
    struct ViewMatrices 
	{
        m44f model;
        m44f view;
        m44f projection;
        m44f mv;
        m44f mvp;
    };
    
}
