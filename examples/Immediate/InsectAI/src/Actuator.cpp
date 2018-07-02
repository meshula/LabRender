

// Copyright (c) 2003-2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT


#include "InsectAI/InsectAI.h"
#include "InsectAI/InsectAI_Actuator.h"
#include "InsectAI/InsectAI_Sensor.h"

#include <iostream>
#include <math.h>

namespace InsectAI 
{

    Actuator::Actuator(uint32 kind)
    : mKind(kind)
    , mEntity(0)
    {
    }

    Actuator::~Actuator() 
    {
    }

    void Actuator::AddInput(std::shared_ptr<InsectAI::Sensor> pSensor)
    {
        if (pSensor)
            mInputs.push_back(pSensor);
    }

    void Actuator::Update(Engine* engine, float epoch, float dt)
    {
        mActivation = 0;

        for (auto i = mInputs.begin(); i != mInputs.end(); ++i)
            (*i)->Update(engine, epoch, dt);
        
        // Calculation normalization amount
        float totalBlendWeight = 0.0f;
        for (auto i = mInputs.begin(); i != mInputs.end(); ++i)
            totalBlendWeight += (*i)->SensorActivation();

        // Linearly blend activation, weighted by sensor activation
        float speed = 0;
        if (totalBlendWeight > 0.0f)
            for (auto i = mInputs.begin(); i != mInputs.end(); ++i)
                speed += (*i)->Speed() * (*i)->SensorActivation() / totalBlendWeight;
        
        if (mEntity && totalBlendWeight > 0.0f) 
        {
            Lab::DynamicState& d = mEntity->state();
            mActivation = speed - d.GetSpeed();
                
            // Blend intended direction, weighted by sensor activation
            mDirection.set(0, 0);
            for (auto i = mInputs.begin(); i != mInputs.end(); ++i)
                mDirection += ((*i)->Direction() * (*i)->SensorActivation());
                
            mDirection.normalize();
                
            //const float kSteeringSpeed = 2.f;
            const float kThrust = 2.f;
                
            //float scale = -mDirection.j;
            float yaw = d.yaw_pitch_roll[0];
            float newYaw = yaw - mDirection.angle();
            float yaw1 = newYaw;//slerp<float>(0.1f, yaw, newYaw);
            d.yaw_pitch_roll[0] = yaw1;
            //d->ApplyAngularImpulse(dt * scale * kSteeringSpeed);
#if 0
            if (mKind == kMotor) {
                Spinor bodyDir = d->GetDirection();
                float speed = d.GetSpeed() + dt * kThrust * mActivation;
                if (speed > 100.f)
                    speed = 100.f;
                d->SetVelocity(bodyDir.i * speed, bodyDir.j * speed);
            }
#endif
        }
        else
            mDirection.set(1, 0);
    }
    
} // InsectAI

