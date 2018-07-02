
#pragma once

#include <LabRender/MathTypes.h>
#include <array>

namespace InsectAI3 {

using lab::v3f;
using lab::quatf;

enum InertialKind { kI_Immobile, kI_Box,
                    kI_Sphere, kI_Ellipsoid, kI_HollowSphere, kI_Hemisphere, kI_HemisphereBottomHeavy,
                    kI_Cylinder, kI_CylinderBottomHeavy, kI_CylinderThinShell, kI_CylinderThinShellBottomHeavy,
                    kI_Torus, kI_Hoop };

struct DynamicState
{
    v3f pos;
    v3f vel;
    quatf orientation;
    v3f angular_velocity;
    v3f angular_momentum;

    v3f accel_prev;
    v3f torque_prev;

    void Zero();
};

struct PhysicsBody
{
    v3f _inverseTensorDiagonal;
    float _mass;

    DynamicState state0, state1;

    v3f _accumulated_force;
    v3f _accumulated_torque;

    v3f _extent;

    PhysicsBody();

    // setting mass tensor will reset dynamic state to zero.
    PhysicsBody& SetMassTensor(float mass, v3f extent, InertialKind i);

    // setting state will reset the rest of the dynamic state to zero.
    PhysicsBody& SetState(v3f pos, v3f vel, quatf orientation, v3f angular_velocity);
    
    PhysicsBody& ApplyForce(v3f force);
    PhysicsBody& Integrate(float dt);
};

//
// Motor driven by a light
//
// [M] <---- (*)
//


class LightSensor
{
public:
    float _sensitivity = 1.f;
    float _activation = 0.f;

    void accumulate_light(const v3f& light_relative_pos, float radiance);
};

class Motor
{
public:
    float accumulate_activation(float value, float weight) { return 0; }
    v3f force() const { return {0, 1.0f, 0}; }
};

struct Light
{
    v3f pos;
    float radiance;
};

struct Agent
{
    std::array<Light, 1> lights;

    PhysicsBody body;

    Agent()
    {
        lights[0].pos = { 100,100,0 };
        lights[0].radiance = 100.f;
        body.state0.Zero();

        body.SetMassTensor(1.f, {1,1,1}, kI_Sphere)
            .SetState({200,200,0}, {0,0,0}, {0,0,0,1}, {0,0,0});

    }

    void eval(float dt)
    {
        LightSensor ls;
        Motor m;

        for (auto i :lights)
        {
            ls.accumulate_light(i.pos, i.radiance);
        }
        float activation = ls._activation;

        m.accumulate_activation(activation, 1.f);

        v3f force = m.force();

        body
          .ApplyForce(force)
          .Integrate(dt);

/*
        PhysicsWorld w;
        int i = w.add(pb);

        while (dt > 0)
        {
            PhysicsWorld w2 = w.integrate(dt);

            CollisionManifold cm = Collide(w, w2);
            if (cm.count > 0)
            {
                w = w.integrate(cm.first_coVellision_time);
                dt -= cm.first_collision_time;
                get the item to resolve, and bounce it at this time
            }
            else
                break;
        }

        pb = w.get(i);
*/
    }
};

} // InsectAI3
