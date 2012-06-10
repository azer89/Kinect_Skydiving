#ifndef _PLANET_H__
#define _PLANET_H__

#include "Utility.h"
#include "PlanetMath.h"

#include <OgreSceneNode.h>
#include <OgreMovableObject.h>
#include <OgreRenderable.h>
#include <OgreRenderQueue.h>
#include <OgreCamera.h>
#include <OgreTimer.h>
#include <OgreLight.h>
#include <list>

namespace GalaxyEngine
{
	class PlanetLoader;

	class Planet: public Ogre::MovableObject
	{
	public:
		Planet();
		~Planet();

		void setMaxPixelError(float pixelError) { maxPixelError = pixelError; }
		float getMaxPixelError() { return maxPixelError; }
		void setChunkLoader(PlanetLoader *loader) { chunkLoader = loader; updateBounds(); }
		PlanetLoader *getChunkLoader() { return chunkLoader; }
		void setSunLight(Ogre::Light *light) { sunLight = light; }
		void reload();
		void _notifyCurrentCamera(Ogre::Camera *cam);
		void _updateRenderQueue(Ogre::RenderQueue *queue);
		bool isVisible();
		const Ogre::AxisAlignedBox &getBoundingBox(void) const { return planetBounds; }
		Ogre::Real getBoundingRadius(void) const { return outerRadius; }
		const Ogre::String &getMovableType(void) const { static Ogre::String t = "Planet"; return t; }
		void visitRenderables(Ogre::Renderable::Visitor* visitor, bool debugRenderables=false) {}
		Ogre::Vector3 mapCubeToPlanet(const Ogre::Vector3 &cubeCoord, float elevation)
		{
			float heightScale = innerRadius + (elevation * heightRange);
			return PlanetMath::mapCubeToUnitSphere(cubeCoord) * heightScale;
		}
		inline static Utility::Timer &getTimer() { return timer; }

		class ChunkNode;

		Ogre::uint32 getDeepestChunkLevel()
		{
			Ogre::uint32 level = 0;

			for(int a = 0; a < GalaxyEngine::PlanetMath::CubeFace::CUBEFACE_Size; a++)
			{
				Ogre::uint32 temp =  getDeepestChunkLevel(this->getCubeFace(a));
				if(temp > level) level = temp;
			}

			return level;
		}

		Ogre::uint32 getDeepestChunkLevel(ChunkNode* chunk)
		{
			if(chunk->hasSubNodes())
			{
				Ogre::uint32 level = 0;

				for(int a = 0; a < GalaxyEngine::PlanetMath::Quadrant::QUADRANT_Size; a++)
				{
					ChunkNode* subChunk = chunk->getSubNode(a);
					Ogre::uint32 temp = getDeepestChunkLevel(subChunk);
					if(temp > level) level = temp;
				}

				return level;
			}
			else
			{
				return chunk->getChunkLevel();
			}
		}
		

	private:
		void updateBounds();

		//Timing data
		static Utility::Timer timer;
		Ogre::ulong timeOfLastRender;
		//Standard visibility / etc. info
		float innerRadius, outerRadius, heightRange;
		Ogre::AxisAlignedBox planetBounds;
		float maxPixelError;
		float perspectiveScalingFactor;
		bool withinFarDistance;
		Ogre::Real minDistanceSquared;
		//Camera data
		Ogre::Vector3 cameraPosition;	//The position of the camera relative to the planet's center
		Ogre::Camera *camera;
		//Light data
		Ogre::Light *sunLight;		
		//The 6 sides of the planet "cube"
		ChunkNode *planetFace[6];
		//Data loader class for this planet
		PlanetLoader *chunkLoader;

	public:
		ChunkNode *getCubeFace(PlanetMath::CubeFace faceID) { return planetFace[faceID]; }
		ChunkNode *getCubeFace(Ogre::uint32 faceID) { return planetFace[faceID]; }

		class ChunkNode: public Ogre::Renderable
		{
		public:
			ChunkNode(Planet *owner, ChunkNode *parent, const Ogre::FloatRect dataBounds, const PlanetMath::Quadrant subQuad, const PlanetMath::CubeFace cubeFace, Ogre::uint32 chunkLevel);
			~ChunkNode();

			void renderNodeHierarchyWithLOD(Ogre::RenderQueue *queue, bool parentChunkVisible);
			bool checkIfVisible();
			void render(Ogre::RenderQueue *queue, float distanceToCamera);
			void loadSubNodes();
			void deleteSubNodes();
			bool areAllSubNodesLoaded();
			void loadNow();
			void unloadNow();

