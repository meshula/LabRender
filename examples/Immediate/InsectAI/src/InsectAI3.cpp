
#include "InsectAI/InsectAI3.h"

#include <cmath>
#include <vector>

namespace InsectAI3
{
        constexpr float kEps = 0.0001f;

    using std::sqrt;

template <typename T>
T Sqr(T v) { return v * v; }

void DynamicState::Zero()
{
    pos = {0,0,0};
    vel = {0,0,0};
    orientation = {0,0,0,0};
    angular_velocity = {0,0,0};
    angular_momentum = {0,0,0};

    accel_prev = {0,0,0};
    torque_prev = {0,0,0};
}



PhysicsBody& PhysicsBody::SetMassTensor(float mass, v3f extent, InertialKind i)
{
    state0.Zero();
    _mass = mass;
    _extent = extent;

    float x2 = Sqr(extent.x);         // get the radius squared
    float y2 = Sqr(extent.y);
    float z2 = Sqr(extent.z);
    
    switch (i)
    {
    case kI_Sphere:
        _inverseTensorDiagonal.x = 
        _inverseTensorDiagonal.y = 
        _inverseTensorDiagonal.z = 5.f / (2.f * mass * x2);
        break;      // I = (2Mr*r) / 5

    case kI_Box:
        _inverseTensorDiagonal.x = 12.f / (mass * 4.f * (y2 + z2));  // multiply each dimension by 2 squared because x2 etc are half extent
        _inverseTensorDiagonal.y = 12.f / (mass * 4.f * (x2 + z2));
        _inverseTensorDiagonal.z = 12.f / (mass * 4.f * (x2 + y2));
        break;
            
    case kI_Ellipsoid:
        _inverseTensorDiagonal.x = 5.f / (mass * (y2 + z2));
        _inverseTensorDiagonal.y = 5.f / (mass * (x2 + z2));
        _inverseTensorDiagonal.z = 5.f / (mass * (x2 + y2));
        break;

    case kI_Cylinder:
        _inverseTensorDiagonal.x =                                               // x
        _inverseTensorDiagonal.z = 12.f / (mass * (3.f * x2 + y2));              // z
        _inverseTensorDiagonal.y = 2.f  / (mass * x2);                          // y
        break;
    
    case kI_CylinderBottomHeavy:
        _inverseTensorDiagonal.x  =                                              // x
        _inverseTensorDiagonal.z  = 12.f / (mass * (3.f * x2 + 4.f * y2));            // z
        _inverseTensorDiagonal.y  = 2.f  / (mass * x2);                         // y
        break;
        
    case kI_Hemisphere:
        _inverseTensorDiagonal.x =                                               // x
        _inverseTensorDiagonal.z = 320.0f / (mass * (83.0f * x2)); // z
        _inverseTensorDiagonal.y = 5.f    / (mass * (2.f * x2));          // y
        break;

    case kI_HemisphereBottomHeavy:
        _inverseTensorDiagonal.x  = 
        _inverseTensorDiagonal.y  = 
        _inverseTensorDiagonal.z  = 5.f / (mass * (2.f * x2));
        break;
        
    case kI_HollowSphere:
        {
            float a3 = y2 * extent.y;
            float a5 = a3 * y2;
            float b3 = x2 * extent.x;
            float b5 = b3 * x2;
            _inverseTensorDiagonal.x  = 
            _inverseTensorDiagonal.y  = 
            _inverseTensorDiagonal.z  = (5.f * (a3 - b3)) / (2.f * mass * (a5 - b5));
        }
        break;
        
    case kI_Hoop:  // radius squared in x2
        _inverseTensorDiagonal.x =                                                       // x
        _inverseTensorDiagonal.z = 1.f / (mass * x2);                                       // z
        _inverseTensorDiagonal.y = 2.f / (mass * x2);                                       // y
        break;
        
    case kI_CylinderThinShell: // x2 = radius squared, y2 = height squared
        _inverseTensorDiagonal.x =                                                       // x
        _inverseTensorDiagonal.z = 12.0f / (mass * (6.0f * x2 + y2));      // z
        _inverseTensorDiagonal.y = 1.f   / (mass * x2);                          // y
        break;
        
    case kI_CylinderThinShellBottomHeavy:  // x2 = radius squared, y2 = height squared
        _inverseTensorDiagonal.x =                                                       // x
        _inverseTensorDiagonal.z = 6.0f / (mass * (3.f * x2 + 2.f * y2));          // z
        _inverseTensorDiagonal.y = 1.f        / (mass * x2);                               // y
        break;
        
    case kI_Torus: // x2 = radius, y2 = secondary radius
        _inverseTensorDiagonal.x =                                                       // x
        _inverseTensorDiagonal.z = 8.f / (mass * (5.f * y2 + 4.f * x2));                  // z
        _inverseTensorDiagonal.y = 4.f / (mass * 3.f * y2 + 4.f * x2);                        // y
        break;
    }

    return *this;
}

PhysicsBody& PhysicsBody::SetState(v3f pos_, v3f vel_, quatf orientation_, v3f angular_velocity_)
{
    state0.Zero();
    state0.pos = pos_;
    state0.vel = vel_;
    state0.orientation = orientation_;
    state0.angular_velocity = angular_velocity_;
    state1 = state0;

    _accumulated_force = {0,0,0};
    _accumulated_torque = {0,0,0};

    return *this;
}


PhysicsBody::PhysicsBody()
{
    state0.Zero();
    state1 = state0;
    _inverseTensorDiagonal = {0,0,0};
    _mass = 0;
}

PhysicsBody& PhysicsBody::ApplyForce(v3f force)
{
    _accumulated_force += force;
    return *this;
}

PhysicsBody& PhysicsBody::Integrate(float dt)
{
	bool fellAsleep = false;

    bool m_Translatable = true;
    bool m_Collided = false;    // it should be set to true if collided in previous frame
    bool m_Spinnable = true;
    bool m_Gravity = false;

    v3f gravity = {0,-1,0};

    float m_LinearVelocityDamp = 0.9995f;
    float m_AngularVelocityDamp = 0.9995f;

    // reset for next time step

	// Normally in a Verlet integrator derives velocity from the previous state.
	// This engine tracks velocity explicitly, so the initialization step is simply to copy state 1 over 0.

    state0 = state1;

    //-------------------------------------------------------------------------
    // accumulate damping forces

	if (m_Translatable) {
		if (m_Collided) {
            // collided flag indicates sliding contact. Accumulate coulomb friction.

			// much more interesting friction is anisotropic; for example, a tire has much more transverse than lateral friction

			float vel = length(state0.vel);
            _accumulated_force += -0.95f * vel * state0.vel;
		}
		else {
			// linear velocity damping, can be used to simulate viscosity
			float forceSquared = dot(_accumulated_force, _accumulated_force);
			if (forceSquared < kEps) {
                state1.vel *= m_LinearVelocityDamp;
			}
		}

		/// @todo put body to sleep, if it is both in contact with an immobile object and moving at less
        /// than v = sqrt(2*g*eps). That's the velocity
		/// of an object initially at rest would have falling through the collision envelope [Mirtich95]
        // fellAsleep = theCheck();
	}

	if (m_Spinnable) {
		float aDamp = -_mass * m_AngularVelocityDamp;
        _accumulated_torque = aDamp * state0.angular_velocity; // input a torque opposing the angular velocity
	}

	m_Collided = false; // this will be retested in the contact resolution phase

    if (fellAsleep)
        return *this;

    // before springs are calculated

    //-------------------------------------------------------------------------
    // now follows the first half step integrand

	// if the body's position can change:
	if (m_Translatable) {
        state1.vel += dt * 0.5f * state1.accel_prev;      // v += 1/2 a * t
		state1.pos += dt * state1.vel;	                  // pos += v * dt
		state1.pos += 0.5f * dt * dt * state1.accel_prev; // pos += 1/2 a * t * t
	}

	// if the body can spin:			
	if (m_Spinnable) {		
		// L += 1/2 T * t
        state1.angular_momentum += dt * 0.5f * state1.torque_prev;	
		
        // update angular velocity from momentum
        // note, since the tensor is diagonal, world space and local space tensor is the same, so this works.
        /// @TODO shouldn't this be an accumulation, a multiply, and shouldn't it also be guaranteed <= 1?
        state1.angular_velocity = _inverseTensorDiagonal * state1.angular_momentum;

		/// @todo if using full matrix for tensor,
        /// must do worldTensor = orientation * inverseinertiatensor * transpose(orientation) before multiplying
		// R += t * angular velocity
        state1.orientation = lab::inputAngularVelocity(state1.orientation, dt, state1.angular_velocity);
	}

    // calculate springs here
    /// @TODO explain why here

    // solve contact constraints and collision resolution here
    // the first step should have rewound time until collision was resolved.

    //-------------------------------------------------------------------------
    // second integration step.
    //
    // if the body's position can change:
	if (m_Translatable) {
        // dvdt = f / m     works for gravity? yes:  f = mg    a = mg / m = g
        state0.accel_prev = _accumulated_force / _mass;  
		if (m_Gravity) {
			state0.accel_prev += gravity;
		}

        // v += 1/2 a * t
        state1.vel += dt * 0.5f * state0.accel_prev;

        // clear out the force accumulator
        _accumulated_force = {0,0,0};
	}

	// if the body can spin:
	if (m_Spinnable) {
		state0.torque_prev = _accumulated_torque;
        
        // L += 1/2 T * t        
        state1.angular_momentum += _accumulated_torque * dt * 0.5f;

        // clear the torque accumulator
        _accumulated_torque = {0,0,0};
	}

    return *this;
}


