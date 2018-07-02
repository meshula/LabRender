
// Copyright (c) 2003-2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#pragma once

#include "InsectAI/InsectAI.h"
#include <array>
#include <cmath>

namespace Lab
{

	struct DynamicState
    {
        uint32_t body_id;
        LabVec3 position;
        LabVec3 yaw_pitch_roll;
        LabVec3 velocity;
        LabVec3 angular_velocity;
        void ApplyTorque(LabVec3 axis, float magnitude, float duration);
        void ApplyForce(LabVec3 location, LabVec3 impulse, float duration);

        float GetSpeed() const { return std::sqrt(velocity[0] * velocity[0] + velocity[1] * velocity[1] + velocity[2] * velocity[2]); }
	};

}
