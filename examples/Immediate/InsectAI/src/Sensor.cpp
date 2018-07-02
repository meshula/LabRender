
// Copyright (c) 2003-2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT


#include "InsectAI/InsectAI_Sensor.h"

#include "InsectAI/InsectAI.h"
#include "InsectAI/InsectAI_Agent.h"
#include "InsectAI/InsectAI_BoidSensors.h"
#include "InsectAI/InsectAI_Sensors.h"
#include "InsectAI/DynamicState.h"

#include <float.h>
#include <limits.h>
#include <stdlib.h>



namespace InsectAI 
{
    using Spinor = Lab::Spinor<float>;
    using Lab::LabVec3;
    
    Sensor::~Sensor()
    {
    }

    void Sensor::AddInput(std::shared_ptr<Sensor> pSensor)
    {
        mInputs.push_back(pSensor);
    }
    
    void Sensor::Update(Engine* engine, float epoch, float dt)
    {
        // bail if done
        if (mEpoch >= epoch)
            return;
        
        // recurse for any uncomputed DAG inputs
        for (auto i = mInputs.begin(); i != mInputs.end(); ++i) {
            if ((*i)->mEpoch < epoch)
                (*i)->Update(engine, epoch, dt);
        }
        
        // do self computation
        _Update(engine, dt);
        
        // record epoch
        mEpoch = epoch;
    }

    void Sensor::Reset()
    {
        if (mbClearEachFrame) {
            mSensorActivation = 0.0f;
            mSpeed = 0.0f;
            mDirection.set(1, 0);
        }
        mClosestDistance = FLT_MAX;
    }
    
    float Sensor::BlendSpeedInputs() const
    {
        float speed = 0.f;
        size_t j = mInputs.size();
        float totalBlendWeight = (float) j;
        if (0 == totalBlendWeight)
            return 0;
        
        for (int i = 0; i < j; ++i)
            speed += mInputs[i]->Speed();
        
        return speed / totalBlendWeight;
    }
    
    Spinor Sensor::BlendSteeringInputs() const
    {
        Spinor steer;
        size_t j = mInputs.size();
        float totalBlendWeight = (float) j;
        if (0 == totalBlendWeight)
            return 0;
        
        float oobw = 1.0f / totalBlendWeight;
        steer = mInputs[0]->Direction();
        for (int i = 1; i < j; ++i)
            steer = Lab::slerp<float>(steer, mInputs[0]->Direction(), oobw);
        
        return steer;
    }

    Spinor Sensor::WeightedBlendSteeringInputs() const
    {
        Spinor steer;
        
        size_t j = mInputs.size();
        float totalBlendWeight = 0;
        for (int i = 0; i < j; ++i)
            totalBlendWeight += mInputs[i]->SensorActivation();
        
        if (0 == totalBlendWeight)
            return 0;
        
        float oobw = 1.0f / totalBlendWeight;
        steer = Lab::slerp<float>(0, mInputs[0]->Direction(), mInputs[0]->SensorActivation() * oobw);
        for (int i = 1; i < j; ++i)
            steer = Lab::slerp<float>(steer, mInputs[i]->Direction(), mInputs[i]->SensorActivation() * oobw);
        
        return steer;
    }

