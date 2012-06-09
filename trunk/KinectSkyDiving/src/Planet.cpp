
#include "Stdafx.h"

#include "Planet.h"
#include "Utility.h"
#include "PlanetMath.h"
#include "QueryFlags.h"

using namespace GalaxyEngine::PlanetMath;
//using namespace Ogre;

#include <list>
using namespace std;

namespace GalaxyEngine
{
	Utility::Timer Planet::timer;

	Planet::Planet()
	 :	
		withinFarDistance(false),
		minDistanceSquared(0),
		chunkLoader(NULL),
		sunLight(NULL)
	{
		//Default pixel error
		maxPixelError = 20;

		//Default radius / bounding box
		updateBounds();

		//Initialize the 6 faces of the planet "cube"
		const Ogre::FloatRect globalBounds(0, 0, 1, 1);
		planetFace[CUBEFACE_Front]	= new ChunkNode(this, NULL, globalBounds, (Quadrant)0, CUBEFACE_Front, 0);
		planetFace[CUBEFACE_Back]	= new ChunkNode(this, NULL, globalBounds, (Quadrant)0, CUBEFACE_Back, 0);
		planetFace[CUBEFACE_Right]	= new ChunkNode(this, NULL, globalBounds, (Quadrant)0, CUBEFACE_Right, 0);
		planetFace[CUBEFACE_Left]	= new ChunkNode(this, NULL, globalBounds, (Quadrant)0, CUBEFACE_Left, 0);
		planetFace[CUBEFACE_Top]	= new ChunkNode(this, NULL, globalBounds, (Quadrant)0, CUBEFACE_Top, 0);
		planetFace[CUBEFACE_Bottom]	= new ChunkNode(this, NULL, globalBounds, (Quadrant)0, CUBEFACE_Bottom, 0);

		ChunkManager::registerPlanet(this);
		
		//this->setQueryFlags(1<<0);

	}

	Planet::~Planet()
	{
		ChunkManager::unregisterPlanet(this);

		//Delete the 6 faces of the planet "cube"
		for (Ogre::uint32 i = 0; i < 6; ++i) {
			delete planetFace[i];
		}
	}

	void Planet::_notifyCurrentCamera(Ogre::Camera *cam)
	{
		if (getRenderingDistance() == 0) 
		{
			withinFarDistance = true;
		} 
		else 
		{
			//Calculate camera distance
			Ogre::Vector3 camVec = cam->getDerivedPosition() - getParentSceneNode()->_getDerivedPosition();
			Ogre::Real centerDistanceSquared = camVec.squaredLength();
			minDistanceSquared = std::max(0.0f, centerDistanceSquared - (outerRadius * outerRadius));
			//Note: centerDistanceSquared measures the distance between the camera and the center of the planet,
			//while minDistanceSquared measures the closest distance between the camera and the closest vertex of
			//the planet.

			//Determine whether the planet is within the far rendering distance
			withinFarDistance = minDistanceSquared <= Ogre::Math::Sqr(getRenderingDistance());
		}

		//Calculate the "perspective scaling factor" (used in LOD calculations)
		float viewportHeight = cam->getViewport()->getActualHeight();
		perspectiveScalingFactor = viewportHeight / (2 *Ogre:: Math::Tan(cam->getFOVy()/2));

		//Store camera position data. The camera position is translated to be relative to the
		//planet's center for easier LOD calculation.
		cameraPosition = cam->getDerivedPosition() - getParentSceneNode()->_getDerivedPosition();
		cameraPosition = getParentSceneNode()->getOrientation().Inverse() * cameraPosition;
		camera = cam;

		//Update the planet's lighting calculations
		if (chunkLoader && sunLight) 
		{
			chunkLoader->updateLighting(sunLight, getParentSceneNode());
		}
	}

	bool Planet::isVisible()
	{
		return mVisible && withinFarDistance;
	}

