
#include "Stdafx.h"
#include "RayCastCollision.h"
#include "Planet.h"
#include "PlanetMath.h"

//-------------------------------------------------------------------------------------
RayCastCollision::RayCastCollision(void)
	: mRayScnQuery(0),
	  bChucksVertices(0),
      bChunksIndices(0),
	  bChunksVCount(0),
	  bChunksICount(0)
{
}

//-------------------------------------------------------------------------------------
RayCastCollision::~RayCastCollision(void)
{
	if(mRayScnQuery != 0)     delete mRayScnQuery;
	if(bChucksVertices != 0)  delete[] bChucksVertices;
	if(bChunksIndices != 0)   delete[] bChunksIndices;
}

//-------------------------------------------------------------------------------------
void RayCastCollision::init(Ogre::SceneManager* mSceneManager, GalaxyEngine::Planet* planet)
{
	mRayScnQuery = mSceneManager->createRayQuery(Ogre::Ray());
	mRayScnQuery->setSortByDistance(true);
	this->planet = planet;
}

//-------------------------------------------------------------------------------------
void RayCastCollision::crawlBaseChunks()
{
	bChunksVCount = 0;
	bChunksICount = 0;

	Ogre::uint32 level = planet->getDeepestChunkLevel();
	std::vector<GalaxyEngine::Planet::ChunkNode*> chunks;

	for(int a = 0; a < GalaxyEngine::PlanetMath::CubeFace::CUBEFACE_Size; a++)
	{
		GalaxyEngine::Planet::ChunkNode* cubeFace = planet->getCubeFace(a);
		getAllChunkNodes(cubeFace, chunks);
	}

	for(int a = 0; a < chunks.size(); a++)
	{
		GalaxyEngine::Planet::ChunkNode* cubeFace = chunks[a];
		bChunksVCount += cubeFace->vertex_size;
		bChunksICount += cubeFace->index_size;
	}

	// create arrays
	bChucksVertices = new Ogre::Vector3[bChunksVCount];
	bChunksIndices = new unsigned long[bChunksICount];

	Ogre::SceneNode* parentNode = planet->getParentSceneNode();
	Ogre::Vector3 position = parentNode->_getDerivedPosition();
	Ogre::Quaternion orient = parentNode->_getDerivedOrientation();
	Ogre::Vector3 scale = parentNode->_getDerivedScale();

	int vOffset = 0;
	int iOffset1 = 0;
	int iOffset2 = 0;

	for(int a = 0; a < chunks.size(); a++)
	{
		GalaxyEngine::Planet::ChunkNode* cubeFace = chunks[a];

		for(int b = 0;  b < cubeFace->vertex_size; b++)
		{
			bChucksVertices[vOffset] = (orient * (cubeFace->vertices[b] * scale)) + position;
			vOffset++;
		}

		for(int c = 0;  c < cubeFace->index_size; c++)
		{
			bChunksIndices[iOffset2] = cubeFace->indices[c] + static_cast<unsigned long>(iOffset1);
			iOffset2++;
		}

		iOffset1 += cubeFace->vertex_size;
	}
}

//-------------------------------------------------------------------------------------
void RayCastCollision::getChunksIntersection(const Ogre::Vector3 &point, const Ogre::Vector3 &normal, Ogre::Vector3 &result)
{
	Ogre::Ray ray(Ogre::Vector3(point.x, point.y, point.z), Ogre::Vector3(normal.x, normal.y, normal.z));

	Ogre::Real closest_distance = Ogre::Math::POS_INFINITY;

	// test for hitting individual triangles on the mesh
	for (int i = 0; i < static_cast<int>(bChunksICount); i += 3)
	{
		// check for a hit against this triangle
		std::pair<bool, Ogre::Real> hit = Ogre::Math::intersects(ray, 
			bChucksVertices[bChunksIndices[i]],
			bChucksVertices[bChunksIndices[i+1]], 
			bChucksVertices[bChunksIndices[i+2]], true, false);

		// if it was a hit check if its the closest
		if (hit.first)
		{
			if (hit.second < closest_distance)
			{
				// this is the closest so far, save it off
				closest_distance = hit.second;
			}
		}
	}

	result = ray.getPoint(closest_distance);
}

//-------------------------------------------------------------------------------------
/** traverse the quad tree */
void RayCastCollision::traverseChunkNodes(GalaxyEngine::Planet::ChunkNode* chunkNode, std::vector<GalaxyEngine::Planet::ChunkNode*> &leafChunks, Ogre::uint32 level)
{
	if(chunkNode->hasSubNodes()) // Look only on chunks on leaf
	{
		for(int a = 0; a < GalaxyEngine::PlanetMath::Quadrant::QUADRANT_Size; a++)
		{
			GalaxyEngine::Planet::ChunkNode* subChunkNode = chunkNode->getSubNode(a);
			traverseChunkNodes(subChunkNode, leafChunks, level);
		}
	}
	else if(chunkNode->isLoaded() && chunkNode->getChunkLevel() == level)	// need loaded chunk and also the deepest ones
	{
		leafChunks.push_back(chunkNode);
	}
}

