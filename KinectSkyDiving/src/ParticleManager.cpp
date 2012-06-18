

#include "Stdafx.h"
#include "ParticleManager.h"

//-------------------------------------------------------------------------------------
ParticleManager::ParticleManager(void)
{
	xRot = 0;
	yRot = 0;
	zRot = 0;

	updateSpeed = GameConfig::getSingletonPtr()->getWindUpdateFactor();
	maxAngle = GameConfig::getSingletonPtr()->getWindMaxAngle();
}

//-------------------------------------------------------------------------------------
ParticleManager::~ParticleManager(void)
{
}

//-------------------------------------------------------------------------------------
void ParticleManager::update(Ogre::Real elapsedTime, Ogre::Vector3 characterPosition)
{	
	mainNode->setPosition(characterPosition);
	Ogre::Vector3 upVector = characterPosition.normalisedCopy();
	//Ogre::Quaternion orient = Ogre::Vector3::UNIT_Y.getRotationTo(upVector);
	Ogre::Quaternion orient = character->getBodyNode()->getOrientation();	

	Ogre::Real num = updateSpeed * elapsedTime;
	Ogre::Real angle = maxAngle;
	Movement mov = character->getMovement();
	if(mov == MOVE_RIGHT)			// z-axis rotation
	{
		zRot += num;
		if(zRot > Ogre::Math::HALF_PI) zRot = angle;
	}
	else if(mov == MOVE_LEFT)
	{
		zRot -= num;
		if(zRot < -Ogre::Math::HALF_PI) zRot = -angle;
	}
	else if(mov == MOVE_FRONT)	// x-axis rotation
	{
		xRot += num;
		if(xRot > Ogre::Math::HALF_PI) xRot = angle;
	}
	else if(mov == MOVE_BACK)
	{
		xRot -= num;
		if(xRot < -Ogre::Math::HALF_PI) xRot = -angle;
	}

	if(mov == MOVE_LEFT || mov == MOVE_RIGHT || mov == NOTHING)
	{
		if(xRot < 0) 
		{
			xRot += num;
			if(xRot > 0) xRot = 0;
		}
		else if (xRot > 0) 
		{
			xRot -= num;
			if(xRot < 0) xRot = 0;
		}
	}
	
	if(mov == MOVE_FRONT || mov == MOVE_BACK || mov == NOTHING)
	{
		if(zRot < 0) 
		{
			zRot += num;
			if(zRot > 0) zRot = 0;
		}
		else if (zRot > 0) 
		{
			zRot -= num;
			if(zRot < 0) zRot = 0;
		}
	}

	Ogre::Quaternion xQ = Ogre::Quaternion::IDENTITY;
	Ogre::Quaternion zQ = Ogre::Quaternion::IDENTITY;

	if(xRot != 0) xQ = Ogre::Quaternion(Ogre::Degree(xRot), Ogre::Vector3::UNIT_X);
	if(zRot != 0) zQ = Ogre::Quaternion(Ogre::Degree(zRot), Ogre::Vector3::UNIT_Z);

	mainNode->setOrientation(orient * xQ * zQ);
	//mainNode->setOrientation(orient);
}

//-------------------------------------------------------------------------------------
void ParticleManager::setSlowParticle(void)
{
	baseNode->detachObject(ps);
	ps = mSceneMgr->createParticleSystem("SlowWind", "Examples/SlowWind");	
	baseNode->attachObject(ps);
}

//-------------------------------------------------------------------------------------
/*void ParticleManager::setParticleQuota(int numParticle)
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
}*/

//-------------------------------------------------------------------------------------
void ParticleManager::disableParticle(void)
{
	baseNode->detachAllObjects();
}

//-------------------------------------------------------------------------------------
void ParticleManager::initParticle(Ogre::SceneManager* mSceneMgr, Character* character)
{	
	this->mSceneMgr = mSceneMgr;
	this->character = character;
	//Ogre::ParticleSystem::setDefaultNonVisibleUpdateTimeout(5);	
	ps = mSceneMgr->createParticleSystem("FastWind", "Examples/FastWind");	
	//ps->fastForward(5);                 
	//ps->setKeepParticlesInLocalSpace(true);
	mainNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(Ogre::Vector3(0, 0, 0));
	baseNode = mainNode->createChildSceneNode(Ogre::Vector3(0, -100, 0));
	baseNode->attachObject(ps);
}