	void Planet::_updateRenderQueue(Ogre::RenderQueue *queue)
	{
		//Unit scale required
		getParentSceneNode()->setScale(Ogre::Vector3::UNIT_SCALE);

		//Update and render the planet
		if (chunkLoader) {
			updateBounds();

			if (isVisible()) {
				timeOfLastRender = timer.getMilliseconds();   //for use in node expiration calculations, etc.
				
				//Render all 6 faces of the planet "cube"
				for (Ogre::uint32 i = 0; i < 6; ++i) 
				{
					planetFace[i]->renderNodeHierarchyWithLOD(queue, true);
				}
			}
		}
	}

	void Planet::updateBounds()
	{
		if (chunkLoader) 
		{
			innerRadius = chunkLoader->getInnerRadius();
			outerRadius = chunkLoader->getOuterRadius();
			heightRange = outerRadius - innerRadius;
			planetBounds = Ogre::AxisAlignedBox(-outerRadius, -outerRadius, -outerRadius, outerRadius, outerRadius, outerRadius);
		} 
		else 
		{
			innerRadius = 0.0f;
			outerRadius = 0.0f;
			heightRange = 0.0f;
			planetBounds = Ogre::AxisAlignedBox();
		}
	}

	void Planet::reload()
	{
		for (Ogre::uint32 i = 0; i < 6; ++i) 
		{
			planetFace[i]->deleteSubNodes();
		}

        //The nodes will be reloaded automatically once they've been deleted
	}


	Planet::ChunkNode::ChunkNode(Planet *owner, ChunkNode *parent, const Ogre::FloatRect dataBounds, const Quadrant subQuad, const CubeFace cubeFace, Ogre::uint32 chunkLevel)
	{
		planet = owner;
		this->parent = parent;
		locationFromParent = subQuad;

		userData = NULL;
		maxVertexError = 0;

		subNode[0] = NULL;
		subNode[1] = NULL;
		subNode[2] = NULL;
		subNode[3] = NULL;
		subNodesInitialized = false;
		
		ChunkNode::dataBounds = dataBounds;
		ChunkNode::cubeFace = cubeFace;
		ChunkNode::chunkLevel = chunkLevel;

		lastTimeUsed = Planet::timer.getMilliseconds();
		loaded = false;
		loadNeeded = false;

		//Calculate the chunk normal
		float midX = (dataBounds.right+dataBounds.left)*0.5f;
		float midY = (dataBounds.top+dataBounds.bottom)*0.5f;
		chunkNormal = mapCubeToUnitSphere(mapPlaneToCube(midX, midY, cubeFace));

		vertices = 0;
		indices = 0;
		vertex_size = 0;
		index_size = 0;
	}

	Planet::ChunkNode::~ChunkNode()
	{
		if (parent)
			parent->subNodesInitialized = false;
		unloadNow();
		if (subNodesInitialized) {
			delete subNode[0];
			delete subNode[1];
			delete subNode[2];
			delete subNode[3];
		}

		if(vertices != 0) delete vertices;
		if(indices != 0) delete indices;
	}

	void Planet::ChunkNode::renderNodeHierarchyWithLOD(Ogre::RenderQueue *queue, bool parentChunkVisible)
	{
		if (!loaded) return;

		//Update this chunk's lastTimeUsed value, so it won't expire
		lastTimeUsed = planet->timeOfLastRender;

		//Get the camera's position, relative to the planet
		Ogre::Vector3 camPos = planet->cameraPosition;

		//Check if the chunk is visible
		bool chunkVisible;
		if (parentChunkVisible)
			chunkVisible = checkIfVisible();
		else
			chunkVisible = false;

		//Calculate pixel error
		float distance = std::max(0.00001f, center.distance(camPos) - radius);
		float pixelError = planet->perspectiveScalingFactor * maxVertexError / distance;
		bool acceptablePixelError = (pixelError <= planet->maxPixelError);
		
		//If possible (or necessary), render this chunk, otherwise recurse to the sub-nodes
		if (acceptablePixelError || !areAllSubNodesLoaded()) {
			if (chunkVisible)
				render(queue, distance);
			loadSubNodes();
		}
		else {
			for (Ogre::uint32 i = 0; i < 4; ++i) {
				subNode[i]->renderNodeHierarchyWithLOD(queue, chunkVisible);
			}
		}

	}

