

#include "Stdafx.h"
#include "GameSystem.h"

//-------------------------------------------------------------------------------------
GameSystem::GameSystem(void)
	: collisionDetector(0),
	  colissionDelay(0.0f)
{
}

//-------------------------------------------------------------------------------------
GameSystem::~GameSystem(void)
{
	if(collisionDetector != 0) delete collisionDetector;
}

//-------------------------------------------------------------------------------------
/**  Create your scene here */
void GameSystem::createScene(void)
{
	mCamera->setPosition(0, 550.0f, 550);
	mCamera->lookAt(0, 350, 0);
	Ogre::Quaternion q;
	//q.FromAngleAxis(Ogre::Degree(-90), Ogre::Vector3::UNIT_X);
	//mCamera->setOrientation(q);
	mCamera->setNearClipDistance(0.001f);
	mCamera->setFarClipDistance(1000.0f);

	this->character = new Character();
	this->character->setup(mSceneMgr, Ogre::Vector3(0, 525, 525), Ogre::Vector3(0.5f, 0.5f, 0.5f), Ogre::Quaternion::IDENTITY);
}

//-------------------------------------------------------------------------------------
/** set some variables needed later */
void GameSystem::initSystem(Ogre::Root *mRoot, Ogre::Camera* mCamera, Ogre::SceneManager* mSceneMgr, OIS::Mouse* mMouse, OIS::Keyboard* mKeyboard, GalaxyEngine::Core *planetEngine)
{
	this->mRoot = mRoot;
	this->mCamera = mCamera;
	this->mSceneMgr = mSceneMgr;
	this->mMouse = mMouse;
	this->mKeyboard = mKeyboard;
	this->planetEngine = planetEngine;

	collisionDetector = new RayCastCollision();
	collisionDetector->init(this->mSceneMgr);
}

//------------------------------------------------------------------------------------
/** Updating every frameRenderingQueued */
void GameSystem::update(Ogre::Real elapsedTime)
{
	checkPlanetColission(elapsedTime);
	character->update(elapsedTime);
}

//------------------------------------------------------------------------------------
void GameSystem::checkPlanetColission(Ogre::Real timeElapsed)
{
	colissionDelay += timeElapsed;
	if(colissionDelay < 0.0166f) return;		// 60 fps
	colissionDelay = 0;							// reset

	Ogre::Vector3 camPos = mCamera->getDerivedPosition();
	Ogre::Real distance1 = Ogre::Math::Abs(Ogre::Vector3::ZERO.distance(camPos));	// distance of the position to center of planet
	GalaxyEngine::Planet* planet = planetEngine->getFirstPlanet();					// we only have one planet

	if(planet == 0) return;

	Ogre::Real radius = planet->getBoundingRadius();								// radius of bounding of the planet

	if(distance1 < radius)
	{		
		Ogre::Vector3 result = Ogre::Vector3::ZERO;		// resulted intersection
		Ogre::Vector3 camToCenter = -camPos;			// direction to the center
		camToCenter.normalise();						
		collisionDetector->getPlanetIntersection(planet, camPos, camToCenter, result);
		Ogre::Real distance2 = Ogre::Math::Abs(Ogre::Vector3::ZERO.distance(result));
		Ogre::Real halfRadius = radius * 0.5f;

		if(distance2 < (radius - halfRadius)  || distance2 > (radius + halfRadius))		// don't know why but sometimes it's invalid
		{
		}
		else if(distance1 < (distance2 + 2.0f))
		{
			Ogre::Vector3 pointUp = -camToCenter;
			mCamera->setPosition(camPos + (pointUp * 0.5f));
			//std::cout << distance2 << "\n";
		}

	}
}

