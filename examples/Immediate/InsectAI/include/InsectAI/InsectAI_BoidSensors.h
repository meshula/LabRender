
// Copyright (c) 2003-2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#pragma once

#include "InsectAI_Sensor.h"

namespace InsectAI {


/// @class	SeekCenterSensor
/// @brief	Seeks the local centroid of nearest neighbors
///
class SeekCenterSensor : public Sensor
{
protected:
    friend class SensorFactory;
    SeekCenterSensor(Entity* e);
    virtual ~SeekCenterSensor() { }
    
public:
    static uint32 GetStaticKind() { return 'SkCt'; }
    
    virtual void _Update(Engine*, float dt);
    
    //virtual void Sense(Lab::DynamicState* pOriginState, Lab::DynamicState* pSenseeState);
    ESensorWidth GetSensorWidth() const { return kNearest; }
};

/// @class	SeekAlignSensor
/// @brief	Seeks the common direction of nearest neighbors
///
class SeekAlignSensor : public Sensor
{
protected:
    friend class SensorFactory;
    SeekAlignSensor(Entity* e);
    virtual ~SeekAlignSensor() { }
    
public:
    static uint32 GetStaticKind() { return 'SkAl'; }
    
    virtual void _Update(Engine*, float dt);
    
    //    virtual void Sense(Lab::DynamicState* pOriginState, Lab::DynamicState* pSenseeState);
    ESensorWidth GetSensorWidth() const { return kNearest; }
};

/// @class	SeekSeparationSensor
/// @brief	Seeks to push away from nearest neighbours
///
class SeekSeparationSensor : public Sensor
{
protected:
    friend class SensorFactory;
    SeekSeparationSensor(Entity* e) : Sensor(e) { }
    virtual ~SeekSeparationSensor() { }
    
public:
    static uint32 GetStaticKind() { return 'SkSp'; }
    
    virtual void _Update(Engine*, float dt);
    
    //    virtual void Sense(Lab::DynamicState* pOriginState, Lab::DynamicState* pSenseeState);
    ESensorWidth GetSensorWidth() const { return kNearest; }
};

} // InsectAI
