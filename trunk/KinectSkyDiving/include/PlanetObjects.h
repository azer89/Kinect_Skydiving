
#ifndef __PlanetObjects_h_
#define __PlanetObjects_h_

#include "Stdafx.h"
#include "DotSceneLoader.h"
#include "RayCastCollision.h"
#include "Planet.h"

class PlanetObjects
{
public:
	PlanetObjects(void);
	virtual ~PlanetObjects(void);

	void setup(Ogre::SceneManager* mSceneManager, RayCastCollision* collisionDetector);

private:
	Ogre::SceneManager*  mSceneManager;	
	Ogre::SceneNode*     mMainNode;
	Ogre::Entity*        mMainEntity;
	
	Ogre::DotSceneLoader*		 sceneLoader;
};

#endif // #ifndef __PlanetObjects_h_


