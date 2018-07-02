
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#if defined(__APPLE__) && defined(__MACH__)
#include <OpenGL/gl.h>		// Header File For The OpenGL32 Library
#include <OpenGL/glu.h>	// Header File For The GLu32 Library
#else
#include <GL/gl.h>		// Header File For The OpenGL32 Library
#include <GL/glu.h>	// Header File For The GLu32 Library
#endif

#include "SDL.h"

#include "PMath.h"
#include "InsectAI.h"
#include "Clock.h"

#include "demo.h"

#define kNone 0
#define kDragging 1

extern bool showBrains;
using PMath::randf;
void DrawUserPrompts(char* title, char* mouse, char* keys);

#define MAXDEMO 7

using InsectAI::LightSensor;
using InsectAI::CollisionSensor;
using InsectAI::Actuator;
using InsectAI::Function;
using InsectAI::Switch;

#ifdef USE_GLFONT
//#include "glf.h"
// gl font library
// http://students.cs.byu.edu/~bfish/glfont.php
#endif

bool showBrains = true;

static bool gFullScreen = false;

int font;

Demo::Demo() : 
m_pNN(0),
mCurrentDemo(0), m_AICount(0), m_DemoName(0), 
mMousex(0), mMousey(0), mDemoMode(0), VOpenGLMain() 
{
	ClearAll();
}

Demo::~Demo() { 
	delete m_pNN;
}

void Demo::BuildTestBrain(InsectAI::Vehicle* pVehicle, uint32 brainType) {
	LightSensor* pLightSensor;
	CollisionSensor* pCollisionSensor;
	Actuator* pMotor;

	static uint32 funcs[3] = {Function::kBuffer, Function::kInvert, Function::kSigmoid};

	bool directional = (brainType > 3 && brainType < 8);

	switch (brainType) {

		// light activated or light seeking
		case 0:
		case 4:
			{	
				pVehicle->AllocBrain(1, 1);
				pLightSensor = new LightSensor(directional);
				pMotor = new Actuator(Actuator::kMotor);
				pMotor->SetInput(pLightSensor);

				pVehicle->AddSensor(pLightSensor);
				pVehicle->AddActuator(pMotor);
			}
			break;

			// light activated or light seeking with a transfer function
		case 1:
		case 2:
		case 3:
		case 5:
		case 6:
		case 7:
			{
				pVehicle->AllocBrain(2, 1);
				pLightSensor = new LightSensor(directional);
				Function* pFunc = new Function(funcs[(brainType-1) & 3]);
				pFunc->AddInput(pLightSensor);

				pMotor = new Actuator(Actuator::kMotor);
				pMotor->SetInput(pFunc);

				pVehicle->AddSensor(pFunc);
				pVehicle->AddSensor(pLightSensor);
				pVehicle->AddActuator(pMotor);
			}
			break;

			// light seeking, with collision avoidance
		case 8:
			{
				pVehicle->AllocBrain(3, 1);
				pLightSensor = new LightSensor(true);		// directional light sensor
				pCollisionSensor = new CollisionSensor();
				Switch* pSwitch = new Switch();
				pSwitch->SetControl(pCollisionSensor);
				pSwitch->SetInputs(pLightSensor, pCollisionSensor);

				pMotor = new Actuator(Actuator::kMotor);
				pMotor->SetInput(pSwitch);

				pVehicle->AddSensor(pCollisionSensor);
				pVehicle->AddSensor(pSwitch);
				pVehicle->AddSensor(pLightSensor);
				pVehicle->AddActuator(pMotor);
			}
			break;
	}
}


static void DrawDart() {
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f( 0.0f, 0.75f);	// nose
	glVertex2f(-0.5f, -0.75f);	// left wing
	glVertex2f( 0.0f, -0.5f);
	glVertex2f( 0.5f, -0.75f);	// right wing
	glEnd();
}

void DrawCircle() {
	glBegin(GL_LINE_STRIP);
	for (float i = 0; i < kPi * 2.0f; i += kPi / 10.0f) {
		glVertex2f(cosf(i), sinf(i));
	}
	glVertex2f(1.0f, 0.0f);	// close the circle
	glEnd();
}

void DrawFilledCircle() {
	glBegin(GL_TRIANGLE_FAN);
	for (float i = 0; i < kPi * 2.0f; i += kPi / 10.0f) {
		glVertex2f(cosf(i), sinf(i));
	}
	glVertex2f(1.0f, 0.0f);	// close the circle
	glEnd();
}

void Demo::ClearAll() {
	RemoveAllProxies();
	m_Engine.RemoveAllEntities();
	m_AICount = 0;
}

