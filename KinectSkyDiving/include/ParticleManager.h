
#ifndef __ParticleManager_h_
#define __ParticleManager_h_

#include "Stdafx.h"
class ParticleManager
{
public:
	ParticleManager(void);
	virtual ~ParticleManager(void);

	void initParticle(Ogre::SceneManager* mSceneMgr);
	void disableParticle(void);
	void update(Ogre::Vector3 characterPosition);
	//void setParticleQuota(int numParticle);
	void setSlowParticle(void);

private:
	Ogre::SceneManager* mSceneMgr;
	Ogre::ParticleSystem* ps;
	Ogre::SceneNode* baseNode;
	Ogre::SceneNode* mainNode;
	
	Ogre::Degree xRot;
	Ogre::Degree yRot;
	Ogre::Degree zRot;
};

#endif // #ifndef __ParticleManager_h_
