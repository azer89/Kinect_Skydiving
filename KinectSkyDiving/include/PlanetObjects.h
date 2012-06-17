
#ifndef __PlanetObjects_h_
#define __PlanetObjects_h_

#include "Stdafx.h"
#include "DotSceneLoader.h"
#include "RayCastCollision.h"
#include "Planet.h"
#include "GameConfig.h"

class PlanetObjects
{
public:
	PlanetObjects(void);
	virtual ~PlanetObjects(void);

	Ogre::AnimationState* setup(Ogre::SceneManager* mSceneManager, RayCastCollision* collisionDetector);

	Ogre::SceneNode* getSignNode(){ return signNode; }

private:
	Ogre::SceneManager*  mSceneManager;	
	Ogre::SceneNode*     mMainNode;
	Ogre::Entity*        mMainEntity;

	Ogre::SceneNode* signNode;
	Ogre::Entity* signEntity;
	
	//Ogre::DotSceneLoader*		 sceneLoader;
};

#endif // #ifndef __PlanetObjects_h_