	bool Planet::ChunkNode::checkIfVisible()
	{
		//Warning: This function assumes the planet is at (0,0,0) and not rotated
		bool chunkVisible = true;

		//Bounding sphere and bounding box frustum check
		if (!planet->camera->isVisible(Ogre::Sphere(center, radius)))
			chunkVisible = false;
		else
			if (!planet->camera->isVisible(boundingBox))
				chunkVisible = false;

		//Horizon culling check
		if (chunkVisible) {
			Ogre::Vector3 camPos = planet->cameraPosition;
			Ogre::Vector3 camVec = camPos.normalisedCopy();
			float camHeight = camPos.length();

			Ogre::Radian cosAng = Ogre::Math::ACos(chunkNormal.dotProduct(camVec)) - Ogre::Math::ATan(dataBounds.width());
			Ogre::Degree theta = Ogre::Math::ACos(planet->innerRadius / camHeight) + Ogre::Degree(5);

			if (cosAng > theta)
				chunkVisible = false;
		}

		return chunkVisible;
	}

	void Planet::ChunkNode::render(Ogre::RenderQueue *queue, float distanceToCamera)
	{
		if (!material.isNull()) {
			bestTechnique = material->getBestTechnique(material->getLodIndex(distanceToCamera*distanceToCamera));
			if (!bestTechnique) {
				material->load();
				bestTechnique = material->getBestTechnique(material->getLodIndex(distanceToCamera*distanceToCamera));
			}
		} else
			bestTechnique = NULL;

		queue->addRenderable(this);
	}

	void Planet::ChunkNode::loadSubNodes()
	{
		//If not already done, divide this chunk into 4 sub-chunks (2x2)
		if (chunkLevel < planet->chunkLoader->getMaxLevels()) {
			if (!subNodesInitialized) {
				float top = dataBounds.top;
				float left = dataBounds.left;
				float ww = dataBounds.width();
				float hh = dataBounds.height();
				float w = ww * 0.5f;
				float h = hh * 0.5f;

				subNode[QUADRANT_TopLeft] = new ChunkNode(planet, this, Ogre::FloatRect(left, top, left+w, top+h), QUADRANT_TopLeft, cubeFace, chunkLevel+1);
				subNode[QUADRANT_TopRight] = new ChunkNode(planet, this, Ogre::FloatRect(left+w, top, left+ww, top+h), QUADRANT_TopRight, cubeFace, chunkLevel+1);
				subNode[QUADRANT_BottomLeft] = new ChunkNode(planet, this, Ogre::FloatRect(left, top+h, left+w, top+hh), QUADRANT_BottomLeft, cubeFace, chunkLevel+1);
				subNode[QUADRANT_BottomRight] = new ChunkNode(planet, this, Ogre::FloatRect(left+w, top+h, left+ww, top+hh), QUADRANT_BottomRight, cubeFace, chunkLevel+1);
				subNodesInitialized = true;
			}
		}

		//This chunk, and it's immediate children need to be loaded, while sub-nodes beyond that don't.
		//This marks the nodes appropriately so the threaded loader knows how much to load, and makes sure
		//the sub-nodes don't expire due to the fact that they aren't being used.
		loadNeeded = true;
		if (subNodesInitialized) {
			for (Ogre::uint32 i = 0; i < 4; ++i) {
				ChunkNode *node = subNode[i];
				node->loadNeeded = true;
				node->lastTimeUsed = planet->timeOfLastRender;

				if (node->subNodesInitialized) {
					node->subNode[0]->loadNeeded = false;
					node->subNode[1]->loadNeeded = false;
					node->subNode[2]->loadNeeded = false;
					node->subNode[3]->loadNeeded = false;
				}
			}
		}
	}

	void Planet::ChunkNode::deleteSubNodes()
	{
		if (subNodesInitialized) {
			delete subNode[0];
			delete subNode[1];
			delete subNode[2];
			delete subNode[3];
			subNode[0] = NULL;
			subNode[1] = NULL;
			subNode[2] = NULL;
			subNode[3] = NULL;
			subNodesInitialized = false;
		}
	}

