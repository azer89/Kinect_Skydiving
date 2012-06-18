


#ifndef __GGBirdLoader_h_
#define __GGBirdLoader_h_

#include "Stdafx.h"
#include "DotSceneLoader.h"
#include "RayCastCollision.h"
#include "Planet.h"
#include "GameConfig.h"

class GGBirdLoader
{
public:
	GGBirdLoader(void);
	virtual ~GGBirdLoader(void);

	void setup(Ogre::SceneManager* mSceneManager);
	void update(Ogre::Real elapsedTime);

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
	std::vector<Ogre::AnimationState*> animations;
};

#endif // #ifndef __GGBirdLoader_h_



