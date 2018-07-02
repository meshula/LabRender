

// Copyright (c) 2003-2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT


#include "InsectAI/InsectAI_Agent.h"

#include "InsectAI/DynamicState.h"
#include "InsectAI/InsectAI_Actuator.h"
#include "InsectAI/InsectAI_Sensor.h"

namespace InsectAI 
{
    Agent::Agent() 
    {
        m_State = std::make_unique<Lab::DynamicState>();
//        dynamic state needs to have an id assigned from the physics engine.
    }

    Agent::~Agent() 
    {
        for (auto i = m_Actuators.begin(); i != m_Actuators.end(); ++i) 
            (*i)->SetEntity(0);
    }

    void Agent::AddActuator(std::shared_ptr<Actuator> pActuator)
    {
        if (pActuator) {
            pActuator->SetEntity(this);
            m_Actuators.push_back(pActuator);
        }
    }
    void Agent::AddSensor(std::shared_ptr<Sensor> pSensor)
    {
        if (pSensor) {
            pSensor->SetEntity(this);
            m_Sensors.push_back(pSensor);
        }
    }

    void Agent::Update(Engine* engine, float epoch, float dt)
    {
        for (auto i = m_Actuators.begin(); i != m_Actuators.end(); ++i)
            (*i)->Update(engine, epoch, dt);
        for (auto i = m_Sensors.begin(); i != m_Sensors.end(); ++i)
            (*i)->Update(engine, epoch, dt);
    }
    



} // InsectAI