			inline bool isLoaded() { return loaded; }
			inline bool isLoadNeeded() { return loadNeeded; }
			inline ChunkNode *getParent() { return parent; }
			inline bool hasSubNodes() { return subNodesInitialized; }
			inline ChunkNode *getSubNode(PlanetMath::Quadrant i) { return subNode[(int)i]; }
			inline ChunkNode *getSubNode(int i) { return subNode[i]; }
			inline Ogre::ulong getLastTimeUsed() { return lastTimeUsed; }
			inline Ogre::uint32 getChunkLevel() { return chunkLevel; }
			inline PlanetMath::CubeFace getCubeFace() { return cubeFace; }
			inline Ogre::FloatRect getDataBounds() { return dataBounds; }
			inline void* getUserData() { return userData; }
			inline void setUserData(void* userData) { this->userData = userData; }	
			
			void getRenderOperation(Ogre::RenderOperation& op);
			Ogre::Real getSquaredViewDepth(const Ogre::Camera* cam) const;
			const Ogre::LightList& getLights(void) const;
			Ogre::Technique *getTechnique() const { return bestTechnique; }
			const Ogre::MaterialPtr &getMaterial(void) const { return material; }
			void getWorldTransforms(Ogre::Matrix4* xform) const { *xform = planet->_getParentNodeFullTransform(); }
			bool castsShadows(void) const { return false; }

			inline Ogre::VertexData *getVertexData() { return &vertexData; }
			inline Ogre::IndexData *getIndexData() { return &indexData; }
			inline Ogre::Vector3 getCenter() { return center; }

			Ogre::Vector3 *vertices;
			unsigned long *indices;
			int vertex_size;
			int index_size;

		private:

			//This chunk's 4 sub-chunks (quad-tree)
			ChunkNode *subNode[4];
			bool subNodesInitialized;
			//Location / hierarchy data
			ChunkNode *parent;				//This chunk's parent node (if any)
			PlanetMath::Quadrant locationFromParent;	//Which sub-node of the parent node this node is at
			Ogre::uint32 chunkLevel;		//What level of subdivision this node is at, 0 being the root cube face nodes
			PlanetMath::CubeFace cubeFace;	//What face of the planet cube this node belongs to
			Ogre::FloatRect dataBounds;		//What portion of the face of the planet cube this node belongs to
			//The maximum geometric vertex error of this chunk. This is calculated when the chunk is loaded
			float maxVertexError;
			//Visibility info (bounds, radius, center, etc.)
			Ogre::AxisAlignedBox boundingBox;
			Ogre::Vector3 center;
			Ogre::Real radius;
			Ogre::Vector3 chunkNormal;
			//Rendering data for this chunk
			Ogre::VertexData vertexData;
			Ogre::IndexData indexData;
			Ogre::MaterialPtr material;
			Ogre::Technique *bestTechnique;	//This is recalculated every frame
			//Loading data
			Ogre::ulong lastTimeUsed;
			bool loadNeeded;
			bool loaded;
			//Misc.
			void* userData;
			Planet *planet;
		};
	};

	class ChunkManager
	{
	public:
		static void registerPlanet(Planet *p);
		static void unregisterPlanet(Planet *p);

		static ChunkManager *getSingletonPtr() { return singletonPtr; }
		static ChunkManager &getSingleton() { return *singletonPtr; }

		void update();

	private:
		ChunkManager();
		~ChunkManager();

		void updateNode(Planet::ChunkNode *node);

		std::list<Planet*> planetList;
		Ogre::ulong expirationTime;
		Ogre::ulong timeNow;

		static ChunkManager *singletonPtr;
	};

	class PlanetLoader
	{
	public:
		struct ChunkMesh
		{
			Ogre::VertexData *vertexData;
			Ogre::IndexData *indexData;
			Ogre::MaterialPtr material;
			Ogre::AxisAlignedBox boundingBox;
			Ogre::Vector3 center;
			Ogre::Real radius;
			float maxVertexError;
		};

		virtual ~PlanetLoader() {}
		
		virtual void loadChunkMesh(ChunkMesh &mesh, Planet::ChunkNode *chunk, Planet *planet) = 0;
		virtual void unloadChunkMesh(Planet::ChunkNode *chunk, Planet *planet) {}

		virtual Ogre::Real getInnerRadius() = 0;
		virtual Ogre::Real getOuterRadius() = 0;

		virtual Ogre::uint32 getMaxLevels() = 0;
		virtual Ogre::ulong getExpirationTime() { return 2000; }

		virtual void updateLighting(Ogre::Light *light, Ogre::SceneNode *planet) 
		{
		}
	};

}


#endif
