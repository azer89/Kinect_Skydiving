
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
	  bStopFalling(false),
	  isGameStarted(false),
	  isLandingSuccess(false),
	  ggBirdLoader(0),
	  targetPosition(GameConfig::getSingletonPtr()->getTargetPosition()),
	  originalPosition(GameConfig::getSingletonPtr()->getCharacterPosition()),
	  numAttacked(0),
	  isLanding(false),
	  isKinectActive(GameConfig::getSingletonPtr()->getKinectStatus()),
	  openParacuteDelay(GameConfig::getSingletonPtr()->getOpenParachuteDelay()),
	  openParachuteCounter(0.0f),
	  mAni(NULL)
	  //distancePercentage(0.0f)
{
	originalDistance = originalPosition.distance(Ogre::Vector3::ZERO) - 5000;
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
	if(ggBirdLoader != 0)			delete ggBirdLoader;
}

//-------------------------------------------------------------------------------------
/**  Create your scene here */
void GameSystem::createScene(void)
{
	GameConfig* gameConfig = GameConfig::getSingletonPtr();

	mPPSoundManager = new PPSoundManager();

	mSceneMgr->setSkyBox(true, "Sky/Bright", 10000, true);

	exCamera = new ThirdPersonCamera("ThirdPersonCamera", mSceneMgr, mCamera);
	mCameraListener = new CameraListener(mWindow, mCamera);
	mCameraListener->setExtendedCamera(exCamera);

	this->character = new Character();
	this->character->setup(mSceneMgr, gameConfig->getCharacterPosition(), gameConfig->getCharacterScale(),  Ogre::Quaternion::IDENTITY);
	this->character->setGravity(gameConfig->getGravity());

	mCameraListener->setCharacter(character);
	mCameraListener->instantUpdate();

	mLoadingBar->update();
	
	mGGBirds = new GGBirdFactory();					// GGBird
	//mPPSoundManager = new PPSoundManager();		// Sound	
	mOgreKinect = new OgreKinect(mSceneMgr);		// Kinect

	mLoadingBar->update();

	cloud = new SimpleCloud();
	cloud->initCloud(mSceneMgr, 30);

	mLoadingBar->update();

	ggBirdLoader = new GGBirdLoader();
	ggBirdLoader->setup(mSceneMgr);
	std::vector<Ogre::SceneNode*> birds = ggBirdLoader->getNodeList();
	for(int a = 0; a < birds.size(); a++) { mGGBirds->addBird(mSceneMgr, birds[a]); }

	mLoadingBar->update();

	mPPSoundManager->playMusic("skydiving_menu.mp3");
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
	if(mPPSoundManager != 0) mPPSoundManager->update(elapsedTime);
	if(isKinectActive) { mOgreKinect->update(elapsedTime); }
	if(!isPlanetInitialized) isPlanetReady();
	if(!isGameStarted) return;

	prevCharacterPosition = character->getBodyNode()->getPosition();

	checkPlanetColission(elapsedTime);
	character->update(elapsedTime);
	mCameraListener->update(elapsedTime);
	cloud->updateClouds(elapsedTime);

	Ogre::Quaternion charOrient = character->getBodyNode()->getOrientation();
	Ogre::Vector3 upVector = charOrient * Ogre::Vector3::UNIT_Y;

	int numAtk = 0;

	if(!character->getParachuteStatus())	// free fall
	{
		numAtk = mGGBirds->Update(elapsedTime, character->getBodyNode()->_getDerivedPosition() - (upVector * 7.0));
	}
	else	// parachute opened
	{
		numAtk = mGGBirds->Update(elapsedTime, character->getBodyNode()->_getDerivedPosition() - (upVector * 1.5));
	}

	//if(numAtk != 0) std::cout << numAtk << "\n";
	numAttacked += numAtk;
	character->addScore(numAtk * -10);
	
	if(isKinectActive) { processKinectInput(elapsedTime); }

	if(pManager != 0) pManager->update(elapsedTime, character->getBodyNode()->_getDerivedPosition());
	if(collisionDetector != 0) collisionDetector->update(elapsedTime);
	if(tCircles != 0) tCircles->update(elapsedTime);

	currentAltitude = character->getBodyNode()->getPosition().distance(Ogre::Vector3::ZERO) - 5000;
	percentAltitude = currentAltitude / originalDistance;

	if (mAni) mAni->addTime(elapsedTime);
}