void Demo::CreateDemoZero_Zero() {
	m_DemoName = "Light Sensitive - linear response";
	ClearAll();
	DemoLight* pLight = new DemoLight(&m_State[m_AICount]);
	m_State[m_AICount].m_Kind = kLight;
	m_State[m_AICount].m_Rotation = k0;
	m_State[m_AICount].m_Position[0] = kHalf;
	m_State[m_AICount].m_Position[1] = kHalf;
	m_AI[m_AICount++] = m_Engine.AddEntity(pLight);
	DemoVehicle* pVehicle = new DemoVehicle(&m_State[m_AICount]);;
	m_State[m_AICount].m_Kind = kVehicle;
	m_State[m_AICount].m_Vehicle = pVehicle;
	m_State[m_AICount].m_Rotation = k0;
	m_State[m_AICount].m_Position[0] = randf();
	m_State[m_AICount].m_Position[1] = randf();
	BuildTestBrain(pVehicle, 0);
	pVehicle->mMaxSpeed = randf(0.8f, 1.0f);
	m_AI[m_AICount++] = m_Engine.AddEntity(pVehicle);
}

void Demo::CreateDefaultDemo() {
	CreateDemoZero_Zero();
}

void Demo::CreateDemoZero_One() {
	m_DemoName = "Light Sensitive with Buffer - delayed response";

	ClearAll();
	DemoLight* pLight = new DemoLight(&m_State[m_AICount]);
	m_State[m_AICount].m_Kind = kLight;
	m_State[m_AICount].m_Rotation = k0;
	m_State[m_AICount].m_Position[0] = kHalf;
	m_State[m_AICount].m_Position[1] = kHalf;
	m_AI[m_AICount++] = m_Engine.AddEntity(pLight);
	DemoVehicle* pVehicle = new DemoVehicle(&m_State[m_AICount]);;
	m_State[m_AICount].m_Vehicle = pVehicle;
	m_State[m_AICount].m_Kind = kVehicle;
	m_State[m_AICount].m_Rotation = k0;
	m_State[m_AICount].m_Position[0] = randf();
	m_State[m_AICount].m_Position[1] = randf();
	BuildTestBrain(pVehicle, 1);
	pVehicle->mMaxSpeed = randf(0.8f, 1.0f);
	m_AI[m_AICount++] = m_Engine.AddEntity(pVehicle);
}

void Demo::CreateDemoZero_Two() {
	m_DemoName = "Light Sensitive with Inverter";
	ClearAll();
	DemoLight* pLight = new DemoLight(&m_State[m_AICount]);
	m_State[m_AICount].m_Kind = kLight;
	m_State[m_AICount].m_Rotation = k0;
	m_State[m_AICount].m_Position[0] = kHalf;
	m_State[m_AICount].m_Position[1] = kHalf;
	m_AI[m_AICount++] = m_Engine.AddEntity(pLight);
	DemoVehicle* pVehicle = new DemoVehicle(&m_State[m_AICount]);;
	m_State[m_AICount].m_Vehicle = pVehicle;
	m_State[m_AICount].m_Kind = kVehicle;
	m_State[m_AICount].m_Rotation = k0;
	m_State[m_AICount].m_Position[0] = randf();
	m_State[m_AICount].m_Position[1] = randf();
	BuildTestBrain(pVehicle, 2);
	pVehicle->mMaxSpeed = randf(0.8f, 1.0f);
	m_AI[m_AICount++] = m_Engine.AddEntity(pVehicle);
}

void Demo::CreateDemoZero_Three() {
	m_DemoName = "Light Sensitive with Threshold";
	ClearAll();
	DemoLight* pLight = new DemoLight(&m_State[m_AICount]);
	m_State[m_AICount].m_Kind = kLight;
	m_State[m_AICount].m_Rotation = k0;
	m_State[m_AICount].m_Position[0] = kHalf;
	m_State[m_AICount].m_Position[1] = kHalf;
	m_AI[m_AICount++] = m_Engine.AddEntity(pLight);
	DemoVehicle* pVehicle = new DemoVehicle(&m_State[m_AICount]);;
	m_State[m_AICount].m_Vehicle = pVehicle;
	m_State[m_AICount].m_Kind = kVehicle;
	m_State[m_AICount].m_Rotation = k0;
	m_State[m_AICount].m_Position[0] = randf();
	m_State[m_AICount].m_Position[1] = randf();
	BuildTestBrain(pVehicle, 3);
	pVehicle->mMaxSpeed = randf(0.8f, 1.0f);
	m_AI[m_AICount++] = m_Engine.AddEntity(pVehicle);
}

