
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
#include "SimpleCloud.h"
#include "PlanetObjects.h"
#include "TargetCircles.h"
#include "ParticleManager.h"
#include "LoadingAnimation.h"
#include "CollisionDetector.h"

//#include "PPSoundManager.h"
#include "GGBird.H"
#include "OgreKinect.h"

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
		GalaxyEngine::Core* planetEngine,
		LoadingAnimation* mLoadingBar);

	void createScene(void);
	void update(Ogre::Real elapsedTime);

	void keyPressed( const OIS::KeyEvent &arg );
	void keyReleased( const OIS::KeyEvent &arg );

private:
	/** Check planet collision, similar to RaySceneQuery */
	void checkPlanetColission(Ogre::Real timeElapsed);
	/** Called once when planet is initialized, since it takes a while for creating planet after the program starts */
	void postPlanetInitialization(); 
	/** Check if planet is fully created */
	void isPlanetReady();

private:
	RayCastCollision*   rayCollisionDetector;			// simple planet collision engine	
	GalaxyEngine::Core* planetEngine;				// planet rendering engine
	Character*          character;					// character
	PlanetObjects*		pObjects;
	TargetCircles*		tCircles;
	SimpleCloud*		cloud;
	ParticleManager*	pManager;
	CameraListener*     mCameraListener;
	ThirdPersonCamera*  exCamera;
	LoadingAnimation*	mLoadingBar;
	CollisionDetector*	collisionDetector;

	OgreKinect* mOgreKinect;
	void processKinectInput();

	bool bStopFalling;
	GGBirdFatory* mGGBirds;
	//PPSoundManager* mPPSoundManager;

	Ogre::Real          collisionDelay;				// collision engine isn't optimized so make it run every 1/60 second
	bool				isPlanetInitialized;		// is planet is fully initialized?

	Ogre::Root*         mRoot;
	Ogre::Camera*       mCamera;
	Ogre::SceneManager* mSceneMgr;
	OIS::Mouse*         mMouse;
	OIS::Keyboard*      mKeyboard;
	Ogre::RenderWindow* mWindow;
};

#endif // #ifndef __GameSystem_h_