    void LightSensor::accumulate_light(const lab::v3f& light_relative_pos, float radiance) 
    { 
        float atten = dot(light_relative_pos, light_relative_pos) * radiance;
        _activation += atten; /// @TODO atten should be run through an activation function
    }


struct SweptSphere
{
    v3f a, b;
    float r;
};

struct Plane
{
    v3f n;
    float d;
};


float point_to_plane_distance(const v3f& point, const v3f& normal, float d) 
{
    return d + dot(normal, point);
}

// returns normalized time of first collision; FLT_MAX for no intersection
float intersect_swept_sphere_to_plane(float r, const v3f& c0, const v3f& c1, const v3f& n, float d)
{
	float d0 = point_to_plane_distance(c0, n, d);
	float d1 = point_to_plane_distance(c1, n, d);
	float radius = r;

	bool retval = false;

	if (std::abs(d0) <= radius) 
    {
        // object is starting in collision
		return (d0 - radius) / (d0 - d1);		// normalized time of first contact
	}
	else if (d0 > radius && d1 < radius) 
    {
        // object sweeps into collision
		return (d0 - radius) / (d0 - d1);		// normalized time of first contact
	}
	return FLT_MAX;
}




// Quadratic Formula from http://www.gamasutra.com/features/19991018/Gomez_2.htm
// returns true if both roots are real

inline bool QuadraticFormula(float a, float b, float c, float& root1, float& root2) 
{
	float q = b*b - 4.f * a * c;
	if (q >= 0.f) 
    {
		float sq = sqrt(q);
		float d = 1.f / (2.f * a);
		root1 = (-b + sq) * d;
		root2 = (-b - sq) * d;
		return true;			// real roots
	}
	else 
		return false;			// complex roots
}

bool Collide_Sphere___Sphere(v3f contact_normal, float radiusA, v3f posa, v3f posb, v3f va, float radiusB, v3f vb)
{
    bool	retval = false;

    float	u0, u1;
    //Subtract(va, pBodyA->m_StateT1.m_Position, pBodyA->m_StateT0.m_Position);
    //Subtract(vb, pBodyB->m_StateT1.m_Position, pBodyB->m_StateT0.m_Position);
    v3f vab = vb - va;
    v3f ab = posb - posa;
    float	rab = radiusA + radiusB;
    float	a = dot(vab, vab);				// u*u coefficient
    float	b = 2.f * dot(vab, ab);			// u coefficient
    float	c = dot(ab, ab) - rab * rab;	// constant term

    // check if they are overlapping
    if (c <= 0.f)
    {
        u0 = 0.f;
        u1 = 0.f;
        retval = true;
    }

    if (!retval)
    {
        // they didn't hit if they are moving perfectly parallel and at the same speed
        if (std::abs(a) < kEps) {
            retval = false;
        }

        // check if they hit
        else if (QuadraticFormula(a, b, c, u0, u1))
        {
            if (u0 > u1) {
                float temp = u0;
                u0 = u1;
                u1 = temp;
            }
            if (u0 < 0.f && u1 < 0.f) retval = false;
            else if (u1 > 1.f && u1 > 1.f) retval = false;
            else retval = true;
        }
    }

    if (retval)
    {
        //		contact_normal = pBodyA->m_StateT1.m_Position - pBodyB->m_StateT1.m_Position;
            //s	contact_normal = normalized(contact_normal);
    }

    return retval;
}

float intersect_swept_sphere_to_swept_sphere(float r1, const v3f& a0, const v3f& a1, const v3f& b0, const v3f& b1)
{
    return 0;
}


struct ContactManifold
{