//------------------------------------------------------------------------------------
void GameSystem::checkPlanetColission(Ogre::Real timeElapsed)
{
	if(character->getLandingStatus()) return;

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
			pManager->disableParticle();
			pObjects->getSignNode()->setVisible(false);
			isLanding = true;
			//mCamera->setOrientation(Ogre::Quaternion::IDENTITY);
			mGGBirds->killAllBirds();

			Ogre::Real disToTarget = charPos.distance(targetPosition);
			if(disToTarget < pObjects->getTargetRadius() && character->getParachuteStatus()) 
			{
				isLandingSuccess = true;
				character->addScore(1000);
			}
		}
	}	
}

//-------------------------------------------------------------------------------------
void GameSystem::keyPressed( const OIS::KeyEvent &arg )
{	
	if(isLanding) return;

	if (arg.key == OIS::KC_P)
	{
		bStopFalling = !bStopFalling;
		mPPSoundManager->playMusic("skydiving_gamemenu_loop2.mp3");
	}

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
	else if(arg.key == OIS::KC_O)
	{
		this->openParachute();
	}
}

//-------------------------------------------------------------------------------------
void GameSystem::openParachute()
{
	if(character->getParachuteStatus()) return;

	character->openParachute();
	pManager->setSlowParticle();
}

//-------------------------------------------------------------------------------------
void GameSystem::keyReleased( const OIS::KeyEvent &arg )
{
	if(isLanding) return;

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
	mAni = pObjects->setup(mSceneMgr, rayCollisionDetector);

	mLoadingBar->update();

	tCircles = new TargetCircles();
	tCircles->setup(mSceneMgr);

	collisionDetector = new CollisionDetector();
	collisionDetector->initCollisionDetector(this->character, this->tCircles);

	mLoadingBar->update();

	pManager = new ParticleManager();
	pManager->initParticle(mSceneMgr, this->character);

	mLoadingBar->finish();
}

//-------------------------------------------------------------------------------------
bool GameSystem::isUpsideTarget()
{
	/* don't use this code if x and z value of target position are not zero */
	Ogre::Vector3 char3DPos = character->getBodyNode()->getPosition();

	Ogre::Vector2 target(targetPosition.x, targetPosition.z);
	Ogre::Vector2 charPos(char3DPos.x, char3DPos.z);

	Ogre::Real distance = charPos.distance(target);	
	if(distance > 50) return false;
	return true;
}

//-------------------------------------------------------------------------------------
Ogre::Real GameSystem::getArrowDirection()
{
	Ogre::Quaternion q = character->getBodyNode()->getOrientation();
	Ogre::Vector3 frontVector = Ogre::Vector3::NEGATIVE_UNIT_Z;
	frontVector = q * frontVector;

	Ogre::Vector3 dir = targetPosition - character->getBodyNode()->getPosition();
	dir.normalise();

	Ogre::Quaternion rotAngle = frontVector.getRotationTo(dir);

	Ogre::Radian radDegree = -rotAngle.getYaw();;
	return radDegree.valueDegrees();
}

//-------------------------------------------------------------------------------------
void GameSystem::isPlanetReady()
{
	GalaxyEngine::Planet* planet = planetEngine->getFirstPlanet();	
	if(planet != 0)
	{
		Ogre::SceneNode* parentNode = planet->getParentSceneNode();
		if(parentNode == NULL) 
		{
		}
		else
		{
			isPlanetInitialized = true;
			postPlanetInitialization();
		}
	}
}


void GameSystem::processKinectInput(Ogre::Real elapsedTime)
{
	if(isLanding) return;

	bool flag = false;

	if(mOgreKinect->mPoseDetect->isPose("open"))
	{
		flag = true;
		openParachuteCounter += elapsedTime;
		if(openParachuteCounter >= openParacuteDelay)
		{
			this->openParachute();
			openParachuteCounter = 0;
		}
	}
	else
	{
		openParachuteCounter = 0.0f;
	}

	if(flag) return;	// means we're opening parachute
						// ignoring all poses

	if(mOgreKinect->mPoseDetect->isPose("front"))
	{
		character->setState(Movement::MOVE_FRONT);
	}	
	else if(mOgreKinect->mPoseDetect->isPose("back"))
	{
		character->setState(Movement::MOVE_BACK);
	}
	else if(mOgreKinect->mPoseDetect->isPose("left"))
	{
		character->setState(Movement::MOVE_LEFT);
	}
	else if(mOgreKinect->mPoseDetect->isPose("right"))
	{
		character->setState(Movement::MOVE_RIGHT);
	}
	else if(mOgreKinect->mPoseDetect->isPose("rotate_left"))
	{
		character->setState(Movement::ROTATE_LEFT);
	}
	else if(mOgreKinect->mPoseDetect->isPose("rotate_right"))
	{
		character->setState(Movement::ROTATE_RIGHT);
	}
	else if(mOgreKinect->mPoseDetect->isPose("none"))
	{
		character->setState(Movement::NOTHING);
	}
}

