
#ifndef __GameSystem_h_
#define __GameSystem_h_

#include "Stdafx.h"
#include "RayCastCollision.h"
#include "Core.h"
#include "Universe.h"
#include "Planet.h"
#include "Character.h"
#include "CameraListener.h"
#include "ThirdPersonCamera.h"


class GameSystem
{
public:
	GameSystem(void);
	virtual ~GameSystem(void);

	void initSystem(Ogre::Root* mRoot, 
		Ogre::Camera* mCamera, 
		Ogre::SceneManager* mSceneMgr, 
		OIS::Mouse* mMouse, 
		OIS::Keyboard* mKeyboard,
		Ogre::RenderWindow* mWindow,
		GalaxyEngine::Core* planetEngine);

	void createScene(void);
	void update(Ogre::Real elapsedTime);

	void keyPressed( const OIS::KeyEvent &arg );
	void keyReleased( const OIS::KeyEvent &arg );

private:
	/** Check planet collision, similar to RaySceneQuery */
	void checkPlanetColission(Ogre::Real timeElapsed);

private:
	RayCastCollision*   collisionDetector;			// simple planet collision engine
	Ogre::Real          colissionDelay;				// collision engine isn't optimized so make it run every 1/60 second
	GalaxyEngine::Core* planetEngine;				// planet rendering engine
	Character*          character;					// character
	CameraListener*     mCameraListener;
	ThirdPersonCamera*  exCamera;

	Ogre::Root*         mRoot;
	Ogre::Camera*       mCamera;
	Ogre::SceneManager* mSceneMgr;
	OIS::Mouse*         mMouse;
	OIS::Keyboard*      mKeyboard;
	Ogre::RenderWindow* mWindow;
};

#endif // #ifndef __GameSystem_h_

