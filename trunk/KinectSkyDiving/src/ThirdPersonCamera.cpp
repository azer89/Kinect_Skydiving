
#include "Stdafx.h"
#include "ThirdPersonCamera.h"

//using namespace Ogre;

ThirdPersonCamera::ThirdPersonCamera (Ogre::String name, Ogre::SceneManager *sceneMgr, Ogre::Camera *camera) 
{
	Ogre::Vector3 charPos(0, 5750, 1000);
	Ogre::Vector3 targetPos(0, 5000, 0);

	camera->setPosition(charPos);
	camera->lookAt(targetPos);
	camera->setNearClipDistance(0.5f);
	camera->setFarClipDistance(17500.0f);

	// Basic member references setup
	mName = name;
	mSceneMgr = sceneMgr;

	// Create the camera's node structure
	mCameraNode = mSceneMgr->getRootSceneNode ()->createChildSceneNode (mName);
	mTargetNode = mSceneMgr->getRootSceneNode ()->createChildSceneNode (mName + "_target");
	mCameraNode->setAutoTracking (true, mTargetNode);		// The camera will always look at the camera target
	//mCameraNode->setFixedYawAxis (true);					// Needed because of auto tracking
		
	if (camera == 0)	// Create our camera if it wasn't passed as a parameter
	{
		mCamera = mSceneMgr->createCamera(mName);
		mOwnCamera = true;
	}
	else 
	{
		mCamera = camera;
		mCamera->setPosition(0, 0, 0);
		mOwnCamera = false;
	}

	mCameraNode->attachObject(mCamera);

	// Default tightness
	mTightness = GameConfig::getSingletonPtr()->getCameraTightness();
}

ThirdPersonCamera::~ThirdPersonCamera () 
{
	mCameraNode->detachAllObjects ();
	if (mOwnCamera) delete mCamera;
	mSceneMgr->destroySceneNode (mName);
	mSceneMgr->destroySceneNode (mName + "_target");
}

void ThirdPersonCamera::setTightness (Ogre::Vector3 tightness) { mTightness = tightness; }
Ogre::Vector3 ThirdPersonCamera::getTightness () { return mTightness; }
Ogre::Vector3 ThirdPersonCamera::getCameraPosition () { return mCameraNode->getPosition (); }
void ThirdPersonCamera::instantUpdate (Ogre::Vector3 cameraPosition) { mCameraNode->setPosition (cameraPosition); }

void ThirdPersonCamera::instantUpdate (Ogre::Vector3 cameraPosition, Ogre::Vector3 targetPosition) 
{
	mCameraNode->setPosition (cameraPosition);
	mTargetNode->setPosition (targetPosition);
}

void ThirdPersonCamera::update (Ogre::Real elapsedTime, Ogre::Vector3 cameraPosition, Ogre::Vector3 targetPosition, Ogre::Quaternion camOrientation) 
{
	Ogre::Vector3 displacement01 = (targetPosition - mTargetNode->getPosition());
	displacement01.x *= mTightness.x * 10.0f * elapsedTime;
	displacement01.y *= mTightness.y * 10.0f * elapsedTime;
	displacement01.z *= mTightness.z * 10.0f * elapsedTime;

	Ogre::Vector3 displacement02 = (cameraPosition - mCameraNode->getPosition());
	displacement02.x *= mTightness.x * 10.0f * elapsedTime;
	displacement02.y *= mTightness.y * 10.0f * elapsedTime;	
	displacement02.z *= mTightness.z * 10.0f * elapsedTime;
	
	mTargetNode->translate(displacement01);
	mCameraNode->translate(displacement02);

	mCameraNode->setOrientation(camOrientation);


}

