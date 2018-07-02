
// Copyright (c) 2003-2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT



#include "InsectAI/InsectAI.h"
#include "InsectAI/InsectAI_BoidSensors.h"
#include "InsectAI/InsectAI_Engine.h"

#include <math.h>
#include <iostream>


namespace InsectAI {

    SeekAlignSensor::SeekAlignSensor(Entity* e)
    : Sensor(e)
    {
        mbDirectional = true;
    }

    void SeekAlignSensor::_Update(Engine* engine, float dt)
    {
        #if 0
        mSpeed = 0;
        
        Lab::DynamicState& d = mEntity->state();
        
        mClosestDistance = 2.0f;
        
        //const size_t k, Entity* pEntity, uint32_t kind, std::vector<Entity*>& result
        int contribs = 0;
        
        float totalBlendWeight = 0.0f;

        const int k = 7; // detect self and one other
        std::vector<Entity*> result(k);
        engine->FindClosestKEntities(k, mEntity, kKindAgent, result);
        for (std::vector<Entity*>::const_iterator i = result.begin(); i != result.end(); ++i) {
            if ((*i) == mEntity)
                continue;
            
            Lab::DynamicState* pTo = (*i)->state();
            mSpeed += pTo->GetSpeed();
            
            totalBlendWeight += 1.0f;   /// @TODO blend by distance - push positions back on a vector
            ++contribs;
        }
        
        if (contribs > 0) {
            mSpeed /= totalBlendWeight; // desired speed is average of neighbours'
            
            mDirection.set(0, 0);
            for (std::vector<Entity*>::const_iterator i = result.begin(); i != result.end(); ++i) {
                if ((*i) == mEntity)
                    continue;
                
                mDirection += (*i)->state()->GetDirection();
            }
            mDirection.normalize();
            mDirection.j *= -1;

            float yaw = d->GetYaw();
            Spinor dir(yaw);
            mDirection = mDirection * dir;  // move the angle into the agent's local space
        }
        else
            mDirection.set(1, 0);
        
        mSensorActivation = contribs > 0 ? 1.f : 0.f;
#endif
    }
    
    void SeekSeparationSensor::_Update(Engine*, float dt)
    {
        mSpeed = 0;
        mDirection.set(1, 0);
        mSensorActivation = 0;
    }
    
    SeekCenterSensor::SeekCenterSensor(Entity* e)
    : Sensor(e)
    {
        mbDirectional = true;
    }
    
    void SeekCenterSensor::_Update(Engine* engine, float dt)
    {
#if 0
        using Lab::LabVec3;

        Lab::DynamicState* d = mEntity->state();
        if (!d)
            return;
        
        mClosestDistance = 2.0f;
        
        //const size_t k, Entity* pEntity, uint32_t kind, std::vector<Entity*>& result
        int contribs = 0;
        
        float totalBlendWeight = 0.0f;
        
        const int k = 7; // detect self and one other
        std::vector<Entity*> result(k);
        engine->FindClosestKEntities(k, mEntity, kKindAgent, result);
        LabVec3 targetPos = { 0, 0, 0 };
        
        for (std::vector<Entity*>::const_iterator i = result.begin(); i != result.end(); ++i) 
        {
            if ((*i) == mEntity)
                continue;
            
            Lab::DynamicState* pTo = (*i)->state();
            LabVec3 pos = pTo->GetPosition();
            for (int i = 0; i < 3; ++i)
                targetPos[i] += pos[i];

            totalBlendWeight += 1.0f;
            ++contribs;
        }

        float sc = 1.0f / totalBlendWeight;
        for (int i = 0; i < 3; ++i)
            targetPos[i] *= sc;
        
        if (contribs > 0) 
        {
            LabVec3 from = d->GetPosition();
            LabVec3 temp;
            for (int i = 0; i < 3; ++i)
                temp[i] = targetPos[i] - from[i];

            mSpeed = std::sqrt(temp[0] * temp[0] + temp[1] * temp[1] + temp[2] * temp[2]);
            
            // normalize versus sensor radius
            float tempLength = mSpeed;
            mClosestDistance = tempLength / 100.f;//radius;
            
            // inverse square falloff
            mSensorActivation = std::max(0.0f, 1.0f - mClosestDistance);
            mSensorActivation = std::min(mSensorActivation, 1.0f);
            
            // TODO when steering considers more than one light, steering should be proportional to normalized activation
            // after activations have been calculated into a vector
            if (mbDirectional) {
                float ool = 1.0f / tempLength;
                mDirection.set(temp[0] * ool, -temp[1] * ool);
                
                float yaw = d->GetYaw();
                Lab::Spinor<float> dir(yaw);
                mDirection = mDirection * dir;  // move the angle into the agent's local space
            }            
        }
        else
            mDirection.set(1, 0);
        
        mSensorActivation = contribs > 0 ? 1.f : 0.f;
#endif
    }


}