//-------------------------------------------------------------------------------------
/** get all chunks recursively */
void RayCastCollision::getAllChunkNodes(GalaxyEngine::Planet::ChunkNode* chunkNode, std::vector<GalaxyEngine::Planet::ChunkNode*> &leafChunks)
{
	leafChunks.push_back(chunkNode); // insert the current chunk to array

	if(chunkNode->hasSubNodes())	// crawl the children
	{
		for(int a = 0; a < GalaxyEngine::PlanetMath::Quadrant::QUADRANT_Size; a++)
		{
			GalaxyEngine::Planet::ChunkNode* subChunkNode = chunkNode->getSubNode(a);
			getAllChunkNodes(subChunkNode, leafChunks);
		}
	}
}

//-------------------------------------------------------------------------------------
/**  Implement simple intersection algorithm using Ogre::Ray and Planet's Mesh */
void RayCastCollision::getPlanetIntersection(Ogre::Ray ray, 
											 Ogre::Vector3 &result)
{
	std::vector<GalaxyEngine::Planet::ChunkNode*> leafChunks;
	Ogre::uint32 level = planet->getDeepestChunkLevel();

	//std::cout << level << "\n";
	Ogre::SceneNode* parentNode = planet->getParentSceneNode();

	if(parentNode == NULL) { return; }

	for(int a = 0; a < GalaxyEngine::PlanetMath::CubeFace::CUBEFACE_Size; a++)
	{
		GalaxyEngine::Planet::ChunkNode* cubeFace = planet->getCubeFace(a);
		traverseChunkNodes(cubeFace, leafChunks, level);
	}	

	Ogre::Real shortestDistance = Ogre::Math::POS_INFINITY;
	GalaxyEngine::Planet::ChunkNode* chunk = 0;
	for(int a = 0; a < leafChunks.size(); a++)
	{
		GalaxyEngine::Planet::ChunkNode* tempChunk = leafChunks[a];
		Ogre::Real temp = tempChunk->getCenter().distance(ray.getOrigin());

		if(temp < shortestDistance)
		{
			shortestDistance = temp;
			chunk = tempChunk;
		}
	}

	if(chunk == 0) { return; }

	// mesh data to retrieve         
	size_t vertex_count = 0;
	size_t index_count = 0;
	Ogre::Vector3 *vertices;
	unsigned long *indices;
	vertex_count += chunk->vertex_size;
	index_count += chunk->index_size;

	vertices = new Ogre::Vector3[vertex_count];
	indices = new unsigned long[index_count];
	unsigned long vertex_offset = 0;
	unsigned long index_offset = 0;
	Ogre::Vector3 position = parentNode->_getDerivedPosition();
	Ogre::Quaternion orient = parentNode->_getDerivedOrientation();
	Ogre::Vector3 scale = parentNode->_getDerivedScale();

	for(int a = 0; a < chunk->vertex_size; a++) { vertices[a] = (orient * (chunk->vertices[a] * scale)) + position; }
	for(int a = 0; a < chunk->index_size; a++) { indices[a] = chunk->indices[a]; }

	Ogre::Real closest_distance = Ogre::Math::POS_INFINITY;

	// test for hitting individual triangles on the mesh
	for (int i = 0; i < static_cast<int>(index_count); i += 3)
	{
		// check for a hit against this triangle
		std::pair<bool, Ogre::Real> hit = Ogre::Math::intersects(ray, vertices[indices[i]],
			vertices[indices[i+1]], vertices[indices[i+2]], true, false);

		// if it was a hit check if its the closest
		if (hit.first)
		{
			if (hit.second < closest_distance)
			{
				// this is the closest so far, save it off
				closest_distance = hit.second;
			}
		}
	}

	result = ray.getPoint(closest_distance);

	// free the vertices and indices memory
	delete[] vertices;
	delete[] indices;
}

//-------------------------------------------------------------------------------------
void RayCastCollision::getPlanetIntersection(const Ogre::Vector3 &point, 
											const Ogre::Vector3 &normal, 
											Ogre::Vector3 &result)
{
	Ogre::Ray ray(Ogre::Vector3(point.x, point.y, point.z), Ogre::Vector3(normal.x, normal.y, normal.z));
	getPlanetIntersection(ray, result);
}

