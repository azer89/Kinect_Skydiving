
#ifndef __ParticleManager_h_
#define __ParticleManager_h_

#include "Stdafx.h"
class ParticleManager
{
public:
	ParticleManager(void);
	virtual ~ParticleManager(void);

	void initParticle(Ogre::SceneManager* mSceneMgr);

	void update(Ogre::Vector3 characterPosition);

private:
	Ogre::ParticleSystem* ps;
	Ogre::SceneNode* baseNode;
	Ogre::SceneNode* mainNode;
};

#endif // #ifndef __ParticleManager_h_
