
#ifndef __Character_h_
#define __Character_h_

#include "Stdafx.h"

/**  Main Character Class*/
class Character
{
public:
	Character(void);
	virtual ~Character(void);

	virtual void setup(Ogre::SceneManager* mSceneManager, Ogre::Vector3 position, Ogre::Vector3 scale, Ogre::Quaternion orientation);
	virtual void update(Ogre::Real elapsedTime);

	inline Ogre::Entity* getBodyEntity() { return bodyEntity; }
	inline Ogre::SceneNode* getBodyNode() { return bodyNode; }

public:
	Ogre::String entityName;

protected:
	
	Ogre::SceneManager*  mSceneManager;	
	Ogre::Entity*        bodyEntity;
	Ogre::SceneNode*     bodyNode;
	Ogre::Skeleton*      skeleton;
};

#endif // #ifndef __Character_h_