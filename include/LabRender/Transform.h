//
//  Transform.h
//  LabApp
//
//  Created by Nick Porcino on 2014 02/24.
//  Copyright (c) 2014 Nick Porcino. All rights reserved.
//
#pragma once

#include "LabRender/LabRender.h"
#include <LabMath/LabMath.h>
#include <memory>

namespace lab {

    class Transform {
    public:
        Transform()
        : _translate(0,0,0)
        , _ypr(0,0,0)
        , _scale(1,1,1) {
			_transform = m44f_identity;
        }

        LR_API
        void setView(v3f target, v3f up);

        LR_API
        void setTRS(v3f t, v3f ypr_, v3f s);

        v3f  translate() const   { return _translate;                      }
        void setTranslate(v3f t) { _translate = t;   updateTransformTRS(); }
        v3f  ypr() const         { return _ypr;                            }
        void setYpr(v3f r)       { _ypr = r;         updateTransformTRS(); }
        v3f  scale() const       { return _scale;                          }
        void setScale(v3f s)     { _scale = s;       updateTransformTRS(); }

        const m44f & transform() const { return _transform; }

        LR_API
        Bounds transformBounds(const Bounds & bounds);

    private:
        LR_API void updateTransformTRS();

        v3f _translate;
        v3f _ypr;
        v3f _scale;
        m44f _transform;
    };

} // namespace Lab
