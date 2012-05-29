
#include "Stdafx.h"
#include "SimpleCloud.h"

#include <cstdlib> 
#include <ctime> 
#include <iostream>

//-------------------------------------------------------------------------------------
SimpleCloud::SimpleCloud(void)
{
}

//-------------------------------------------------------------------------------------
SimpleCloud::~SimpleCloud(void)
{
}

//-------------------------------------------------------------------------------------
Ogre::Vector3 SimpleCloud::sphericalToCartesianNormal(Ogre::Real theta, Ogre::Real phi)
{
	return Ogre::Vector3(sin(phi) * cos(theta), sin(phi) * sin(theta), cos(phi));
}

//-------------------------------------------------------------------------------------
void SimpleCloud::initCloud(Ogre::SceneManager* mSceneMgr, int numBillboards)
{
	this->mSceneMgr = mSceneMgr;

	srand (time(0));
	for(int a = 0; a < numBillboards; a++)
	{
		int randomx = rand()%200; 
		int randomy = rand()%200;
				
		Ogre::Vector3 pos = sphericalToCartesianNormal(randomx, randomy) * 575;
		std::cout << pos.distance(Ogre::Vector3::ZERO) << "\n";

		createSingleCloud(pos);
	}
}

//-------------------------------------------------------------------------------------
void SimpleCloud::updateClouds(Ogre::Real elapsedTime)
{
	Ogre::Vector3 dir = Ogre::Vector3::NEGATIVE_UNIT_X;

	for(int a = 0; a < cloudNodes.size(); a++)
	{
		Ogre::SceneNode* node = cloudNodes[a];
		Ogre::Vector3 up = node->_getDerivedPosition().normalisedCopy();
		Ogre::Quaternion orient = Ogre::Vector3::UNIT_Y.getRotationTo(up);

		Ogre::Vector3 nextDir = orient * dir;
		nextDir = nextDir * 50 * elapsedTime;

		node->translate(nextDir);
	}
}

//-------------------------------------------------------------------------------------
void SimpleCloud::createSingleCloud(Ogre::Vector3 pos)
{
	int num = cloudNodes.size();
	char name[16];
	sprintf(name, "Cloud%d", num);

	Ogre::SceneNode* myNode = static_cast<Ogre::SceneNode*>(mSceneMgr->getRootSceneNode()->createChild());
	Ogre::BillboardSet* mySet = mSceneMgr->createBillboardSet(name);
	mySet->setBillboardType(Ogre::BillboardType::BBT_POINT);
	//mySet->setBillboardType(Ogre::BillboardType::BBT_PERPENDICULAR_COMMON);
	mySet->setMaterialName("Examples/Cloud01");
	//mySet->setCommonDirection(pos.normalisedCopy());
	Ogre::Billboard* myBillboard = mySet->createBillboard(Ogre::Vector3::ZERO);
	myNode->attachObject(mySet);
	myNode->setPosition(pos);
	myNode->setScale(Ogre::Vector3(2, 2, 2));
	cloudNodes.push_back(myNode);
}


