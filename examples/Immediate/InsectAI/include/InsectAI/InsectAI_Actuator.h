// Copyright (c) 2003-2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#ifndef __ACTUATOR_H_
#define __ACTUATOR_H_

#include "InsectAI/InsectAI.h"
#include "InsectAI/InsectAI_Agent.h"

#include <stdint.h>
#include <memory>
#include <vector>

namespace InsectAI {

    class Engine;
	class Sensor;

	/// @class	Actuator
	/// @brief	Actuators are things like arms and motors
	class Actuator 
	{
	public:
        Actuator(uint32_t kind);
        virtual ~Actuator();
        
        enum { kMotor = 'Motr', kSteering = 'Ster' };

		void        Update(Engine*, float epoch, float dt);
		void        AddInput(std::shared_ptr<Sensor> pSensor);
		uint32_t    GetKind() const { return mKind; }

        void        SetEntity(Entity* e) { mEntity = e; }
        
		float       mActivation;
        Lab::Spinor<float> mDirection;

	protected:
        Entity*     mEntity;
		uint32_t    mKind;
        std::vector<std::shared_ptr<Sensor>> mInputs;
	};
    
} // end namespace InsectAI

#endif
