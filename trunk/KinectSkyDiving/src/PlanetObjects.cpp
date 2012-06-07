
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

	Ogre::Vector3 scaleVect(5.0f);
	Ogre::Vector3 transVect(0, 5250, 0);
	
	for(int a = 0; a < nodeList.size(); a++)
	{
		Ogre::SceneNode* childNode = nodeList[a];

		//Ogre::String name = childNode->getName();
		//if(!Ogre::StringUtil::startsWith(name, "object"))
		//{			
		//	continue;
		//}

		Ogre::Vector3 pos = childNode->_getDerivedPosition();

		Ogre::Vector3 tempPos((pos.x * scaleVect.x) + transVect.x, 
							 transVect.y, 
							 (pos.z * scaleVect.z) + transVect.z);

		Ogre::Vector3 dirVector = tempPos.normalisedCopy();
		childNode->setScale(scaleVect);
		//childNode->setPosition(tempPos);

		Ogre::Vector3 actualPos = dirVector * 5000;
		
		Ogre::Vector3 intersection = Ogre::Vector3::ZERO;
		collisionDetector->getChunksIntersection(tempPos, -dirVector, intersection);

		if(intersection.x == 0 && intersection.y == 5000 && intersection.z == 0)	// ray intersection fails
		{
			childNode->setPosition(actualPos);	// guess the terrain height is 2000 from planet's center
		}
		else	// ray intersection is obtained
		{
			childNode->setPosition(intersection);
			actualPos = intersection;
		}

		Ogre::Vector3 upVector = tempPos.normalisedCopy();
		Ogre::Quaternion q = Ogre::Vector3::UNIT_Y.getRotationTo(upVector);
		childNode->setOrientation(q);
	}

	delete sceneLoader;

}
