

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
	this->bodyNode->attachObject(bodyEntity);

	this->bodyNode->setPosition(position);
	this->bodyNode->setScale(scale);
	this->bodyNode->setOrientation(orientation);
}

//--------------------------------------------------------------------------------------
void Character::update(Ogre::Real elapsedTime)
{
	Ogre::Vector3 upVector = this->bodyNode->getPosition();
	Ogre::Quaternion q = Ogre::Vector3::ZERO.getRotationTo(upVector);
	this->bodyNode->setOrientation(q);
}

