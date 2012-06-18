
#ifndef __TargetCircles_h_
#define __TargetCircles_h_

#include "Stdafx.h"
#include "DotSceneLoader.h"
#include "RayCastCollision.h"
#include "Planet.h"
#include "GameConfig.h"

class TargetCircles
{
public:
	TargetCircles(void);
	virtual ~TargetCircles(void);

	void setup(Ogre::SceneManager* mSceneManager);
	void update(Ogre::Real elapsedTime);

	inline std::vector<Ogre::SceneNode*> getNodeList(void) 
	{
		return this->nodeList; 
	}

public:
	std::vector<bool>	flag;

private:
	Ogre::SceneManager*  mSceneManager;	
	Ogre::SceneNode*     mMainNode;
	Ogre::Entity*        mMainEntity;

	Ogre::DotSceneLoader* sceneLoader;
	std::vector<Ogre::SceneNode*> nodeList;
	
	std::vector<Ogre::AnimationState*> animations;
	Ogre::Real	animSpeed;
};

#endif // #ifndef __TargetCircles_h_


