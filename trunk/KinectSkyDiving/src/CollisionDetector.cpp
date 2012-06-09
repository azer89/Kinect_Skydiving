
#include "Stdafx.h"
#include "CollisionDetector.h"

CollisionDetector::CollisionDetector(void)
{
}

CollisionDetector::~CollisionDetector(void)
{
}

void CollisionDetector::initCollisionDetector(Character* character, TargetCircles* tCircles)
{
	this->character = character;
	this->tCircles = tCircles;
}

void CollisionDetector::update(Ogre::Real elapsedTime)
{
	
	std::vector<Ogre::SceneNode*> nodeList = tCircles->getNodeList();

	if(nodeList.size() == 0) 
		return;

	for(int a = 0; a < nodeList.size(); a++)
	{
		Ogre::SceneNode* node = nodeList[a];
		//Ogre::AxisAlignedBox cBBox = character->getBodyNode()->getAttachedObject(0)->getBoundingBox();
		//Ogre::AxisAlignedBox bBox = node->getAttachedObject(0)->getBoundingBox();
		//if(bBox.intersects(cBBox))
		//{
		//	std::cout << "===========asdasdasdasdasd===========\n";
		//}

		
	}
}