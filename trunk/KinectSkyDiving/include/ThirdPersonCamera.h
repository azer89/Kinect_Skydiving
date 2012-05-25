

#ifndef __ThirdPersonCamera_h_
#define __ThirdPersonCamera_h_

#include "Ogre.h"

using namespace Ogre;

// Class to create chase camera/third person shooter
class ThirdPersonCamera
{
protected:

	Camera *mCamera;			// Ogre camera

	SceneManager *mSceneMgr;
	String mName;

	bool mOwnCamera;			// To know if the ogre camera binded has been created outside or inside of this class
	Ogre::Vector3 mTightness;	// Determines the movement of the camera - 1 means tight movement, while 0 means no movement

public:
	ThirdPersonCamera (String name, SceneManager *sceneMgr, Camera *camera = 0);
	virtual ~ThirdPersonCamera ();

	void setTightness (Ogre::Vector3 tightness);
	Ogre::Vector3 getTightness ();
	Vector3 getCameraPosition ();
	void instantUpdate (Vector3 cameraPosition, Vector3 targetPosition);
	void instantUpdate (Vector3 cameraPosition);
	void update (Real elapsedTime, Vector3 cameraPosition, Vector3 targetPosition, Quaternion camOrientation);

	SceneNode *mTargetNode;		// The camera target
	SceneNode *mCameraNode;		// The camera itself
};

#endif