    CollisionSensor::CollisionSensor(Entity* e)
    : Sensor(e)
    {
        m_Kind = GetStaticKind();
        m_SensedAgent = Agent::GetStaticKind();
        mbDirectional = true;
        mbClearEachFrame = true;
        mbInternalSensor = false;
        mbChooseClosest = true;
    }

/*
    void CollisionSensor::Sense(Lab::DynamicState* pFrom, Lab::DynamicState* pTo) 
    {
        LabVec3 temp;
        LabVec3 to = pTo->GetPosition();
        LabVec3 from = pFrom->GetPosition();
        LabVec3Sub(&temp, &to, &from);

        float distance = LabVec3Length(&temp);

        // if close enough to collide
        if (distance < 0.1f) {
            float activation = std::max(0.0f, 1.0f - distance);
            activation *= activation;
            activation = std::min(1.0f, activation);

            // if closer than something else we're avoiding
            if (distance < mClosestDistance) {
                float steeringActivation;

                LabVec2Rotate(reinterpret_cast<LabVec2*>(&temp), reinterpret_cast<LabVec2*>(&temp), pFrom->GetYaw());

                // if the possible collidee is in front of us, or simply very very close
                if ((distance < 0.05f) || (temp.y > 0.0f)) {
                    LabVec3Scale(&temp, &temp, 1.0f / distance);
                    steeringActivation = -temp.x + PMath::randf(0.0f, 0.1f);	// a tiny bit of noise

                    temp.x = 0.0f;
                    temp.y = 1.0f;
                    LabVec2Rotate(reinterpret_cast<LabVec2*>(&temp), reinterpret_cast<LabVec2*>(&temp), pFrom->GetYaw());

                    LabVec2 temp2;
                    temp2.x = 0.0f;
                    temp2.y = 1.0f;
                    LabVec2Rotate(&temp2, &temp2, pTo->GetYaw());

                    // if the collidee is heading towards us or simply very very close
                    float dot = LabVec2Dot(&temp2, reinterpret_cast<LabVec2*>(&temp));
                    if ((distance < 0.05f) || (dot < 0.0f)) {
                        mClosestDistance = distance;
                        mSensorActivation = activation;
                        mSteeringActivation = steeringActivation;
                    }
                }
            }
        }
        mThrustActivation = mSensorActivation;
    }
    */
    
    void SumFunction::_Update(Engine*, float dt)
    {
        mSensorActivation = 0;
        mDirection = WeightedBlendSteeringInputs();
        mSpeed = BlendSpeedInputs();
        for (int i = 0; i < mInputs.size(); ++i) {
            mSensorActivation += mInputs[i]->SensorActivation();
        }
    }

    
    void Switch::_Update(Engine*, float dt)
    {
        if (!mInputs.size())
            return;
        
        int i = 0;
        int maxIndex = -1;
        float maxActivation = -std::numeric_limits<float>::infinity();
        for ( ; i < mInputs.size(); ++i) {
            if (mInputs[i]->SensorActivation() > maxActivation) {
                maxActivation = mInputs[i]->SensorActivation();
                maxIndex = i;
            }
        }
        if (maxIndex < 0)
            return;
        
        mSensorActivation = mInputs[maxIndex]->SensorActivation();
        mSpeed = mInputs[maxIndex]->Speed();
        mDirection = mInputs[maxIndex]->Direction();
    }
    
    LightSensor::LightSensor(Entity* e)
    : Sensor(e)
    , radius(100)
    {
        m_Kind = GetStaticKind();
        m_SensedAgent = Light::GetStaticKind();
        mbClearEachFrame = true;
        mbInternalSensor = false;
        mbDirectional = true;
    }
    
    void LightSensor::_Update(Engine* engine, float dt)
    {
        // https://github.com/sjuxax/cube2/blob/master/engine/bih.cpp
        // http://kaba.hilvi.org/pastel/pastel/geometry/bihtree_tools.hpp.htm
        // http://electronic-blue.wdfiles.com/local--files/research%3Agpurt/WK06.pdf

        mSensorActivation = 0;

        Lab::DynamicState& d = mEntity->state();
        mClosestDistance = 2.0f;
        
        /// TODO if mbChooseClosest, then FindClosestEntities and sum results
        
        Entity* closestLight = engine->FindClosestEntity(mEntity, kKindLight);
        if (closestLight) {            
            Lab::DynamicState& pTo = closestLight->state();
            LabVec3 temp;
            LabVec3 to = pTo.position;
            LabVec3 from = d.position;
            for (int i = 0; i < 3; ++i)
                temp[i] = to[i] - from[i];

            // normalize versus sensor radius
            float tempLength = std::sqrt(temp[0] * temp[0] + temp[1] * temp[1] + temp[2] * temp[2]);
            mClosestDistance = tempLength / radius;

            // inverse square falloff
            mSensorActivation = std::max(0.0f, 1.0f - mClosestDistance);
            mSensorActivation = std::min(mSensorActivation, 1.0f);
                
            // TODO when steering considers more than one light, steering should be proportional to normalized activation
            // after activations have been calculated into a vector
            if (mbDirectional) {
                float ool = 1.0f / tempLength;
                mDirection.set(temp[0] * ool, -temp[1] * ool);
                    
                float yaw = d.yaw_pitch_roll[0];
                Spinor dir(yaw);
                mDirection = mDirection * dir;  // move the angle into the agent's local space
            }
        }
        mSpeed = mSensorActivation;
    }


} // InsectAI
