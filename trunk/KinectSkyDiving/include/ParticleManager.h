
#ifndef __ParticleManager_h_
#define __ParticleManager_h_

#include "Stdafx.h"
#include "Character.h"
#include "GameConfig.h"

class ParticleManager
{
public:
	ParticleManager(void);
	virtual ~ParticleManager(void);

	void initParticle(Ogre::SceneManager* mSceneMgr, Character* character);
	void disableParticle(void);
	void update(Ogre::Real elapsedTime, Ogre::Vector3 characterPosition);
	void setSlowParticle(void);

private:
	Character* character;

	Ogre::SceneManager* mSceneMgr;
	Ogre::ParticleSystem* ps;
	Ogre::SceneNode* baseNode;
	Ogre::SceneNode* mainNode;
	
	Ogre::Real xRot;
	Ogre::Real yRot;
	Ogre::Real zRot;

	Ogre::Real updateSpeed;
	Ogre::Real maxAngle;
};

#endif // #ifndef __ParticleManager_h_
