
#ifndef __GameSystem_h_
#define __GameSystem_h_

#include "Stdafx.h"
#include "RayCastCollision.h"
#include "Core.h"
#include "Universe.h"
#include "Planet.h"
#include "Character.h"

class GameSystem
{
public:
	GameSystem(void);
	virtual ~GameSystem(void);

	void initSystem(Ogre::Root *mRoot, 
		Ogre::Camera* mCamera, 
		Ogre::SceneManager* mSceneMgr, 
		OIS::Mouse* mMouse, 
		OIS::Keyboard* mKeyboard,
		GalaxyEngine::Core *planetEngine);

	void update(Ogre::Real elapsedTime);

private:
	/** Check planet collision, similar to RaySceneQuery */
	void checkPlanetColission(Ogre::Real timeElapsed);

private:
	RayCastCollision* collisionDetector;			// simple planet collision engine
	Ogre::Real colissionDelay;						// collision engine isn't optimized so make it run every 1/60 second
	GalaxyEngine::Core *planetEngine;				// planet rendering engine
	Character* character;							// character

	Ogre::Root *mRoot;
	Ogre::Camera* mCamera;
	Ogre::SceneManager* mSceneMgr;
	OIS::Mouse*    mMouse;
	OIS::Keyboard* mKeyboard;
};

#endif // #ifndef __GameSystem_h_

