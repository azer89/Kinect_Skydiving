
#ifndef __Character_h_
#define __Character_h_

#include "Stdafx.h"

/** Character's Movement*/
enum Movement
{
	NOTHING = 0,
	GO_DOWN = 1,
	MOVE_FRONT = 2,
	MOVE_BACK = 3,
	MOVE_LEFT = 4,
	MOVE_RIGHT = 5,
	ROTATE_LEFT = 6,
	ROTATE_RIGHT = 7,
};

/**  Main Character Class*/
class Character
{
public:
	Character(void);
	virtual ~Character(void);

	void setup(Ogre::SceneManager* mSceneManager, 
		Ogre::Vector3 position, 
		Ogre::Vector3 scale, 
		Ogre::Quaternion orientation);

	void update(Ogre::Real elapsedTime);
	
	void setState(Movement m);
	void setGravity(Ogre::Real gravity) { this->gravity = gravity; }
	void setLanding() { isLanding = true; }

	inline Ogre::Entity*    getBodyEntity() { return bodyEntity; }
	inline Ogre::SceneNode* getBodyNode() { return mMainNode; }
	inline Ogre::SceneNode* getSightNode() { return mSightNode; }
	inline Ogre::SceneNode* getCameraNode() { return mCameraNode; }
	inline Ogre::Vector3    getWorldPosition() { return mMainNode->_getDerivedPosition (); }

public:
	Ogre::String entityName;

protected:
	void moveCharacter(Ogre::Real elapsedTime);
	void fallDown(Ogre::Real elapsedTime);

protected:

	bool isLanding;
	Ogre::Real gravity;
	Ogre::Real degreeRotation;
	Movement state;

	Ogre::SceneNode*     mMainNode;			// Main character node
	Ogre::SceneNode*	 mSightNode;		// "Sight" node - The character is supposed to be looking here
	Ogre::SceneNode*	 mCameraNode;		// Node for the chase camera
	
	Ogre::SceneManager*  mSceneManager;	
	Ogre::Entity*        bodyEntity;	
	Ogre::Skeleton*      skeleton;
};

#endif // #ifndef __Character_h_