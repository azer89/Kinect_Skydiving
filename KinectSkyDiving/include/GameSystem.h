
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
#include "GameConfig.h"
#include "GGBirdLoader.h"
#include "GGBird.h"
#include "OgreKinect.h"
#include "PPSoundManager.h"

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

	inline bool getlandingStatus(void) { return isLanding;}
	inline bool checkPlanetInitialization(void){ return isPlanetInitialized; }
	inline void startGame(void) { isGameStarted = true; mPPSoundManager->playMusic("skydiving_gamemusic_loop_new_2..30min.mp3"); }
	inline Ogre::Real getPercentAltitude(void) { return percentAltitude; }
	inline int getNumAttacked(void) { return numAttacked; }

	int getScore(void){ return character->getGameplayScore();}		
	bool isUpsideTarget();
	Ogre::Real getArrowDirection();
	Ogre::Vector2 getWristPosition(void){ return mOgreKinect->getWristPosition(); }
	bool isSkeletonTracked(void) { return mOgreKinect->isTracking; }

private:
	/** Check planet collision, similar to RaySceneQuery */
	void checkPlanetColission(Ogre::Real timeElapsed);
	/** Called once when planet is initialized, since it takes a while for creating planet after the program starts */
	void postPlanetInitialization(); 
	/** Check if planet is fully created */
	void isPlanetReady();
	/** Open the parachute */
	void openParachute();
	/** Pose detection by kinect */
	void processKinectInput(Ogre::Real elapsedTime);

private:
	RayCastCollision*   rayCollisionDetector;		// simple planet collision engine	
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
	GGBirdLoader*		ggBirdLoader;
	PPSoundManager*		mPPSoundManager;
	OgreKinect*			mOgreKinect;	

	bool bStopFalling;
	GGBirdFactory* mGGBirds;

	Ogre::Vector3	targetPosition;
	Ogre::Vector3	originalPosition;
	Ogre::Real		originalDistance;
	Ogre::Vector3	prevCharacterPosition;
	Ogre::Real		distancePercentage;

	Ogre::Real      collisionDelay;				// collision engine isn't optimized so make it run every 1/60 second
	bool			isPlanetInitialized;		// is planet is fully initialized?
	bool			isGameStarted;
	bool			isLanding;
	bool			isKinectActive;

	Ogre::Real		openParacuteDelay;
	Ogre::Real		openParachuteCounter;
	Ogre::Real		currentAltitude;
	Ogre::Real		percentAltitude;
	int				numAttacked;

	Ogre::Root*				mRoot;
	Ogre::Camera*			mCamera;
	Ogre::SceneManager*		mSceneMgr;
	OIS::Mouse*				mMouse;
	OIS::Keyboard*			mKeyboard;
	Ogre::RenderWindow*		mWindow;
	Ogre::AnimationState*	mAni;
};

#endif // #ifndef __GameSystem_h_

