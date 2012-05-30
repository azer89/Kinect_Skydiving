
#include "Stdafx.h"
#include "SimpleCloud.h"

#include <cstdlib> 
#include <ctime> 
#include <iostream>

//-------------------------------------------------------------------------------------
SimpleCloud::SimpleCloud(void)
	: highestElevation(2500.0f)
	  //updateDelay(0.0f)
{
}

//-------------------------------------------------------------------------------------
SimpleCloud::~SimpleCloud(void)
{
	if(cloudNodes.size() > 0) cloudNodes.clear();
}

//-------------------------------------------------------------------------------------
/** note it returns Vector2(theta, phi) */
Ogre::Vector2 SimpleCloud::cartesiantoSpherical(Ogre::Vector3 pos)
{
	Ogre::Real dist = pos.distance(Ogre::Vector3::ZERO);
	Ogre::Radian theta = Ogre::Math::ACos(pos.z / dist);
	Ogre::Radian phi = Ogre::Math::ASin( pos.y / (Ogre::Math::Sqrt(pos.x * pos.x + pos.y * pos.y)) );

	return Ogre::Vector2(theta.valueRadians(), phi.valueRadians());
}

//-------------------------------------------------------------------------------------
Ogre::Vector3 SimpleCloud::sphericalToCartesian(Ogre::Real theta, Ogre::Real phi)
{
	return Ogre::Vector3(sin(phi) * cos(theta), sin(phi) * sin(theta), cos(phi));
}

//-------------------------------------------------------------------------------------
void SimpleCloud::initCloud(Ogre::SceneManager* mSceneMgr, int numBillboards)
{
	this->mSceneMgr = mSceneMgr;
	int div3 = numBillboards / 3;

	srand (time(0));

	//Ogre::Vector3 initPos(0, 3000, 3000);
	//Ogre::Vector2 spCoord = cartesiantoSpherical(initPos);

	for(int a = 0; a < div3; a++)
	{
		int randomx = (rand() % 50) - 25; 
		int randomy = (rand() % 50) - 25;
		Ogre::Vector3 pos = sphericalToCartesian(randomx, randomy) * highestElevation;
		createSingleCloud(pos, Ogre::Vector3(5.0));
	}

	for(int a = 0; a < div3; a++)
	{
		int randomx =(rand() % 50) - 25; 
		int randomy = (rand() % 50) - 25;
		Ogre::Vector3 pos = sphericalToCartesian(randomx, randomy) * (highestElevation - 150);
		createSingleCloud(pos, Ogre::Vector3(5.0));
	}

	for(int a = 0; a < div3; a++)
	{
		int randomx = (rand() % 50) - 25; 
		int randomy = (rand() % 50) - 25;
		Ogre::Vector3 pos = sphericalToCartesian(randomx, randomy) * (highestElevation - 100);
		createSingleCloud(pos, Ogre::Vector3(5.0));
	}
}

//-------------------------------------------------------------------------------------
void SimpleCloud::updateClouds(Ogre::Real elapsedTime)
{
	//updateDelay += elapsedTime;
	//if(updateDelay < 0.0166f) return;
	//updateDelay = 0.0f;

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
void SimpleCloud::createSingleCloud(Ogre::Vector3 pos, Ogre::Vector3 scale)
{
	int num = cloudNodes.size();
	char name[16];
	sprintf(name, "Cloud%d", num);

	Ogre::SceneNode* myNode = static_cast<Ogre::SceneNode*>(mSceneMgr->getRootSceneNode()->createChild());
	Ogre::BillboardSet* mySet = mSceneMgr->createBillboardSet(name);
	mySet->setBillboardType(Ogre::BillboardType::BBT_POINT);
	//mySet->setBillboardType(Ogre::BillboardType::BBT_PERPENDICULAR_COMMON);
	//mySet->setCommonDirection(pos.normalisedCopy());
	mySet->setMaterialName("Examples/Cloud01");	
	Ogre::Billboard* myBillboard = mySet->createBillboard(Ogre::Vector3::ZERO);
	myNode->attachObject(mySet);
	myNode->setPosition(pos);
	myNode->setScale(scale);
	cloudNodes.push_back(myNode);
}


