
#include "Stdafx.h"
#include "ThirdPersonCamera.h"

using namespace Ogre;

ThirdPersonCamera::ThirdPersonCamera (String name, SceneManager *sceneMgr, Camera *camera) 
{
	camera->setPosition(0, 3000.0f, 3000.0f);
	camera->lookAt(0, 1000, 0);
	camera->setNearClipDistance(0.5f);
	camera->setFarClipDistance(17500.0f);

	// Basic member references setup
	mName = name;
	mSceneMgr = sceneMgr;

	// Create the camera's node structure
	mCameraNode = mSceneMgr->getRootSceneNode ()->createChildSceneNode (mName);
	mTargetNode = mSceneMgr->getRootSceneNode ()->createChildSceneNode (mName + "_target");
	//mCameraNode->setPosition (Ogre::Vector3::ZERO);
	//mTargetNode->setPosition (Ogre::Vector3::ZERO);
	mCameraNode->setAutoTracking (true, mTargetNode);		// The camera will always look at the camera target
	//mCameraNode->setFixedYawAxis (true);					// Needed because of auto tracking
	
	// Create our camera if it wasn't passed as a parameter
	if (camera == 0) 
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

	// and attach the Ogre camera to the camera node
	mCameraNode->attachObject(mCamera);

	// Default tightness
	mTightness = Ogre::Vector3(0.5f, 0.5f, 0.5f);
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

Vector3 ThirdPersonCamera::getCameraPosition () { return mCameraNode->getPosition (); }

void ThirdPersonCamera::instantUpdate (Vector3 cameraPosition) { mCameraNode->setPosition (cameraPosition); }

void ThirdPersonCamera::instantUpdate (Vector3 cameraPosition, Vector3 targetPosition) 
{
	mCameraNode->setPosition (cameraPosition);
	mTargetNode->setPosition (targetPosition);

	//mCamera->lookAt(targetPosition);
}

void ThirdPersonCamera::update (Real elapsedTime, Vector3 cameraPosition, Vector3 targetPosition, Quaternion camOrientation) 
{
	Vector3 displacement01 = (targetPosition - mTargetNode->getPosition());
	displacement01.x *= mTightness.x * 10.0f * elapsedTime;
	displacement01.y *= mTightness.y * 10.0f * elapsedTime;
	displacement01.z *= mTightness.z * 10.0f * elapsedTime;

	Vector3 displacement02 = (cameraPosition - mCameraNode->getPosition());
	displacement02.x *= mTightness.x * 10.0f * elapsedTime;
	displacement02.y *= mTightness.y * 10.0f * elapsedTime;	
	displacement02.z *= mTightness.z * 10.0f * elapsedTime;
	
	mTargetNode->translate(displacement01);
	mCameraNode->translate(displacement02);


	mCameraNode->setOrientation(camOrientation);
	//Ogre::Vector3 upVector = mCameraNode->getPosition() - mTargetNode->getPosition();
	//upVector.normalise();
	//mCameraNode->lookAt(upVector, Ogre::Node::TransformSpace::TS_WORLD);


}

