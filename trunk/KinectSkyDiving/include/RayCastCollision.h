

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

	void init(Ogre::SceneManager* mSceneManager, GalaxyEngine::Planet* planet);

	void getPlanetIntersection(const Ogre::Vector3 &point, const Ogre::Vector3 &normal, Ogre::Vector3 &result);
	void getPlanetIntersection(Ogre::Ray ray, Ogre::Vector3 &result);
	void getChunksIntersection(const Ogre::Vector3 &point, const Ogre::Vector3 &normal, Ogre::Vector3 &result);

	/** Get vertices and indices of base chunks */
	void crawlBaseChunks(); 

private:
	Ogre::RaySceneQuery* mRayScnQuery;

	GalaxyEngine::Planet* planet;
	Ogre::Vector3* bChucksVertices;		// vertices of six cube faces
	unsigned long* bChunksIndices;		// indices of six cube faces
	size_t bChunksVCount;               // number of vertices of six cube faces
	size_t bChunksICount;				// number of indices of six cube faces

private:
	/** only look at deepest chunk */
	void traverseChunkNodes(GalaxyEngine::Planet::ChunkNode* chunkNode, std::vector<GalaxyEngine::Planet::ChunkNode*> &leafChunks, Ogre::uint32 level);
	
	/** get all chunks */
	void getAllChunkNodes(GalaxyEngine::Planet::ChunkNode* chunkNode, std::vector<GalaxyEngine::Planet::ChunkNode*> &leafChunks);
};

#endif // #ifndef __RayCastCollision_h_
