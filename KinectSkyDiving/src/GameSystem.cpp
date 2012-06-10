
#include "Stdafx.h"
#include "GameSystem.h"

//-------------------------------------------------------------------------------------
GameSystem::GameSystem(void)
	: rayCollisionDetector(0),
	  collisionDetector(0),
	  collisionDelay(0.0f),
	  character(0),
	  cloud(0),
	  mCameraListener(0),
	  exCamera(0),
	  pObjects(0),
	  tCircles(0),
	  pManager(0),
	  isPlanetInitialized(false),
	  bStopFalling(false)
{
}

//-------------------------------------------------------------------------------------
GameSystem::~GameSystem(void)
{
	if(rayCollisionDetector != 0)	delete rayCollisionDetector;
	if(collisionDetector != 0)		delete collisionDetector;
	if(character != 0)				delete character;
	if(cloud != 0)					delete cloud;
	if(mCameraListener != 0)		delete mCameraListener;
	if(exCamera != 0)				delete exCamera;
	if(pObjects != 0)				delete pObjects;
	if(tCircles != 0)				delete tCircles;
	if(pManager != 0)				delete pManager;
}

//-------------------------------------------------------------------------------------
/**  Create your scene here */
void GameSystem::createScene(void)
{
	mSceneMgr->setSkyBox(true, "Sky/Space", 10000, true);

	exCamera = new ThirdPersonCamera("ThirdPersonCamera", mSceneMgr, mCamera);
	mCameraListener = new CameraListener(mWindow, mCamera);
	mCameraListener->setExtendedCamera(exCamera);
	this->character = new Character();
	mCameraListener->setCharacter(character);

	this->character->setup(mSceneMgr, 
						   Ogre::Vector3(0, 5750, 4750), 
						   Ogre::Vector3(0.5f, 0.5f, 0.5f), 
						   Ogre::Quaternion::IDENTITY);
	this->character->setGravity(8.0f);

	mLoadingBar->update();
	
	mGGBirds = new GGBirdFatory();					// GGBird
	//mPPSoundManager = new PPSoundManager();		// Sound	
	mOgreKinect = new OgreKinect(mSceneMgr);		// Kinect

	mLoadingBar->update();

	cloud = new SimpleCloud();
	cloud->initCloud(mSceneMgr, 60);

	mLoadingBar->update();
}

//-------------------------------------------------------------------------------------
/** set some variables needed later */
void GameSystem::initSystem(Ogre::Root *mRoot, 
	Ogre::Camera* mCamera, 
	Ogre::SceneManager* mSceneMgr, 
	OIS::Mouse* mMouse, 
	OIS::Keyboard* mKeyboard, 
	Ogre::RenderWindow* mWindow, 
	GalaxyEngine::Core *planetEngine,
	LoadingAnimation* mLoadingBar)
{
	this->mRoot = mRoot;
	this->mCamera = mCamera;
	this->mSceneMgr = mSceneMgr;
	this->mMouse = mMouse;
	this->mKeyboard = mKeyboard;
	this->planetEngine = planetEngine;
	this->mWindow = mWindow;
	this->mLoadingBar = mLoadingBar;
}

//------------------------------------------------------------------------------------
/** Updating every frameRenderingQueued */
void GameSystem::update(Ogre::Real elapsedTime)
{
	if(!isPlanetInitialized) isPlanetReady();
	checkPlanetColission(elapsedTime);
	character->update(elapsedTime);
	mCameraListener->update(elapsedTime);
	cloud->updateClouds(elapsedTime);

	mGGBirds->Update(elapsedTime, character->getBodyNode()->_getDerivedPosition());
	mOgreKinect->update(elapsedTime);
	//processKinectInput();

	if(pManager != 0) pManager->update(character->getBodyNode()->_getDerivedPosition());
	if(collisionDetector != 0) collisionDetector->update(elapsedTime);
}

//------------------------------------------------------------------------------------
void GameSystem::checkPlanetColission(Ogre::Real timeElapsed)
{
	collisionDelay += timeElapsed;
	if(collisionDelay < 0.0166f) return;		// 60 fps
	collisionDelay = 0;							// reset

	if(rayCollisionDetector == 0) return;			// collision detector isn't ready yet
		
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
		rayCollisionDetector->getPlanetIntersection(charPos, camToCenter, result);
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
	if (arg.key == OIS::KC_P)
		bStopFalling = !bStopFalling;
	if (arg.key == OIS::KC_G)
	{
		Ogre::Vector3 vecA = character->getWorldPosition();
		Ogre::Vector3 vecUP = character->getWorldPosition().normalisedCopy() * 5;
		Ogre::Vector3 vecRand(rand() % 24 - 12, rand() % 24 - 12, rand() % 24 - 12 );
		while (vecA.distance(vecA - vecUP + vecRand) < 8)
		{
			vecRand = Ogre::Vector3(rand() % 24 - 12, rand() % 24 - 12, rand() % 24 - 12 );
		}
		mGGBirds->addBird(mSceneMgr, vecA - vecUP + vecRand);
	}

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

//-------------------------------------------------------------------------------------
void GameSystem::postPlanetInitialization()
{
	GalaxyEngine::Planet* planet = planetEngine->getFirstPlanet();	

	rayCollisionDetector = new RayCastCollision();
	rayCollisionDetector->init(this->mSceneMgr, planet);
	rayCollisionDetector->crawlBaseChunks();

	mLoadingBar->update();

	pObjects = new PlanetObjects();
	pObjects->setup(mSceneMgr, rayCollisionDetector);

	mLoadingBar->update();

	tCircles = new TargetCircles();
	tCircles->setup(mSceneMgr);

	collisionDetector = new CollisionDetector();
	collisionDetector->initCollisionDetector(this->character, this->tCircles);

	mLoadingBar->update();

	pManager = new ParticleManager();
	pManager->initParticle(mSceneMgr);

	mLoadingBar->finish();
}

//-------------------------------------------------------------------------------------
void GameSystem::isPlanetReady()
{
	GalaxyEngine::Planet* planet = planetEngine->getFirstPlanet();	
	//std::cout << "~isPlanetReady~";
	if(planet != 0)
	{
		Ogre::SceneNode* parentNode = planet->getParentSceneNode();
		if(parentNode == NULL) 
		{
			//std::cout << "~parent not found~\n";
		}
		else
		{
			/** Now the planet is fully initialized */
			isPlanetInitialized = true;
			postPlanetInitialization();
		}
	}
}


void GameSystem::processKinectInput()
{

	if(mOgreKinect->mPoseDetect->isPose("front"))
	{
		//printf("front\n");
		character->setState(Movement::MOVE_FRONT);
	}	
	else if(mOgreKinect->mPoseDetect->isPose("back"))
	{
		character->setState(Movement::MOVE_BACK);
	}
	else if(mOgreKinect->mPoseDetect->isPose("left"))
	{
		//printf("left\n");
		character->setState(Movement::MOVE_LEFT);
	}
	else if(mOgreKinect->mPoseDetect->isPose("right"))
	{
		//printf("right\n");
		character->setState(Movement::MOVE_RIGHT);
	}
	else if(mOgreKinect->mPoseDetect->isPose("rotate_left"))
	{
		//printf("rotate_left\n");
		character->setState(Movement::ROTATE_LEFT);
	}
	else if(mOgreKinect->mPoseDetect->isPose("rotate_right"))
	{
		//printf("rotate_right\n");
		character->setState(Movement::ROTATE_RIGHT);
	}
}

