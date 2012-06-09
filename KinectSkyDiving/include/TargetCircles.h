
#ifndef __TargetCircles_h_
#define __TargetCircles_h_

#include "Stdafx.h"
#include "DotSceneLoader.h"
#include "RayCastCollision.h"
#include "Planet.h"

class TargetCircles
{
public:
	TargetCircles(void);
	virtual ~TargetCircles(void);

	void setup(Ogre::SceneManager* mSceneManager);
	inline std::vector<Ogre::SceneNode*> getNodeList(void) 
	{
		return this->nodeList; 
	}

private:
	Ogre::SceneManager*  mSceneManager;	
	Ogre::SceneNode*     mMainNode;
	Ogre::Entity*        mMainEntity;

	Ogre::DotSceneLoader* sceneLoader;
	std::vector<Ogre::SceneNode*> nodeList;
	//Ogre::DotSceneLoader*		 sceneLoader;
};

#endif // #ifndef __TargetCircles_h_


