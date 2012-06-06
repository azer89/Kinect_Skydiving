
#include "Stdafx.h"
#include "PlanetObjects.h"

//------------------------------------------------------------------------------------
PlanetObjects::PlanetObjects(void)
	//: sceneLoader(0)
{
}

//------------------------------------------------------------------------------------
PlanetObjects::~PlanetObjects(void)
{
	//if(sceneLoader != 0) delete sceneLoader;
}

//------------------------------------------------------------------------------------
void PlanetObjects::setup(Ogre::SceneManager* mSceneManager, RayCastCollision* collisionDetector)
{
	this->mSceneManager = mSceneManager;	
	this->mMainNode = mSceneManager->getRootSceneNode()->createChildSceneNode();

	Ogre::DotSceneLoader* sceneLoader = new Ogre::DotSceneLoader();
	sceneLoader->parseDotScene("village.scene", "Popular", mSceneManager, mMainNode);
	
	std::vector<Ogre::SceneNode*> nodeList = sceneLoader->nodeList;

	for(int a = 0; a < nodeList.size(); a++)
	{
		Ogre::SceneNode* childNode = nodeList[a];

		childNode->setScale(5, 5, 5);
		childNode->translate(0, 5100, 0);

		Ogre::Vector3 devPos = childNode->_getDerivedPosition();
		Ogre::Vector3 upVector = devPos;
		upVector.normalise();
		Ogre::Quaternion q = Ogre::Vector3::UNIT_Y.getRotationTo(upVector);

		Ogre::Vector3 intersection = Ogre::Vector3::ZERO;
		collisionDetector->getChunksIntersection(devPos, -upVector, intersection);
				
		childNode->setOrientation(q);
		
		if(intersection.x == 0 && intersection.y == 5000 && intersection.z == 0)	// ray intersection fails
		{
			childNode->setPosition(upVector * 5000);	// guess the terrain height is 2000 from planet's center
		}
		else	// ray intersection is obtained
		{
			childNode->setPosition(intersection);
		}
		
	}

	delete sceneLoader;

}
