

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

	this->bodyEntity = this->mSceneManager->createEntity(entityName, "bomberman.mesh");
	this->bodyNode = this->mSceneManager->getRootSceneNode()->createChildSceneNode();
	this->bodyNode->attachObject(bodyEntity);

	//this->bodyNode->setPosition(position);
	//this->bodyNode->setScale(scale);
	//this->bodyNode->setOrientation(orientation);
}

