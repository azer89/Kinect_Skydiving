

#include "Stdafx.h"
#include "ParticleManager.h"
#include <sstream>

//-------------------------------------------------------------------------------------
ParticleManager::ParticleManager(void)
{
}

//-------------------------------------------------------------------------------------
ParticleManager::~ParticleManager(void)
{
}

//-------------------------------------------------------------------------------------
void ParticleManager::update(Ogre::Vector3 characterPosition)
{
	/*
	mainNode->setPosition(characterPosition);
	Ogre::Vector3 upVector = characterPosition.normalisedCopy();
	Ogre::Quaternion orient = Ogre::Vector3::UNIT_Y.getRotationTo(upVector);
	mainNode->setOrientation(orient);
	*/
}

//-------------------------------------------------------------------------------------
void ParticleManager::initParticle(Ogre::SceneManager* mSceneMgr)
{
	/*
	Ogre::ParticleSystem::setDefaultNonVisibleUpdateTimeout(5);				  // set non visible timeout
	ps = mSceneMgr->createParticleSystem("WindCloud", "Examples/WindCloud");  // create a rainstorm
	ps->fastForward(5);                                                       // fast-forward the rain so it looks more natural
	ps->setKeepParticlesInLocalSpace(true);
	mainNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(Ogre::Vector3(0, 0, 0));
	baseNode = mainNode->createChildSceneNode(Ogre::Vector3(0, -500, 0));
	baseNode->attachObject(ps);
	*/
}
