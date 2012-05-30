

#include "Stdafx.h"
#include "GameSystem.h"

//-------------------------------------------------------------------------------------
GameSystem::GameSystem(void)
	: collisionDetector(0),
	  colissionDelay(0.0f),
	  character(0),
	  cloud(0),
	  mCameraListener(0),
	  exCamera(0)
{
}

//-------------------------------------------------------------------------------------
GameSystem::~GameSystem(void)
{
	if(collisionDetector != 0)	delete collisionDetector;
	if(character != 0)			delete character;
	if(cloud != 0)				delete cloud;
	if(mCameraListener != 0)	delete mCameraListener;
	if(exCamera != 0)			delete exCamera;

}

//-------------------------------------------------------------------------------------
/**  Create your scene here */
void GameSystem::createScene(void)
{
	mSceneMgr->setSkyBox(true, "Sky/Clouds", 10000, true);

	exCamera = new ThirdPersonCamera("ThirdPersonCamera", mSceneMgr, mCamera);
	mCameraListener = new CameraListener(mWindow, mCamera);
	mCameraListener->setExtendedCamera(exCamera);
	this->character = new Character();
	mCameraListener->setCharacter(character);

	this->character->setup(mSceneMgr, Ogre::Vector3(0, 3000, 3000), Ogre::Vector3(0.5f, 0.5f, 0.5f), Ogre::Quaternion::IDENTITY);
	this->character->setGravity(9.8f);

	cloud = new SimpleCloud();
	cloud->initCloud(mSceneMgr, 60);
}

//-------------------------------------------------------------------------------------
/** set some variables needed later */
void GameSystem::initSystem(Ogre::Root *mRoot, 
	Ogre::Camera* mCamera, 
	Ogre::SceneManager* mSceneMgr, 
	OIS::Mouse* mMouse, 
	OIS::Keyboard* mKeyboard, 
	Ogre::RenderWindow* mWindow, 
	GalaxyEngine::Core *planetEngine)
{
	this->mRoot = mRoot;
	this->mCamera = mCamera;
	this->mSceneMgr = mSceneMgr;
	this->mMouse = mMouse;
	this->mKeyboard = mKeyboard;
	this->planetEngine = planetEngine;
	this->mWindow = mWindow;

	collisionDetector = new RayCastCollision();
	collisionDetector->init(this->mSceneMgr);
}

//------------------------------------------------------------------------------------
/** Updating every frameRenderingQueued */
void GameSystem::update(Ogre::Real elapsedTime)
{
	checkPlanetColission(elapsedTime);
	character->update(elapsedTime);
	mCameraListener->update(elapsedTime);
	cloud->updateClouds(elapsedTime);
}

//------------------------------------------------------------------------------------
void GameSystem::checkPlanetColission(Ogre::Real timeElapsed)
{
	colissionDelay += timeElapsed;
	if(colissionDelay < 0.0166f) return;		// 60 fps
	colissionDelay = 0;							// reset
		
	Ogre::Vector3 charPos = character->getBodyNode()->_getDerivedPosition();
	Ogre::Real distance1 = Ogre::Math::Abs(Ogre::Vector3::ZERO.distance(charPos));	// distance of the position to center of planet
	GalaxyEngine::Planet* planet = planetEngine->getFirstPlanet();					// we only have one planet

	if(planet == 0) return;

	Ogre::Real radius = planet->getBoundingRadius();								// radius of bounding of the planet

	if(distance1 < radius)
	{		
		Ogre::Vector3 result = Ogre::Vector3::ZERO;		// resulted intersection
		Ogre::Vector3 camToCenter = -charPos;			// direction to the center
		camToCenter.normalise();						
		collisionDetector->getPlanetIntersection(planet, charPos, camToCenter, result);
		Ogre::Real distance2 = Ogre::Math::Abs(Ogre::Vector3::ZERO.distance(result));
		Ogre::Real halfRadius = radius * 0.5f;

		if(distance2 < (radius - halfRadius)  || distance2 > (radius + halfRadius))		
		{
			// don't know why but sometimes it's invalid
		}
		else if(distance1 < (distance2 + 5.0f))
		{
			character->setLanding();
		}

	}
	
}

//-------------------------------------------------------------------------------------
void GameSystem::keyPressed( const OIS::KeyEvent &arg )
{
	if(arg.key == OIS::KC_W)
	{
		character->setState(Movement::MOVE_FRONT);
	}	
	else if(arg.key == OIS::KC_S)
	{
		character->setState(Movement::MOVE_BACK);
	}
	else if(arg.key == OIS::KC_A)
	{
		character->setState(Movement::MOVE_LEFT);
	}
	else if(arg.key == OIS::KC_D)
	{
		character->setState(Movement::MOVE_RIGHT);
	}
	else if(arg.key == OIS::KC_Q)
	{
		character->setState(Movement::ROTATE_LEFT);
	}
	else if(arg.key == OIS::KC_E)
	{
		character->setState(Movement::ROTATE_RIGHT);
	}
}

//-------------------------------------------------------------------------------------
void GameSystem::keyReleased( const OIS::KeyEvent &arg )
{
	if(arg.key == OIS::KC_W ||
		arg.key == OIS::KC_S ||
		arg.key == OIS::KC_A ||
		arg.key == OIS::KC_D ||
		arg.key == OIS::KC_Q ||
		arg.key == OIS::KC_E)
	{
		character->setState(Movement::NOTHING);
	}
}