void Demo::CreateDemoOne() {
	m_DemoName = "Light Sensitive Comparison";
	ClearAll();
	DemoLight* pLight = new DemoLight(&m_State[m_AICount]);
	m_State[m_AICount].m_Kind = kLight;
	m_State[m_AICount].m_Rotation = k0;
	m_State[m_AICount].m_Position[0] = kHalf;
	m_State[m_AICount].m_Position[1] = kHalf;
	m_AI[m_AICount++] = m_Engine.AddEntity(pLight);
	DemoVehicle* pVehicle = new DemoVehicle(&m_State[m_AICount]);;
	m_State[m_AICount].m_Vehicle = pVehicle;
	m_State[m_AICount].m_Kind = kVehicle;
	m_State[m_AICount].m_Rotation = k0;
	m_State[m_AICount].m_Position[0] = randf();
	m_State[m_AICount].m_Position[1] = randf();
	BuildTestBrain(pVehicle, 0);
	pVehicle->mMaxSpeed = randf(0.8f, 1.0f);
	m_AI[m_AICount++] = m_Engine.AddEntity(pVehicle);
	pVehicle = new DemoVehicle(&m_State[m_AICount]);;
	m_State[m_AICount].m_Vehicle = pVehicle;
	m_State[m_AICount].m_Kind = kVehicle;
	m_State[m_AICount].m_Rotation = k0;
	m_State[m_AICount].m_Position[0] = randf();
	m_State[m_AICount].m_Position[1] = randf();
	BuildTestBrain(pVehicle, 1);
	pVehicle->mMaxSpeed = randf(0.8f, 1.0f);
	m_AI[m_AICount++] = m_Engine.AddEntity(pVehicle);
	pVehicle = new DemoVehicle(&m_State[m_AICount]);;
	m_State[m_AICount].m_Vehicle = pVehicle;
	m_State[m_AICount].m_Kind = kVehicle;
	m_State[m_AICount].m_Rotation = k0;
	m_State[m_AICount].m_Position[0] = randf();
	m_State[m_AICount].m_Position[1] = randf();
	BuildTestBrain(pVehicle, 2);
	pVehicle->mMaxSpeed = randf(0.8f, 1.0f);
	m_AI[m_AICount++] = m_Engine.AddEntity(pVehicle);
	pVehicle = new DemoVehicle(&m_State[m_AICount]);;
	m_State[m_AICount].m_Vehicle = pVehicle;
	m_State[m_AICount].m_Kind = kVehicle;
	m_State[m_AICount].m_Rotation = k0;
	m_State[m_AICount].m_Position[0] = randf();
	m_State[m_AICount].m_Position[1] = randf();
	BuildTestBrain(pVehicle, 3);
	pVehicle->mMaxSpeed = randf(0.8f, 1.0f);
	m_AI[m_AICount++] = m_Engine.AddEntity(pVehicle);
}

void Demo::CreateDemoTwo_Zero() {
	m_DemoName = "Light Seeking - linear response";
	ClearAll();
	DemoLight* pLight = new DemoLight(&m_State[m_AICount]);
	m_State[m_AICount].m_Kind = kLight;
	m_State[m_AICount].m_Rotation = k0;
	m_State[m_AICount].m_Position[0] = kHalf;
	m_State[m_AICount].m_Position[1] = kHalf;
	m_AI[m_AICount++] = m_Engine.AddEntity(pLight);
	DemoVehicle* pVehicle = new DemoVehicle(&m_State[m_AICount]);;
	m_State[m_AICount].m_Vehicle = pVehicle;
	m_State[m_AICount].m_Kind = kVehicle;
	m_State[m_AICount].m_Rotation = k0;
	m_State[m_AICount].m_Position[0] = randf();
	m_State[m_AICount].m_Position[1] = randf();
	BuildTestBrain(pVehicle, 4);
	pVehicle->mMaxSpeed = randf(0.8f, 1.0f);
	m_AI[m_AICount++] = m_Engine.AddEntity(pVehicle);
}

void Demo::CreateDemoTwo() {
	m_DemoName = "Light Seeking Comparison";
	ClearAll();
	DemoLight* pLight = new DemoLight(&m_State[m_AICount]);
	m_State[m_AICount].m_Kind = kLight;
	m_State[m_AICount].m_Rotation = k0;
	m_State[m_AICount].m_Position[0] = kHalf;
	m_State[m_AICount].m_Position[1] = kHalf;
	m_AI[m_AICount++] = m_Engine.AddEntity(pLight);
	DemoVehicle* pVehicle = new DemoVehicle(&m_State[m_AICount]);;
	m_State[m_AICount].m_Vehicle = pVehicle;
	m_State[m_AICount].m_Kind = kVehicle;
	m_State[m_AICount].m_Rotation = k0;
	m_State[m_AICount].m_Position[0] = randf();
	m_State[m_AICount].m_Position[1] = randf();
	BuildTestBrain(pVehicle, 4);
	pVehicle->mMaxSpeed = randf(0.8f, 1.0f);
	m_AI[m_AICount++] = m_Engine.AddEntity(pVehicle);
	pVehicle = new DemoVehicle(&m_State[m_AICount]);;
	m_State[m_AICount].m_Vehicle = pVehicle;
	m_State[m_AICount].m_Kind = kVehicle;
	m_State[m_AICount].m_Rotation = k0;
	m_State[m_AICount].m_Position[0] = randf();
	m_State[m_AICount].m_Position[1] = randf();
	BuildTestBrain(pVehicle, 5);
	pVehicle->mMaxSpeed = randf(0.8f, 1.0f);
	m_AI[m_AICount++] = m_Engine.AddEntity(pVehicle);
	pVehicle = new DemoVehicle(&m_State[m_AICount]);;
	m_State[m_AICount].m_Vehicle = pVehicle;
	m_State[m_AICount].m_Kind = kVehicle;
	m_State[m_AICount].m_Rotation = k0;
	m_State[m_AICount].m_Position[0] = randf();
	m_State[m_AICount].m_Position[1] = randf();
	BuildTestBrain(pVehicle, 6);
	pVehicle->mMaxSpeed = randf(0.8f, 1.0f);
	m_AI[m_AICount++] = m_Engine.AddEntity(pVehicle);
	pVehicle = new DemoVehicle(&m_State[m_AICount]);;
	m_State[m_AICount].m_Vehicle = pVehicle;
	m_State[m_AICount].m_Kind = kVehicle;
	m_State[m_AICount].m_Rotation = k0;
	m_State[m_AICount].m_Position[0] = randf();
	m_State[m_AICount].m_Position[1] = randf();
	BuildTestBrain(pVehicle, 7);
	pVehicle->mMaxSpeed = randf(0.8f, 1.0f);
	m_AI[m_AICount++] = m_Engine.AddEntity(pVehicle);
}

