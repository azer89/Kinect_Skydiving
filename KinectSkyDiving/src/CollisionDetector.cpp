
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
	if(nodeList.size() == 0)  return;

	for(int a = 0; a < nodeList.size(); a++)
	{
		Ogre::SceneNode* node = nodeList[a];
		
		Ogre::String name = node->getName();
		
		Ogre::AxisAlignedBox cBBox;
		if(!character->getParachuteStatus()) cBBox = character->getBodyEntity01()->getWorldBoundingBox();
		else cBBox = character->getBodyEntity02()->getWorldBoundingBox();
		
		Ogre::AxisAlignedBox bBox = node->getAttachedObject(0)->getWorldBoundingBox();	
		//std::cout << distance << "\n";

		if(bBox.intersects(cBBox) && !tCircles->flag[a])
		{
			Ogre::Real distance = bBox.getCenter().distance(cBBox.getCenter());
			tCircles->flag[a] = true;

			if(distance <= 15.0)
			{
				int point = 0;
				
				if(Ogre::StringUtil::startsWith(name, "r")) point = -100;
				else if (Ogre::StringUtil::startsWith(name, "b")) point = 100;
				this->character->addScore(point);
			}
		}
	}
}