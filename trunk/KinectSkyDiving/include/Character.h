
#ifndef __Character_h_
#define __Character_h_

#include "Stdafx.h"
#include "GameConfig.h"
//#include "CCPhysics.h"

/** Character's Movement*/
enum Movement
{	
	MOVE_FRONT = 0,
	MOVE_BACK = 1,
	MOVE_LEFT = 2,
	MOVE_RIGHT = 3,
	NOTHING = 4,
	ROTATE_LEFT = 5,
	ROTATE_RIGHT = 6,
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
	//inline CharacterControllerPhysics* getCCPhysics() { return mCCPhysics; }

public:
	Ogre::String entityName;

private:
	void moveCharacter(Ogre::Real elapsedTime);
	void fallDown(Ogre::Real elapsedTime);

private:
	//CharacterControllerPhysics*		mCCPhysics;
	bool			isLanding;
	Ogre::Real		gravity;
	Ogre::Real		degreeRotation;

	Movement		state;
	Movement		previousState;

	Ogre::Real		currentSpeed[4];
	Ogre::Real		maxSpeed;
	Ogre::Real		acceleration;
	Ogre::Real		rotationSpeed;

	Ogre::SceneNode*     mMainNode;			// Main character node
	Ogre::SceneNode*	 mSightNode;		// "Sight" node - The character is supposed to be looking here
	Ogre::SceneNode*	 mCameraNode;		// Node for the chase camera
	
	Ogre::SceneManager*  mSceneManager;	
	Ogre::Entity*        bodyEntity;	
	Ogre::Skeleton*      skeleton;

	//Ogre::RibbonTrail*	 mTrail;
	//Ogre::SceneNode*	 trail01Node;
	//Ogre::SceneNode*	 trail02Node;
};

#endif // #ifndef __Character_h_