void Demo::CreateLightSeekingAvoider() {
	DemoVehicle* pVehicle = new DemoVehicle(&m_State[m_AICount]);;
	m_State[m_AICount].m_Vehicle = pVehicle;
	m_State[m_AICount].m_Kind = kVehicle;
	m_State[m_AICount].m_Rotation = k0;
	m_State[m_AICount].m_Position[0] = randf();
	m_State[m_AICount].m_Position[1] = randf();
	BuildTestBrain(pVehicle, 8);
	pVehicle->mMaxSpeed = randf(0.8f, 1.0f);
	m_AI[m_AICount++] = m_Engine.AddEntity(pVehicle);
}

void Demo::CreateDemoThree() {
	m_DemoName = "Light Seeking with Collision Avoidance";
	ClearAll();
	DemoLight* pLight = new DemoLight(&m_State[m_AICount]);
	m_State[m_AICount].m_Kind = kLight;
	m_State[m_AICount].m_Rotation = k0;
	m_State[m_AICount].m_Position[0] = kHalf;
	m_State[m_AICount].m_Position[1] = kHalf;
	m_AI[m_AICount++] = m_Engine.AddEntity(pLight);

	for (int i = 0; i < 2; ++i) {
		CreateLightSeekingAvoider();
	}
}

void Demo::Reset() {
	ClearAll();
	mDemoMode = kNone;
	mPotentialPick = -1;
	mCurrentPick = -1;
}

bool Demo::HandleKey(int key) {
	bool handled = false;
	int i;

	switch (key) {
		case SDLK_SPACE:
			handled = true;
			Reset();
			++mCurrentDemo;
			if (mCurrentDemo > MAXDEMO)
				mCurrentDemo = 0;

			switch (mCurrentDemo) {
		case 0:		CreateDemoZero_Zero();		break;
		case 1:		CreateDemoZero_One();		break;
		case 2:		CreateDemoZero_Two();		break;
		case 3:		CreateDemoZero_Three();		break;
		case 4:		CreateDemoOne();			break;
		case 5:		CreateDemoTwo_Zero();		break;
		case 6:		CreateDemoTwo();			break;
		case 7:		CreateDemoThree();			break;
			}

			AddAllProxies();
			break;

		case SDLK_h:
			handled = true;
			showBrains = !showBrains;
			break;

		case SDLK_PAGEUP:
			// double the number of entities each time
			if (mCurrentDemo == 7) {
				int count = m_Engine.GetEntityCount() - 1; // subtract 1 for the sun
				if (count < 100) {
					for (i = 0; i < count; ++i) {
						CreateLightSeekingAvoider();
					}

					RemoveAllProxies();
					AddAllProxies();
				}
			}
			break;
	}
	return handled;
}

void Demo::DragEntity(int id) {
	PMath::Vec3f pos;
	ConvertWindowCoordsToOrthoGL(mMousex, mMousey, pos[0], pos[1]);
	pos[2] = k0;
	PMath::Vec3fSet(m_State[id].m_Position, pos);
}

void Demo::ChoosePotentialPick() {
	float x, y;
	ConvertWindowCoordsToOrthoGL(mMousex, mMousey, x, y);
	mPotentialPick = FindClosestEntity(x, y, 0.1f);
}

void Demo::MouseMotion(int x, int y) {
	mMousex = (float) x; 
	mMousey = (float) y;

	if (mDemoMode == kDragging) {
		DragEntity(mCurrentPick);
	}
}

void Demo::MouseClick(eMouseButton button) {
	switch (button) {
		case eLeft:
			if (mPotentialPick != -1) {
				mDemoMode = kDragging;
				mCurrentPick = mPotentialPick;
			}
			break;
	}
}

void Demo::MouseUnclick(eMouseButton button) {
	switch (button) {
		case eLeft:
			mDemoMode = kNone;
			mCurrentPick = -1;
			break;
	}
}

void Demo::WrapAround(float left, float right, float bottom, float top) {
	for (int i = 0; i < m_AICount; ++i) {
		PhysState* pState = &m_State[i];

		if (pState->m_Position[0] > right)			pState->m_Position[0] = left;
		else if (pState->m_Position[0] < left)		pState->m_Position[0] = right;
		if (pState->m_Position[1] > top)			pState->m_Position[1] = bottom;
		else if (pState->m_Position[1] < bottom)	pState->m_Position[1] = top;
	}
}