	bool Planet::ChunkNode::areAllSubNodesLoaded()
	{
		if (subNodesInitialized) {
			if (!subNode[0]->isLoaded()) return false;
			else if (!subNode[1]->isLoaded()) return false;
			else if (!subNode[2]->isLoaded()) return false;
			else if (!subNode[3]->isLoaded()) return false;
			else return true;
		}
		else return false;
	}


	void Planet::ChunkNode::loadNow()
	{
		if (!loaded && planet->chunkLoader != NULL) {
			PlanetLoader::ChunkMesh mesh;
			mesh.vertexData = &vertexData;
			mesh.indexData = &indexData;
			
			planet->chunkLoader->loadChunkMesh(mesh, this, planet);

			boundingBox = mesh.boundingBox;
			center = mesh.center;
			radius = mesh.radius;
			maxVertexError = mesh.maxVertexError;

			material = mesh.material;

			loaded = true;
		}
	}

	void Planet::ChunkNode::unloadNow()
	{
		if (loaded) {
			vertexData.vertexStart = 0;
			vertexData.vertexCount = 0;
			vertexData.vertexDeclaration->removeAllElements();
			vertexData.vertexBufferBinding->unsetAllBindings();

			indexData.indexStart = 0;
			indexData.indexCount = 0;
			indexData.indexBuffer.setNull();

			loaded = false;
		}
	}

	void Planet::ChunkNode::getRenderOperation(Ogre::RenderOperation& op)
	{
		op.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
		op.srcRenderable = this;
		op.useIndexes = true;
		op.vertexData = &vertexData;
		op.indexData = &indexData;
	}

	Ogre::Real Planet::ChunkNode::getSquaredViewDepth(const Ogre::Camera* cam) const
	{
		Ogre::Vector3 camVec = cam->getDerivedPosition() - planet->getParentSceneNode()->_getDerivedPosition();
		return camVec.squaredLength();
	}

	const Ogre::LightList& Planet::ChunkNode::getLights(void) const
	{
		return planet->queryLights();
	}



	ChunkManager *ChunkManager::singletonPtr = NULL;

	void ChunkManager::registerPlanet(Planet *p)
	{
		//Create ChunkManager if none exists, and add planet to planetList
		if (singletonPtr == NULL)
			singletonPtr = new ChunkManager();

		getSingleton().planetList.push_back(p);
	}

	void ChunkManager::unregisterPlanet(Planet *p)
	{
		//Remove planet from planetList, and delete ChunkManager if no more planets exist
		getSingleton().planetList.remove(p);

		if (getSingleton().planetList.empty()) {
			delete singletonPtr;
			singletonPtr = NULL;
		}
	}

	ChunkManager::ChunkManager()
	{
	}

	ChunkManager::~ChunkManager()
	{
	}

	void ChunkManager::update()
	{
		if (!planetList.empty()) {
			timeNow = Planet::getTimer().getMilliseconds();

			std::list<Planet*>::iterator i;
			for (i = planetList.begin(); i != planetList.end(); ++i) 
			{
				Planet *planet = *i;

				expirationTime = planet->getChunkLoader()->getExpirationTime();
				for (Ogre::uint32 o = 0; o < 6; ++o) 
				{
					updateNode(planet->getCubeFace(o));
				}
			}
		}
	}

	void ChunkManager::updateNode(Planet::ChunkNode *node)
	{
		node->loadNow();

		if (node->hasSubNodes()) 
		{
			//If all subnodes have expired, delete them
			Ogre::ulong inactiveTime = timeNow - node->getSubNode(QUADRANT_TopLeft)->getLastTimeUsed();
			if (inactiveTime >= expirationTime) 
			{
				node->deleteSubNodes();
			}
			else 
			{
				//If not expired, then continue to update them
				for (Ogre::uint32 i = 0; i < 4; ++i) 
				{
					Planet::ChunkNode *subNode = node->getSubNode((Quadrant)i);
					if (subNode && subNode->isLoadNeeded())
						updateNode(subNode);
				}
			}
		}
	}

}
