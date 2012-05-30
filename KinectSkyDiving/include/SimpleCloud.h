

#ifndef __SimpleCloud_h_
#define __SimpleCloud_h_

#include "Stdafx.h"
class SimpleCloud
{
public:
	SimpleCloud(void);
	virtual ~SimpleCloud(void);

	void initCloud(Ogre::SceneManager* mSceneMgr, int numBillboards);
	void createSingleCloud(Ogre::Vector3 pos, Ogre::Vector3 scale);
	void updateClouds(Ogre::Real elapsedTime);

private:
	Ogre::Vector3 sphericalToCartesian(Ogre::Real theta, Ogre::Real phi);
	Ogre::Vector2 cartesiantoSpherical(Ogre::Vector3 pos);

private:
	Ogre::SceneManager* mSceneMgr;
	std::vector<Ogre::SceneNode*> cloudNodes;
	Ogre::Real highestElevation;
	Ogre::Real updateDelay;
};

#endif // #ifndef __SimpleCloud_h_