void Demo::AddAllProxies() {
	if (m_pNN) {
		for (int i = 0; i < m_AICount; ++i) {
			m_pNN->AddProxy(&m_State[i]);
		}
	}
}

void Demo::RemoveAllProxies() {
	if (m_pNN) {
		for (int i = 0; i < m_AICount; ++i) {
			m_pNN->RemoveProxy(&m_State[i]);
		}
	}
}

void Demo::SetWindowSize(int width, int height, bool fullScreen) {
	VOpenGLMain::SetWindowSize(width, height, fullScreen);				// mRight and mTop will get set
	PMath::Vec3f origin;
	origin[0] = origin[1] = origin[2] = k0;
	PMath::Vec3f dimensions;
	dimensions[0] = dimensions[1] = dimensions[2] = 2.5f;

	RemoveAllProxies();
	delete m_pNN;
	m_pNN = new NearestNeighbours(origin, dimensions, 10, 10, 10);
	AddAllProxies();
}


void Demo::Update(float dt) {
	ClearWindow();
	RenderEntities();
	DrawUserPrompts(m_DemoName, "click to drag", (mCurrentDemo != 7) ? "keys: h, space" : "keys: h, space, pageUp");

	ChoosePotentialPick();

	if (mCurrentPick == -1 && mPotentialPick != -1) {
		HighlightEntity(mPotentialPick, 1.0f, 0.7f, 0.2f, 0.2f);		// highlighted object
	}

	if (mCurrentPick != -1) {
		HighlightEntity(mCurrentPick, 1.1f, 0.7f, 0.7f, 0.7f);		// highlighted object
	}

	m_Engine.UpdateEntities(dt, this);

	MoveEntities();

	WrapAround(0.0f, mRight, 0.0f, mTop);
}

static void RenderCollisionSensor(CollisionSensor* pSensor) { 
	glDisable(GL_TEXTURE_2D);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(0.1f, 0.05f, k0);

	// draw the collision activation
	glScalef(0.015f, 0.015f, 0.015f);
	glColor3f(pSensor->mActivation, pSensor->mActivation, 0.0f);
	glBegin(GL_TRIANGLE_FAN);
	for (float f = 0.0f; f < kPi; f += kPi / 8.0f) {
		glVertex2f(cosf(f), sinf(f));
	}
	glEnd();

	// draw the sensor
	glColor3f(0.7f,0.7f, 0.3f);
	glBegin(GL_LINE_STRIP);
	for (float f = 0.0f; f < kPi; f += kPi / 8.0f) {
		glVertex2f(cosf(f), sinf(f));
	}
	glVertex2f(1.0f, 0.0f);
	glEnd();

	glColor3f(1,0,0);
	glTranslatef(0,0.5f,0);
	glRotatef(90.0f, 0,0,1);
	glScalef(0.5f,0.5f,0.5f);
	DrawDart();
	glPopMatrix();
}


static void RenderLightSensor(LightSensor* pLS) {
	glDisable(GL_TEXTURE_2D);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(0.05f, 0.05f, 0);

	if (pLS->mbDirectional) {
		// draw steering activation
		glColor3f(0.7f, 0.7f, 0.3f);
		glBegin(GL_LINES);
		glVertex2f(0.0f, 0.0f);
		glVertex2f(pLS->mSteeringActivation * 0.05f, 0.0f);
		glEnd();

		// draw the light activation
		glScalef(0.015f, 0.015f, 0.015f);
		glColor3f(pLS->mActivation, pLS->mActivation, 0.0f);
		glBegin(GL_TRIANGLE_FAN);
		for (float f = 0.0f; f < kPi; f += kPi / 8.0f) {
			glVertex2f(cosf(f), sinf(f));
		}
		glEnd();

		// draw the sensor
		glColor3f(0.7f,0.7f, 0.3f);
		glBegin(GL_LINE_STRIP);
		for (float f = 0.0f; f < kPi; f += kPi / 8.0f) {
			glVertex2f(cosf(f), sinf(f));
		}
		glVertex2f(1.0f, 0.0f);
		glEnd();
	}
	else {
		glScalef(0.03f, 0.03f, 0.03f);
		glColor3f(pLS->mActivation, pLS->mActivation, 0.0f);
		glBegin(GL_QUADS);
		glVertex2f(-0.5f, 0.2f);
		glVertex2f(+0.5f, 0.2f);
		glVertex2f(+0.5f, -0.2f);
		glVertex2f(-0.5f, -0.2f);
		glVertex2f(-0.5f, 0.2f);
		glEnd();

		glColor3f(0.7f,0.7f, 0.3f);
		glBegin(GL_LINE_STRIP);
		glVertex2f(-0.5f, 0.2f);
		glVertex2f(+0.5f, 0.2f);
		glVertex2f(+0.5f, -0.2f);
		glVertex2f(-0.5f, -0.2f);
		glVertex2f(-0.5f, 0.2f);
		glEnd();
	}

	glPopMatrix();
}

