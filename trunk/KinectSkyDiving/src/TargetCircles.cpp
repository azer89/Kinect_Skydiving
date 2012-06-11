
#include "Stdafx.h"
#include "TargetCircles.h"

#include <cstdlib> 
#include <ctime> 
#include <iostream>

//------------------------------------------------------------------------------------
TargetCircles::TargetCircles(void)
{
}

//------------------------------------------------------------------------------------
TargetCircles::~TargetCircles(void)
{
}

//------------------------------------------------------------------------------------
void TargetCircles::setup(Ogre::SceneManager* mSceneManager)
{
	this->mSceneManager = mSceneManager;	
	this->mMainNode = mSceneManager->getRootSceneNode()->createChildSceneNode();

	sceneLoader = new Ogre::DotSceneLoader();
	sceneLoader->parseDotScene(GameConfig::getSingletonPtr()->getCircleSceneName(), "Popular", mSceneManager, mMainNode);
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

	Ogre::Vector3 objScale(GameConfig::getSingletonPtr()->getCircleScale());
	
	srand (time(0));

	for(int a = 0; a < nodeList.size(); a++)
	{
		Ogre::SceneNode* childNode = nodeList[a];

		//childNode->showBoundingBox(true);

		Ogre::Entity* entity = static_cast<Ogre::Entity*>(childNode->getAttachedObject(0));
		Ogre::AnimationState* state = entity->getAnimationState("go");
		state->setEnabled(true);
		state->setLoop(true);
		animations.push_back(state);

		Ogre::Vector3 pos = childNode->getPosition();

		Ogre::Vector3 newPos((pos.x * scaleX) + targetPoint.x, 
							 (pos.y * scaleY) + targetPoint.y, 
							 (pos.z * scaleZ) + targetPoint.z);
				
		Ogre::Vector3 upVector = newPos.normalisedCopy();
		Ogre::Quaternion q = Ogre::Vector3::UNIT_Y.getRotationTo(upVector);

		childNode->setPosition(newPos);
		childNode->setOrientation(q);
		childNode->setScale(objScale);

		if(a > 5)
		{
			char name[16];
			sprintf(name, "StaticCloud%d", a);

			Ogre::SceneNode* myNode = static_cast<Ogre::SceneNode*>(mSceneManager->getRootSceneNode()->createChild());
			Ogre::BillboardSet* mySet = mSceneManager->createBillboardSet(name);
			mySet->setBillboardType(Ogre::BillboardType::BBT_POINT);
			mySet->setMaterialName("Examples/Cloud01");	
			Ogre::Billboard* myBillboard = mySet->createBillboard(Ogre::Vector3::ZERO);
			myNode->attachObject(mySet);

			int rx = (rand() % 500) - 250;
			int ry = (rand() % 500) - 250;
			int rz = (rand() % 500) - 250;

			myNode->setPosition(newPos + Ogre::Vector3(rx, ry, rz));
			myNode->setScale(Ogre::Vector3(1.5));
		}
	}

	delete sceneLoader;
}

void TargetCircles::update(Ogre::Real elapsedTime)
{
	for(int a = 0; a < animations.size(); a++)
	{
		animations[a]->addTime(elapsedTime * 1.0);
	}
}


