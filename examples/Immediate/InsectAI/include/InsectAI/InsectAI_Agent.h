
// Copyright (c) 2003-2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

/** @file	Agent.h
@brief	Most of the AI types are defined here
*/

#ifndef _AGENT_H_
#define _AGENT_H_

#include "InsectAI/DynamicState.h"

#include <memory>
#include <set>
#include <vector>

typedef unsigned int uint32;

namespace InsectAI 
{
    enum SensingMask { kKindNull = 0, kKindAgent = 1, kKindLight = 2 };

    class Engine;
    class Entity;
    class Sensor;
    class Actuator;
    class EntityDatabase;

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    /// @class	Entity
    /// @brief	Agents and other things which can be sensed derive from this

    class Entity 
    {
    public:
        Entity() { }
        virtual ~Entity() { }

        virtual void Update(Engine*, float epoch, float dt) = 0;
        virtual	SensingMask GetKind() const = 0;
        
        Lab::DynamicState& state() const { return *m_State.get(); }
        
    protected:
        std::unique_ptr<Lab::DynamicState> m_State;
    };
    

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    /// @class	Agent
    /// @brief	Agent base class

    class Agent : public Entity 
    {
    public:
        Agent();
        virtual ~Agent();

        SensingMask      GetKind() const { return GetStaticKind(); }
		static constexpr SensingMask GetStaticKind() { return kKindAgent; }

        void			 AddActuator(std::shared_ptr<Actuator> pActuator);
        void			 AddSensor(std::shared_ptr<Sensor> pSensor);

		void             Update(Engine*, float epoch, float dt);

        std::vector<std::shared_ptr<Actuator>> m_Actuators;
        std::vector<std::shared_ptr<Sensor>>   m_Sensors;
        uint32			 m_Kind;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    /// @class	Light
    /// @brief	A sense-able Light source
    ///			Lights are implemented as entities because Sensors are sensitive to entities

    class Light : public Entity 
    {
    public:
        Light();
        virtual ~Light();

        void             Update(Engine*, float epoch, float dt);
        SensingMask           GetKind() const { return GetStaticKind(); }
        static constexpr SensingMask GetStaticKind() { return kKindLight; }
    };

}	// end namespace InsectAI

#endif