void RenderFunction(Function* pFunction) {
	glDisable(GL_TEXTURE_2D);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(0.05f, k0, k0);
	glScalef(0.02f, 0.02f, 0.02f);
	glColor3f(0.0f, 0.0f, 0.0f);
	DrawFilledCircle();
	glColor3f(1.0f, 1.0f, 1.0f);
	DrawCircle();

	switch (pFunction->mFunction) {
			case Function::kBuffer:
				glBegin(GL_LINES);
				glVertex2f(-0.5f, 0.2f);
				glVertex2f(+0.5f, 0.2f);
				glVertex2f(-0.5f, -0.2f);
				glVertex2f(+0.5f, -0.2f);
				glEnd();
				break;
			case Function::kInvert:
				glBegin(GL_LINES);
				glVertex2f(-0.5f, 0.5f);
				glVertex2f(+0.5f, -0.5f);
				glEnd();
				break;
			case Function::kSigmoid:
				glBegin(GL_LINE_STRIP);
				glVertex2f(0.5f, 0.5f);
				glVertex2f(0.1f, 0.4f);
				glVertex2f(0.0f, 0.0f);
				glVertex2f(-0.1f, -0.4f);
				glVertex2f(-0.5f, -0.5f);
				glEnd();
				break;
	}

	glTranslatef(0.0f, -0.9f, 0.0f);
	if (pFunction->mActivation >= 0.0f) {
		glColor3f(pFunction->mActivation, pFunction->mActivation, 0.0f);	// yellow for positive activation
	}
	else {
		glColor3f(pFunction->mActivation, 0.0f, 0.0f);			// red for positive activation
	}

	glBegin(GL_QUADS);
	glVertex2f(-0.5f, 0.2f);
	glVertex2f(+0.5f, 0.2f);
	glVertex2f(+0.5f, -0.2f);
	glVertex2f(-0.5f, -0.2f);
	glVertex2f(-0.5f, 0.2f);
	glEnd();

	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_LINE_STRIP);
	glVertex2f(-0.5f, 0.2f);
	glVertex2f(+0.5f, 0.2f);
	glVertex2f(+0.5f, -0.2f);
	glVertex2f(-0.5f, -0.2f);
	glVertex2f(-0.5f, 0.2f);
	glEnd();

	glPopMatrix();
}

void RenderSwitch(InsectAI::Switch* pSwitch) {
	glDisable(GL_TEXTURE_2D);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(0.05f, k0, k0);
	glScalef(0.02f, 0.02f, 0.02f);
	glColor3f(0.0f, 0.0f, 0.0f);
	DrawFilledCircle();
	glColor3f(1.0f, 1.0f, 1.0f);
	DrawCircle();

	// draw the switch
	glBegin(GL_LINES);
	glVertex2f((pSwitch->mpSwitch->mActivation <= 0.5f) ? -0.5f : 0.5f, 0.5f);
	glVertex2f(+0.0f, -0.5f);
	glEnd();

	glTranslatef(0.0f, -0.9f, 0.0f);
	if (pSwitch->mActivation >= 0.0f) {
		glColor3f(pSwitch->mActivation, pSwitch->mActivation, 0.0f);	// yellow for positive activation
	}
	else {
		glColor3f(pSwitch->mActivation, 0.0f, 0.0f);			// red for positive activation
	}

	glBegin(GL_QUADS);
	glVertex2f(-0.5f, 0.2f);
	glVertex2f(+0.5f, 0.2f);
	glVertex2f(+0.5f, -0.2f);
	glVertex2f(-0.5f, -0.2f);
	glVertex2f(-0.5f, 0.2f);
	glEnd();

	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_LINE_STRIP);
	glVertex2f(-0.5f, 0.2f);
	glVertex2f(+0.5f, 0.2f);
	glVertex2f(+0.5f, -0.2f);
	glVertex2f(-0.5f, -0.2f);
	glVertex2f(-0.5f, 0.2f);
	glEnd();

	glPopMatrix();
}


static void RenderSensor(InsectAI::Sensor* pSensor) {
	if (pSensor->GetKind() == LightSensor::GetStaticKind())			RenderLightSensor((LightSensor*) pSensor);
	else	if (pSensor->GetKind() == Switch::GetStaticKind())				RenderSwitch((Switch*) pSensor);
	else	if (pSensor->GetKind() == CollisionSensor::GetStaticKind())		RenderCollisionSensor((CollisionSensor*) pSensor);
	else	if (pSensor->GetKind() == Function::GetStaticKind())			RenderFunction((Function*) pSensor);
}

static void RenderActuator(Actuator* pActuator) {
	glDisable(GL_TEXTURE_2D);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(0.05f, -0.05f, k0);
	glScalef(0.02f, 0.02f, 0.02f);
	glColor3f(0.0f, 0.0f, 0.0f);
	DrawFilledCircle();
	glColor3f(1.0f, 1.0f, 1.0f);
	DrawCircle();

	// draw an M for motor
	glBegin(GL_LINE_STRIP);
	glVertex2f(0.5f,  -0.5f);
	glVertex2f(0.5f,   0.5f);
	glVertex2f(0.0f,  -0.3f);
	glVertex2f(-0.5f,  0.5f);
	glVertex2f(-0.5f, -0.5f);
	glEnd();	

	glPopMatrix();
}

