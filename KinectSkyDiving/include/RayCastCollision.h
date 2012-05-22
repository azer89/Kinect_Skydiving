

#ifndef __RayCastCollision_h_
#define __RayCastCollision_h_

#include "Stdafx.h"

#include "Exception.h"
#include "Planet.h"

/**
	Simple class similar to RaySceneQuery to get ray intersection on the planet surface
	It works by reading the vertex position and vertex index
*/
class RayCastCollision
{
public:
	RayCastCollision(void);
	virtual ~RayCastCollision(void);

	void init(Ogre::SceneManager* mSceneManager);
	void getPlanetIntersection(GalaxyEngine::Planet* planet, const Ogre::Vector3 &point, const Ogre::Vector3 &normal, Ogre::Vector3 &result);
	void getPlanetIntersection(GalaxyEngine::Planet* planet, Ogre::Ray ray, Ogre::Vector3 &result);

private:
	Ogre::RaySceneQuery* mRaySceneQuery;

private:
};

#endif // #ifndef __RayCastCollision_h_
