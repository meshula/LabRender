#ifndef __DEMO_H_
#define __DEMO_H_

#include "InsectAI.h"
#include "NearestNeighbours.h"

#define MAX_AI 200

//modify demo main loop to update the nearest neighbour database
//remove my nn checks with calls to lq

//draw connections in brains
//make AI query for k nearest neighbours, don't do it in AI
//derive DynamicState from Physics or Physics2D, make position and rotation derived values

class PhysState;
class DemoVehicle;

class PhysState : public InsectAI::DynamicState, public NNProxy {
public:
			PhysState() : m_Rotation(0.0f), m_Vehicle(0) { m_Position[0] = k0; m_Position[1] = k0; m_Position[2] = k0; }
	virtual ~PhysState() { }

			float			DistanceSquared(float x, float y) {
				float dx = x - m_Position[0];
				float dy = y - m_Position[1];
				float distSquared = dx * dx + dy * dy;
				return distSquared;
			}

			float			DistanceSquared(PhysState* pRHS) {
				return DistanceSquared(pRHS->m_Position[0], pRHS->m_Position[1]);
			}

			// implements the DynamicState API
			Real const*const	GetPosition() const { return &m_Position[0]; }
			Real const			GetHeading() const { return m_Rotation; }
			Real const			GetPitch() const { return k0; }

			// implements the NNProxy API
			float const*const	GetPositionVectorPtr() const { return &m_Position[0]; }
			uint32	GetSearchMask() const { return m_Kind; }

			float				m_Rotation;
			PMath::Vec3f		m_Position;
			DemoVehicle*		m_Vehicle;		///< vehicles are tracked here, so we can render their brains
			uint32				m_Kind;			///< the kind of the AI
};

class DemoVehicle : public InsectAI::Vehicle {
public:
	DemoVehicle(PhysState* pState) : m_pState(pState) { }
	virtual ~DemoVehicle() { }
	InsectAI::DynamicState* GetDynamicState() { return m_pState; }
	PhysState* m_pState;				///< This is used to quickly get the phys state from a vehicle during sensing
};

class DemoLight : public InsectAI::Light {
public:
	DemoLight(PhysState* pState) : m_pState(pState) { }
	virtual ~DemoLight() { }
	InsectAI::DynamicState* GetDynamicState() { return m_pState; }
	PhysState* m_pState;				///< This is used to quickly get the phys state from a vehicle during sensing
};


/** @class	Demo
	@brief	Demonstrates the InsectAI library
	*/

class Demo : public VOpenGLMain, public InsectAI::EntityDatabase {
public:
			Demo();
	virtual ~Demo();

			enum { kVehicle = 1, kLight = 2 };	// bit masks, so we can or them together for searches

			void	Reset();
			void	Update(float dt);

			void	ClearAll();
			void	BuildTestBrain(InsectAI::Vehicle* pVehicle, uint32 brainType);
			void	CreateDefaultDemo();
			void	ChoosePotentialPick();
			void	DragEntity(int id);
			void	RenderEntities();
			int		FindClosestEntity(float x, float y, float maxDistance);
			void	HighlightEntity(int id, float radius, float red, float green, float blue);
			void	MoveEntities();

			InsectAI::DynamicState* GetState(InsectAI::Entity*);
			InsectAI::DynamicState* GetNearest(InsectAI::Entity*, uint32 filter);

			bool	HandleKey(int key);
			void	MouseMotion(int x, int y);
			void	MouseClick(eMouseButton button);
			void	MouseUnclick(eMouseButton button);
	virtual void	SetWindowSize(int width, int height, bool fullScreen);

			/// Wrap all the agents around a rectangular planar universe
			void	WrapAround(float left, float right, float bottom, float top);

protected:

	void	AddAllProxies();
	void	RemoveAllProxies();

	void	CreateDemoZero_Zero();
	void	CreateDemoZero_One();
	void	CreateDemoZero_Two();
	void	CreateDemoZero_Three();
	void	CreateDemoOne();
	void	CreateDemoTwo_Zero();
	void	CreateDemoTwo();
	void	CreateDemoThree();
	void	CreateLightSeekingAvoider();

	NearestNeighbours*		m_pNN;

	int						m_AICount;
	uint32					m_AI[MAX_AI];
	PhysState				m_State[MAX_AI];

	char*					m_DemoName;
	int						mDemoMode;
	int						mCurrentPick;
	int						mPotentialPick;
	InsectAI::Engine		m_Engine;
	int						mCurrentDemo;
	float					mMousex, mMousey;
};


#endif
