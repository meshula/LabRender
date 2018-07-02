
// Copyright (c) 2003-2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT


#ifndef _SENSOR_H_
#define _SENSOR_H_

#include "InsectAI/InsectAI_Agent.h"
#include "InsectAI/InsectAI_Engine.h"
#include "InsectAI/DynamicState.h"

#include <vector>

namespace InsectAI {

    typedef Lab::Spinor<float> Spinor;

	/// @class	Sensor
	/// @brief	Virtual base class for all sensors
	///			Sensors are sensitive to particular kinds of agents
    ///
	class Sensor 
	{
    protected:
		Sensor(Entity* e)
        : m_Kind(0)
        , m_SensedAgent(SensingMask::kKindNull)
        , mbChooseClosest(false)
        , mSensorActivation(0.0f)
        , mSpeed(0.0f)
        , mEpoch(0)
        , mEntity(e)
		{
		}

		virtual ~Sensor();

	public:        
		enum ESensorWidth { kNearest, kAverage };

        void SetEntity(Entity* e) { mEntity = e; }

		void Reset();
        
		virtual void			Update(Engine*, float epoch, float dt);

		virtual	ESensorWidth	GetSensorWidth() const = 0;

		uint32					GetSensedAgentKind() const { return m_SensedAgent; }
		uint32					GetKind() const { return m_Kind; }

		void                    AddInput(std::shared_ptr<Sensor> pSensor);
        
        float                   SensorActivation() const { return mSensorActivation; }
        const Spinor&           Direction() const { return mDirection; }
        float                   Speed() const { return mSpeed; }
        
        float                   BlendSpeedInputs() const;
        Spinor                  BlendSteeringInputs() const;
        Spinor                  WeightedBlendSteeringInputs() const;

        bool					mbDirectional;
		bool					mbInternalSensor;
		bool					mbClearEachFrame;
		bool					mbChooseClosest;						///< if false, accumulate. if true, choose closest
		float					mClosestDistance;

		std::vector<std::shared_ptr<Sensor>>   mInputs;

    protected:
        virtual void _Update(Engine*, float dt) = 0;
        
		uint32					m_Kind;									///< RTTI
		SensingMask				m_SensedAgent;							///< the kind of agent that can be sensed
        float                   mEpoch;
        Entity*                 mEntity;                                ///< The entity holding the sensor
        
		float					mSensorActivation;
        Spinor                  mDirection;
        float                   mSpeed;
	};
    
} // InsectAI

#endif