    float dt;
    std::vector<SweptSphere> sweptSpheres;
    std::vector<Plane> planes;

    ContactManifold() = delete;
    ContactManifold(float dt) : dt(dt) {}

    ContactManifold& inifnite_plane(const v3f& normal, float distance)
    {
        Plane p { normal, distance };
        planes.emplace_back(p);
    }

    ContactManifold& sweep_body(PhysicsBody& b)
    {
        v3f pos = b.state0.pos;
        v3f vel = b.state0.vel;
        float r = lab::length(b._extent);

        SweptSphere sweep = { pos, pos + vel * dt, r };

        sweptSpheres.emplace_back(sweep);
        return *this;
    };

    float resolve()
    {
        if (sweptSpheres.size() <= 1)
            return FLT_MAX;

        float min_time = FLT_MAX;
        for (auto& i : sweptSpheres)
        {
//            float t = intersect_swept_sphere_to_plane(r, a, b, n, d);
  //          if (t > 0 && t <= dt && t < min_time)
    //            min_time = t;
        }

        int c = (int)sweptSpheres.size();
        for (int i = 0; i < c; ++i)
        {
            auto& a = sweptSpheres[i];
            for (int j = 0; j < c; ++j)
            {
                if (i == j)
                    continue;

//                float t = intersect_swept_sphere_to_swept_sphere();
  //              if (t < min_time)
    //                min_time = t;
            }
        }
    }
};


} // InsectAI3
