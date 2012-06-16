
#include "Stdafx.h"
#include "GGBirdLoader.h"

#include <cstdlib> 
#include <ctime> 
#include <iostream>

//------------------------------------------------------------------------------------
GGBirdLoader::GGBirdLoader(void)
{
}

//------------------------------------------------------------------------------------
GGBirdLoader::~GGBirdLoader(void)
{
}

//------------------------------------------------------------------------------------
void GGBirdLoader::setup(Ogre::SceneManager* mSceneManager)
{
	this->mSceneManager = mSceneManager;	
	this->mMainNode = mSceneManager->getRootSceneNode()->createChildSceneNode();

	sceneLoader = new Ogre::DotSceneLoader();
	sceneLoader->parseDotScene(GameConfig::getSingletonPtr()->getGGBirdSceneName(), "Popular", mSceneManager, mMainNode);
	if(nodeList.size() == 0)
		nodeList = sceneLoader->nodeList;

	Ogre::Vector3 v1 = GameConfig::getSingletonPtr()->getTargetPosition();
	Ogre::Vector3 v2 = GameConfig::getSingletonPtr()->getCharacterPosition();

	Ogre::Vector3 targetPoint(v1.x, v1.y, v1.z);				// supposed goal
	Ogre::Vector3 charPos(v2.x, v2.y - 50, v2.z - 100);			// character position a more forward
	Ogre::Vector3 lastPos = nodeList[nodeList.size() - 1]->_getDerivedPosition();

	// avoid divide by zero
	if(lastPos.z == 0) lastPos.z = 1;	
	if(lastPos.y == 0) lastPos.y = 1;

	Ogre::Vector3 diff = charPos - targetPoint;

	Ogre::Real scaleZ = diff.z / lastPos.z;
	Ogre::Real scaleY = diff.y / lastPos.y;
	Ogre::Real scaleX = (scaleZ + scaleY) / 3.0f;

	Ogre::Vector3 objScale(GameConfig::getSingletonPtr()->getGGBirdScale());

	srand (time(0));

	for(int a = 0; a < nodeList.size(); a++)
	{
		Ogre::SceneNode* childNode = nodeList[a];

		//childNode->showBoundingBox(true);

		Ogre::Entity* entity = static_cast<Ogre::Entity*>(childNode->getAttachedObject(0));
		/*Ogre::AnimationState* state = entity->getAnimationState("go");
		state->setEnabled(true);
		state->setLoop(true);
		animations.push_back(state);*/

		Ogre::Vector3 pos = childNode->getPosition();

		Ogre::Vector3 newPos((pos.x * scaleX) + targetPoint.x, 
			(pos.y * scaleY) + targetPoint.y, 
			(pos.z * scaleZ) + targetPoint.z);

		Ogre::Vector3 upVector = newPos.normalisedCopy();
		Ogre::Quaternion q = Ogre::Vector3::UNIT_Y.getRotationTo(upVector);

		childNode->setPosition(newPos);
		childNode->setOrientation(q);
		childNode->setScale(objScale);
	}

	delete sceneLoader;
}

void GGBirdLoader::update(Ogre::Real elapsedTime)
{
	/*for(int a = 0; a < animations.size(); a++)
	{
		animations[a]->addTime(elapsedTime * 1.0);
	}*/
}