static void RenderActuatorConnections(InsectAI::Actuator* pActuator) {
	/*	if (pActuator->mpInput != 0) {
	glColor3f(0.7f, 0.0f, 0.0f);
	glBegin(GL_LINE_STRIP);
	glVertex2f(k0, k0);
	glVertex2f(k0, 0.05f);
	glEnd();
	}
	*/
}

static void RenderSensorConnections(InsectAI::Sensor* pSensor) {
	/*	if (pSensor->GetKind() == Switch::GetStaticKind()) {
	Switch* pSwitch = (Switch*) pSensor;

	float x = pSwitch->m_Position[0];
	float y = pSwitch->m_Position[1];

	glColor3f(0.7f, 0.0f, 0.0f);
	glBegin(GL_LINES);
	glVertex2f(pSwitch->mpA->m_Position[0],	pSwitch->mpA->m_Position[1]);
	glVertex2f(x, y);
	glVertex2f(pSwitch->mpB->m_Position[0],	pSwitch->mpB->m_Position[1]);
	glVertex2f(x, y);

	glColor3f(0.0f, 0.7f, 0.0f);
	glVertex2f(pSwitch->mpB->m_Position[0], pSwitch->mpB->m_Position[1]);
	glVertex2f(x + 0.025f, y);
	glVertex2f(x + 0.025f, y);
	glVertex2f(x, y);
	glEnd();
	}
	else if (pSensor->GetKind() == Function::GetStaticKind()) {
	Function* pFunction = (Function*) pSensor;
	int count = (int) pFunction->mInputs.size();
	glColor3f(0.7f, 0.0f, 0.0f);
	glBegin(GL_LINES);
	for (int i = 0; i < count; ++i ) {
	glVertex2f(pFunction->mInputs[i]->m_Position[0], pFunction->mInputs[i]->m_Position[1]);
	glVertex2f(pFunction->m_Position[0], pFunction->m_Position[1]);
	}
	glEnd();
	}
	*/
}

static void RenderVehicle(DemoVehicle* pVehicle, PhysState* pState) {
	if (pVehicle == 0)
		return;

	int i;
	glDisable(GL_TEXTURE_2D);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(pState->m_Position[0], pState->m_Position[1], k0);
	glPushMatrix();
	glScalef(0.02f, 0.02f, 0.02f);
	glRotatef(pState->m_Rotation * (-360.0f / (2.0f * kPi)), k0,k0,k1);
	glColor3f(0.7f,0.3f, 0.3f);
	DrawDart();
	glPopMatrix();

	if (showBrains) {
		/*			for (i = 0; i < pVehicle->GetSensorCount(); ++i) {
		RenderSensorConnections(pVehicle->m_Sensors[i]);
		}

		for (i = 0; i < pVehicle->GetActuatorCount(); ++i) {
		RenderActuatorConnections(pVehicle->m_Actuators[i]);
		}
		*/

		for (i = 0; i < pVehicle->GetSensorCount(); ++i) {
			RenderSensor(pVehicle->GetSensor(i));
		}

		for (i = 0; i < pVehicle->GetActuatorCount(); ++i) {
			RenderActuator(pVehicle->GetActuator(i));
		}
	}

	glPopMatrix();
}

static void RenderLight(PhysState const*const pState) {
	glDisable(GL_TEXTURE_2D);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(pState->m_Position[0], pState->m_Position[1], 0);
	glScalef(0.017f, 0.017f, 0.017f);
	glRotatef(pState->m_Rotation, 0,0,1);
	glColor3f(0.8f,0.8f, 0.0f);
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f( 0.0f, 0.0f);
	bool tog = true;
	for (float i = 0; i < kPi * 2.0f; i += kPi / 10.0f) {
		float scale = tog ? 1.0f : 0.75f;
		tog = !tog;
		glVertex2f(scale * cosf(i), scale * sinf(i));
	}
	glVertex2f(1.0f, 0.0f);	// close the sun
	glEnd();
	glPopMatrix();
}



void Demo::RenderEntities()
{
	for (int i = 0; i < m_AICount; ++i) {
		// render agent here
		if (m_State[i].m_Kind == kVehicle) {
			RenderVehicle(m_State[i].m_Vehicle, &m_State[i]);
		}
		else if (m_State[i].m_Kind == kLight) {
			RenderLight(&m_State[i]);
		}
	}
}

void Demo::HighlightEntity(int id, float radius, float red, float green, float blue)
{
	radius *= 0.02f;
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(m_State[id].m_Position[0], m_State[id].m_Position[1], 0);
	glScalef(radius, radius, radius);
	glColor3f(red, green, blue);
	DrawCircle();
	glPopMatrix();
}

const float kSteeringSpeed = 0.25f;


