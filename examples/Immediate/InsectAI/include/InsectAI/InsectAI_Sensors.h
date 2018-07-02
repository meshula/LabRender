
// Copyright (c) 2003-2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#pragma once

#include "InsectAI_Sensor.h"

namespace InsectAI {

    
    
	/// @class	SeekCenterSensor
	/// @brief	Seeks the local centroid of nearest neighbors
    ///
	class NullSensor : public Sensor
	{
	public:
        NullSensor(Entity* e) : Sensor(e) { }
        virtual ~NullSensor() { }
        static uint32 GetStaticKind() { return 'Null'; }
        
		virtual void _Update(Engine*, float dt) { }
        
        //virtual void Sense(Lab::DynamicState* pOriginState, Lab::DynamicState* pSenseeState);
		ESensorWidth GetSensorWidth() const { return kNearest; }
	};
	
    /// @class	LightSensor
	/// @brief	The LightSensor can sense Light Agents
    ///
	class LightSensor : public Sensor
	{
	public:
        LightSensor(Entity* e);
        virtual ~LightSensor() { }
        
        static uint32 GetStaticKind() { return 'Lght'; }
		ESensorWidth GetSensorWidth() const { return kNearest; }
        
		virtual void _Update(Engine*, float dt);
		float radius;
	};
    
    
	
    /// @class	CollisionSensor
	/// @brief	Sensitive to collisions versus Vehicles
    ///
	class CollisionSensor : public Sensor
	{
	public:
        CollisionSensor(Entity*);
        virtual ~CollisionSensor() { }
        static uint32 GetStaticKind() { return 'Clld'; }
        
		virtual void _Update(Engine*, float dt) { }
        
        //virtual void Sense(Lab::DynamicState* pOriginState, Lab::DynamicState* pSenseeState);
		ESensorWidth GetSensorWidth() const { return kNearest; }
	};
    
    
	/// @class	Switch
	/// @brief	A switch toggles from input a to b when it's activation level exceeds 1/2
    ///
    /// @todo rename Switch to SelectMax
    ///
	class Switch : public Sensor
	{
	public:
        Switch(Entity*);
        virtual ~Switch();
        static uint32 GetStaticKind() { return 'Swch'; }
        
		void SetControl(Sensor* pS) { mpSwitch = pS; }
        ESensorWidth GetSensorWidth() const { return kNearest; }
        //virtual void Sense(Lab::DynamicState* pOriginState, Lab::DynamicState* pSenseeState) { }
        
        virtual void _Update(Engine*, float dt);
        
		Sensor* mpSwitch;
	};
    
	/// @class	BufferFunction
	/// @brief	adds 0.25s hysteresis to input activation
    ///
    class BufferFunction : public Sensor
	{
    public:
        BufferFunction(Entity*);
        virtual ~BufferFunction();

        static uint32 GetStaticKind() { return 'Buff'; }
		ESensorWidth GetSensorWidth() const { return kNearest; }
        
		virtual void _Update(Engine*, float dt);
        
        virtual void Sense(Lab::DynamicState* pOriginState, Lab::DynamicState* pSenseeState) { }
        
    private:
        float hysteresis; // 1/seconds
	};
    
	/// @class	SumFunction
	/// @brief	calculates sigmoid activation function, 0...1
    ///
    class SumFunction : public Sensor
	{
	public:
        SumFunction(Entity* e) : Sensor(e) { }
        virtual ~SumFunction() { }

        static uint32 GetStaticKind() { return 'Summ'; }
		ESensorWidth GetSensorWidth() const { return kNearest; }
        
        virtual void _Update(Engine*, float dt);
        
		//virtual void Sense(Lab::DynamicState* pOriginState, Lab::DynamicState* pSenseeState) { }
		uint32 mFunction;
	};
    
	/// @class	InverterFunction
	/// @brief	inverts input signal, a' = 1 - a
    ///
    class InverterFunction : public Sensor
	{
	public:
        InverterFunction(Entity*);
        virtual ~InverterFunction() { }

        static uint32 GetStaticKind() { return 'Nvrt'; }
		ESensorWidth GetSensorWidth() const { return kNearest; }
        
        virtual void _Update(Engine*, float dt);
        
		//virtual void Sense(Lab::DynamicState* pOriginState, Lab::DynamicState* pSenseeState) { }
		uint32 mFunction;
	};
    
	/// @class	SigmoidFunction
	/// @brief	calculates sigmoid activation function, 0...1
    ///
    class SigmoidFunction : public Sensor
	{
	public:
        SigmoidFunction(Entity*);
        virtual ~SigmoidFunction() { }

        static uint32 GetStaticKind() { return 'Sigm'; }
		ESensorWidth GetSensorWidth() const { return kNearest; }
        
        virtual void _Update(Engine*, float dt);
        
		//virtual void Sense(Lab::DynamicState* pOriginState, Lab::DynamicState* pSenseeState) { }
		uint32 mFunction;
	};
    
}