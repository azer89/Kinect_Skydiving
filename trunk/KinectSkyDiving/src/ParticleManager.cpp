

#include "Stdafx.h"
#include "ParticleManager.h"

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
	mainNode->setPosition(characterPosition);
	Ogre::Vector3 upVector = characterPosition.normalisedCopy();
	Ogre::Quaternion orient = Ogre::Vector3::UNIT_Y.getRotationTo(upVector);
	mainNode->setOrientation(orient);	
}

//-------------------------------------------------------------------------------------
void ParticleManager::setParticleQuota(int numParticle)
{
	baseNode->detachObject(ps);
	Ogre::ParticleSystem* pClone = mSceneMgr->createParticleSystem();
	ps->copyParametersTo(pClone);
	pClone->setParameter("quota", Ogre::StringConverter::toString(numParticle));
	mSceneMgr->destroyParticleSystem(ps);
	ps = pClone;
	//ps->fastForward(5);     
	//ps->setKeepParticlesInLocalSpace(true);
	baseNode->attachObject(ps);
}

//-------------------------------------------------------------------------------------
void ParticleManager::disableParticle(void)
{
	baseNode->detachAllObjects();
}

//-------------------------------------------------------------------------------------
void ParticleManager::initParticle(Ogre::SceneManager* mSceneMgr)
{	
	this->mSceneMgr = mSceneMgr;
	//Ogre::ParticleSystem::setDefaultNonVisibleUpdateTimeout(5);	
	ps = mSceneMgr->createParticleSystem("FastWind", "Examples/FastWind");	
	//ps->fastForward(5);                 
	//ps->setKeepParticlesInLocalSpace(true);
	mainNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(Ogre::Vector3(0, 0, 0));
	baseNode = mainNode->createChildSceneNode(Ogre::Vector3(0, -500, 0));
	baseNode->attachObject(ps);
}
