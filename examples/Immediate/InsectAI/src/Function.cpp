
// Copyright (c) 2003-2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT


#include "InsectAI/InsectAI_Sensor.h"
#include "InsectAI/InsectAI_Sensors.h"

#include <math.h>

namespace InsectAI 
{

    BufferFunction::BufferFunction(Entity* e)
    : Sensor(e)
    , hysteresis(4.0f)  // 0.25s default
    {
        m_Kind = GetStaticKind();
        m_SensedAgent = SensingMask::kKindNull;
        mbClearEachFrame = false;
        mbInternalSensor = true;
        mbDirectional = false;
    }

    BufferFunction::~BufferFunction() 
    {
    }

    void BufferFunction::_Update(Engine*, float dt)
    {
        // Inputs will have had their _Updates called recursively first
        float input = 0;
        for (auto i = mInputs.begin(); i != mInputs.end(); ++i) {
            input += (*i)->SensorActivation();
        }
        
        mDirection = BlendSteeringInputs();
        mSpeed = BlendSpeedInputs();
        mSensorActivation = mSensorActivation + (input - mSensorActivation) * hysteresis * dt;
    }


    InverterFunction::InverterFunction(Entity* e)
    : Sensor(e)
    {
        m_Kind = GetStaticKind();
        m_SensedAgent = SensingMask::kKindNull;
        mbClearEachFrame = false;
        mbInternalSensor = true;
        mbDirectional = false;
    }

    void InverterFunction::_Update(Engine*, float dt)
    {
        float input = 0;
        
        // Inputs will have had their _Updates called recursively first
        for (auto i = mInputs.begin(); i != mInputs.end(); ++i) {
            input += (*i)->SensorActivation();
        }
        
        mDirection = BlendSteeringInputs();
        mSpeed = BlendSpeedInputs();
        mSensorActivation = 1.0f - input;
    }

    SigmoidFunction::SigmoidFunction(Entity* e)
    : Sensor(e)
    {
        m_Kind = GetStaticKind();
        m_SensedAgent = SensingMask::kKindNull;
        mbClearEachFrame = false;
        mbInternalSensor = true;
        mbDirectional = false;
    }

    void SigmoidFunction::_Update(Engine*, float dt)
    {
        float input = 0;
        
        // Inputs will have had their _Updates called recursively first
        for (auto i = mInputs.begin(); i != mInputs.end(); ++i) {
            input += (*i)->SensorActivation();
        }

        if (input <= 0.0f)
            mSensorActivation = 0.0f;
        else if (input >= 1.0f) 
            mSensorActivation = 1.0f;
        else {
            // 24 controls the speed of the slope. -0.5f shifts the range to be centered about 0.5 instead of 0
            mSensorActivation = (1.0f / (1.0f + expf(-(input-0.5f)*24.0f)));	
        }
        
        mSpeed = BlendSpeedInputs();
        mDirection = BlendSteeringInputs();
    }

    Switch::Switch(Entity* e)
    : Sensor(e)
    , mpSwitch(0)
    {
        m_Kind = GetStaticKind();
        m_SensedAgent = SensingMask::kKindNull;
        mbClearEachFrame = true;
        mbInternalSensor = true;
        mbDirectional = false;
        mInputs.reserve(2);
    }

    Switch::~Switch() 
    { 
    }


} // end namespace InsectAI
