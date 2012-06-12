
#ifndef __Character_h_
#define __Character_h_

#include "Stdafx.h"
#include "GameConfig.h"

enum AnimID01
{
	BACK = 0,
	FRONT = 1,
	LEFT = 2,
	RIGHT = 3,
	NO_ANIM1 = 4
};

enum AnimID02
{
	AFTER_LANDING_WAIT = 0,
	FLY_WITH_PARACHUTE = 1,
	LAND = 2,
	OPEN = 3,
	NO_ANIM2 = 4
};

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

#define NUM_ANIM 4

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
	void openParachute();
	void setGravity(Ogre::Real gravity) { this->gravity = gravity; }
	void setLanding();
	
	inline Ogre::Entity*    getBodyEntity() { return bodyEntity01; }
	inline Ogre::SceneNode* getBodyNode() { return mMainNode; }
	inline Ogre::SceneNode* getSightNode() { return mSightNode; }
	inline Ogre::SceneNode* getCameraNode() { return mCameraNode; }
	inline Ogre::Vector3    getWorldPosition() { return mMainNode->_getDerivedPosition (); }
	inline int				getGameplayPoint() { return point; }
	
	void addPoint(int num) 
	{ 
		point += num;
		if(point < 0) point = 0;
	}

private:
	void moveCharacter(Ogre::Real elapsedTime);
	void fallDown(Ogre::Real elapsedTime);
	void setAnimation(AnimID01 id, bool noTransition = false);
	void setParachuteAnimation(AnimID02 id, bool noTransition = false);
	void fadeAnimations(Ogre::Real deltaTime);
	void updateAnimations(Ogre::Real deltaTime);

private:
	bool			isLanding;
	bool			isParachuteOpen;
	Ogre::Real		gravity;
	Ogre::Real		degreeRotation;

	Movement		state;
	Movement		previousState;

	Ogre::Real		currentSpeed[4];
	Ogre::Real		maxSpeed;
	Ogre::Real		acceleration;
	Ogre::Real		rotationSpeed;

	Ogre::SceneNode*     mMainNode;			// Main character node
	Ogre::SceneNode*	 innerNode;			// Node which has entity attached
	Ogre::SceneNode*	 mSightNode;		// "Sight" node - The character is supposed to be looking here
	Ogre::SceneNode*	 mCameraNode;		// Node for the chase camera
	
	Ogre::SceneManager*  mSceneManager;	
	Ogre::Skeleton*      skeleton;

	int point;

	Ogre::Entity*				 bodyEntity01;		// girl mesh only
	Ogre::Entity*				 bodyEntity02;		// girl mesh with parachute
	const static std::string	 animNames01[];
	const static std::string     animNames02[];
	Ogre::AnimationState*		 mAnims01[NUM_ANIM];
	Ogre::AnimationState*		 mAnims02[NUM_ANIM];
	bool						 mFadingIn01[NUM_ANIM]; 
	bool						 mFadingOut01[NUM_ANIM];        
	bool						 mFadingIn02[NUM_ANIM];           
	bool						 mFadingOut02[NUM_ANIM];   
	AnimID01					 baseAnimID01;
	AnimID02					 baseAnimID02;
	AnimID01					 prevAnimID01;
	AnimID02					 prevAnimID02;
	Ogre::Real					 mTimer;            // general timer to see how long animations have been playing

	//Ogre::RibbonTrail*	 mTrail;
	//Ogre::SceneNode*	 trail01Node;
	//Ogre::SceneNode*	 trail02Node;
};

#endif // #ifndef __Character_h_