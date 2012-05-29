

#ifndef __SimpleCloud_h_
#define __SimpleCloud_h_

#include "Stdafx.h"
class SimpleCloud
{
public:
	SimpleCloud(void);
	virtual ~SimpleCloud(void);

	void initCloud(Ogre::SceneManager* mSceneMgr, int numBillboards);
	void createSingleCloud(Ogre::Vector3 pos);
	void updateClouds(Ogre::Real elapsedTime);

private:
	Ogre::Vector3 sphericalToCartesianNormal(Ogre::Real theta, Ogre::Real phi);

private:
	Ogre::SceneManager* mSceneMgr;
	std::vector<Ogre::SceneNode*> cloudNodes;
};

#endif // #ifndef __SimpleCloud_h_
