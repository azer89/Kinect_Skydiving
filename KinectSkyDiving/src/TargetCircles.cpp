
#include "Stdafx.h"
#include "TargetCircles.h"

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

	Ogre::DotSceneLoader* sceneLoader = new Ogre::DotSceneLoader();
	sceneLoader->parseDotScene("circles.scene", "Popular", mSceneManager, mMainNode);
	std::vector<Ogre::SceneNode*> nodeList = sceneLoader->nodeList;
	
	Ogre::Vector3 targetPoint(0, 2050, 100);
	Ogre::Vector3 charPos(0, 2700, 2700);
	Ogre::Vector3 lastPos = nodeList[nodeList.size() - 1]->_getDerivedPosition();

	if(lastPos.z == 0) lastPos.z = 1;
	if(lastPos.y == 0) lastPos.y = 1;

	Ogre::Vector3 diff = charPos - targetPoint;

	Ogre::Real scaleZ = diff.z / lastPos.z;
	Ogre::Real scaleY = diff.y / lastPos.y;
	Ogre::Real scaleX = (scaleZ + scaleY) / 3.0f;

	Ogre::Vector3 posScale(scaleX, 
						   scaleY, 
						   scaleZ);
	Ogre::Vector3 objScale(2.5f);
	

	for(int a = 0; a < nodeList.size(); a++)
	{
		Ogre::SceneNode* childNode = nodeList[a];

		Ogre::Vector3 pos = childNode->_getDerivedPosition();

		Ogre::Vector3 newPos((pos.x * posScale.x) + targetPoint.x, 
							 (pos.y * posScale.y)  + targetPoint.y, 
							 (pos.z * posScale.z) + targetPoint.z);
				

		Ogre::Vector3 upVector = newPos;
		upVector.normalise();
		Ogre::Quaternion q = Ogre::Vector3::UNIT_Y.getRotationTo(upVector);

		childNode->setPosition(newPos);
		childNode->setOrientation(q);
		childNode->setScale(objScale);
	}

	delete sceneLoader;

}