static void MoveVehicle(DemoVehicle* pVehicle, PhysState* pState)
{
	// connect actuators to physics
	for (int i = 0; i < pVehicle->GetActuatorCount(); ++i) {
		Actuator* pActuator = pVehicle->GetActuator(i);
		switch (pActuator->GetKind()) {
			case Actuator::kMotor:
				pState->m_Rotation += kSteeringSpeed * pActuator->mSteeringActivation;
				//mRotation = 0.25f * kPi;
				if (pState->m_Rotation < 0.0f) pState->m_Rotation += 2.0f * kPi;
				else if (pState->m_Rotation > 2.0f * kPi) pState->m_Rotation -= 2.0f * kPi;

				float activation = 0.005f * pActuator->mActivation;
				pState->m_Position[0] += pVehicle->mMaxSpeed * sinf(pState->m_Rotation) * activation;			// rotation of zero moves forward on y axis
				pState->m_Position[1] += pVehicle->mMaxSpeed * cosf(pState->m_Rotation) * activation;
				break;
		}
	}
}

void Demo::MoveEntities()
{
	for (int i = 0; i < m_AICount; ++i) {
		// render agent here
		if (m_State[i].m_Kind == kVehicle) {
			MoveVehicle(m_State[i].m_Vehicle, &m_State[i]);
			m_pNN->UpdateProxy(&m_State[i]);
		}
	}
}

int Demo::FindClosestEntity(float x, float y, float maxDistance)
{
	int best = -1;

	maxDistance *= maxDistance;
	float bestDistance = 1.0e7f;

	for (int i = 0; i < m_AICount; ++i) {
		float distSquared = m_State[i].DistanceSquared(x, y);

		if (distSquared < bestDistance && distSquared < maxDistance) {
			best = i;
			bestDistance = distSquared;
		}
	}

	return best;
}


void DrawUserPrompts(char* name, char* mouse, char* keys) 
{
	return;
#ifdef USE_GLFONT
	// draw user prompts
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(0.035f,0.075f,0);
	float scale = 1.0f/60.0f;
	glScalef(scale,scale,scale);
	glfSetCurrentFont( font );
	glColor3f( 1,1,1 );
	glfDrawWiredString(name);
	glTranslatef(0,-0.025f * (1.0f/scale),0);
	glColor3f( 0.7f, 0.7f, 1.0f );
	glfDrawWiredString(mouse);
	glTranslatef(0,-0.025f * (1.0f/scale),0);
	glfDrawWiredString(keys);
	glPopMatrix();
#endif
}



int main(int argc, char **argv) 
{  
	int done = 0;
	TimeVal prevTime;
	TimeVal newTime;

	fprintf(stderr, "Starting Insect AI demo\n");

	// Initialize SDL for video output
	if ( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0 ) {
		fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}

	fprintf(stderr, "SDL initialized\n");

	Clock myClock;
	myClock.update();
	prevTime = myClock.getSimulationTime();

	int width = 800;
	int height = 800;

	Demo* pDemo = new Demo();
	pDemo->SetWindowName("Demo of an ALife Architecture - by Nick Porcino");
	pDemo->SetWindowSize(width, height, false);
	pDemo->Reset();
	pDemo->CreateDefaultDemo();

#ifdef USE_GLFONT
	glfInit();
	font = glfLoadFont( "courier1.glf");
	fprintf(stderr, "glf initialized\n");
#endif

	while ( !done ) {
		myClock.update();
		newTime = myClock.getSimulationTime();
		float dt = (float)(newTime - prevTime);
		prevTime = newTime;

		pDemo->Update(dt);

		done = MeshulaApp::PollEvents(*pDemo);
		SDL_GL_SwapBuffers();
	}

	SDL_Quit();
	return 1;
}


InsectAI::DynamicState* Demo::GetNearest(InsectAI::Entity* pE, uint32 filter)
{
	InsectAI::DynamicState* pRetVal = 0;
	if (filter != 0) {
		Real nearest = 1.0e6f;
		Real radius;

		PhysState* pState = (PhysState*) pE->GetDynamicState();

		// if it's a light, simply search the entire database for the closest light
		// (lq is not that fast when the search radius is similar to the size of the database)
		if ((filter & kLight) != 0) {
			for (int i = 0; i < m_AICount; ++i) {
				if ((m_State[i].m_Kind & filter) != 0) {
					if (m_State[i].m_Vehicle != pE) {
						Real distSquared = pState->DistanceSquared(&m_State[i]);
						if (distSquared < nearest) {
							nearest = distSquared;
							pRetVal = &m_State[i];
						}
					}
				}
			}
		}
		else {
			radius = 0.15f;	// this should really account for 2 * maximum velocity of a bug
			PhysState* pNearest = (PhysState*) m_pNN->FindNearestNeighbour(pState->GetPosition(), radius, filter, pState);
			return pNearest;

			// brute force search for comparison
			for (int i = 0; i < m_AICount; ++i) {
				if ((m_State[i].m_Kind & filter) != 0) {
					if (m_State[i].m_Vehicle != pE) {
						Real distSquared = pState->DistanceSquared(&m_State[i]);
						if (distSquared < nearest) {
							nearest = distSquared;
							pRetVal = &m_State[i];
						}
					}
				}
			}
			return pNearest;
		}

	}
	return pRetVal;
}
