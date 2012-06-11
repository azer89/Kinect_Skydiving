

#ifndef __ThirdPersonCamera_h_
#define __ThirdPersonCamera_h_

#include "Ogre.h"
#include "GameConfig.h"

//using namespace Ogre;

// Class to create chase camera/third person shooter
class ThirdPersonCamera
{
protected:

	Ogre::Camera *mCamera;			// Ogre camera

	Ogre::SceneManager *mSceneMgr;
	Ogre::String mName;

	bool mOwnCamera;			// To know if the ogre camera binded has been created outside or inside of this class
	Ogre::Vector3 mTightness;	// Determines the movement of the camera - 1 means tight movement, while 0 means no movement

public:
	ThirdPersonCamera (Ogre::String name, Ogre::SceneManager *sceneMgr, Ogre::Camera *camera = 0);
	virtual ~ThirdPersonCamera ();

	void setTightness (Ogre::Vector3 tightness);
	Ogre::Vector3 getTightness ();
	Ogre::Vector3 getCameraPosition ();
	void instantUpdate (Ogre::Vector3 cameraPosition, Ogre::Vector3 targetPosition);
	void instantUpdate (Ogre::Vector3 cameraPosition);
	void update (Ogre::Real elapsedTime, Ogre::Vector3 cameraPosition, Ogre::Vector3 targetPosition, Ogre::Quaternion camOrientation);

	Ogre::SceneNode *mTargetNode;		// The camera target
	Ogre::SceneNode *mCameraNode;		// The camera itself
};

#endif
