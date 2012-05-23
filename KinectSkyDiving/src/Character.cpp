

#include "Stdafx.h"
#include "Character.h"

//-------------------------------------------------------------------------------------
Character::Character(void)
{
}

//-------------------------------------------------------------------------------------
Character::~Character(void)
{
}

//-------------------------------------------------------------------------------------
/**  Set up Node, Entity, etc.*/
void Character::setup(Ogre::SceneManager* mSceneManager, Ogre::Vector3 position, Ogre::Vector3 scale, Ogre::Quaternion orientation)
{
	this->mSceneManager = mSceneManager;

	this->entityName = "MainBodyCharacter";

	this->bodyEntity = mSceneManager->createEntity(entityName, "bomberman.mesh");
	this->bodyNode = mSceneManager->getRootSceneNode()->createChildSceneNode();
	//this->bodyNode->attachObject(bodyEntity);

	Ogre::SceneNode* innerNode = new Ogre::SceneNode(mSceneManager);
	innerNode->attachObject(bodyEntity);
	Ogre::Quaternion q1;
	Ogre::Quaternion q2;
	q1.FromAngleAxis(Ogre::Degree(180), Ogre::Vector3::UNIT_Y);
	q2.FromAngleAxis(Ogre::Degree(180), Ogre::Vector3::UNIT_X);
	innerNode->setOrientation(q1 * q2);
	this->bodyNode->addChild(innerNode);

	this->bodyNode->setPosition(position);
	this->bodyNode->setScale(scale);
	this->bodyNode->setOrientation(orientation);
}

//--------------------------------------------------------------------------------------
void Character::update(Ogre::Real elapsedTime)
{
	Ogre::Vector3 upVector = this->bodyNode->getPosition();
	//Ogre::Quaternion q = Ogre::Vector3::ZERO.getRotationTo(upVector);
	//this->bodyNode->setOrientation(q);
	//this->bodyNode->setDirection(upVector);
	this->bodyNode->lookAt(upVector * 10, Ogre::Node::TransformSpace::TS_WORLD);
}

