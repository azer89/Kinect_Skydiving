#ifndef _SIMPLEPLANET_H__
#define _SIMPLEPLANET_H__

#include "Utility.h"

#include <OgreSceneNode.h>
#include <OgreMovableObject.h>
#include <OgreRenderable.h>
#include <OgreRenderQueue.h>
#include <OgreCamera.h>
#include <OgreTimer.h>
#include <OgreLight.h>

#include <map>

namespace GalaxyEngine
{
	class SimplePlanetMesh
	{
	public:
		SimplePlanetMesh(Ogre::uint32 resolution, float radius);

		Ogre::uint32 resolution;
		float radius;

		Ogre::VertexData vertexData;
		Ogre::IndexData indexData;
	};

	class SimplePlanet: public Ogre::MovableObject
	{
	public:
		SimplePlanet(float radius, Ogre::uint32 subdivision, const Ogre::String planetName = "");
		~SimplePlanet();

		void setSunLight(Ogre::Light *light) { sunLight = light; }

		float getRadius() { return radius; }
		Ogre::uint32 getSubdivision() { return subdivision; }
		Ogre::MaterialPtr getMaterial() { return material; }
		Ogre::MaterialPtr &getMaterialRef() { return material; }
		Ogre::Technique *getBestTechnique() { return bestTechnique; }

		void _notifyCurrentCamera(Ogre::Camera *cam);
		void _updateRenderQueue(Ogre::RenderQueue *queue);
		bool isVisible();
		const Ogre::AxisAlignedBox &getBoundingBox(void) const { return bounds; }
		Ogre::Real getBoundingRadius(void) const { return radius; }
		const Ogre::String &getMovableType(void) const { static Ogre::String t = "SimplePlanet"; return t; }
		void visitRenderables(Ogre::Renderable::Visitor* visitor, bool debugRenderables) {}

		//If any SimplePlanet's have been created (and even if later deleted),
		//this MUST be called before shutting down Ogre.
		static void SimplePlanet::freeCachedMeshes();

	private:
		//Generates a sphere mesh of the given size/resolution, or returns an existing one
		SimplePlanetMesh *loadSphereMesh(Ogre::uint32 resolution);
		static std::vector<SimplePlanetMesh*> loadedMeshes;

		Ogre::Real radius;
		Ogre::AxisAlignedBox bounds;

		Ogre::uint32 subdivision;
		Ogre::MaterialPtr material;
		Ogre::Technique *bestTechnique;	//This is recalculated every frame

		bool withinFarDistance;
		Ogre::Real minDistanceSquared;

		Ogre::Light *sunLight;
		void updateLighting(Ogre::Light *light, Ogre::SceneNode *planet);

		class SimplePlanetRenderable: public Ogre::Renderable
		{
		public:
			SimplePlanetRenderable(SimplePlanet *planet, SimplePlanetMesh *mesh);

			void getRenderOperation(Ogre::RenderOperation& op);
			Ogre::Real getSquaredViewDepth(const Ogre::Camera* cam) const;
			const Ogre::LightList& getLights(void) const;

			Ogre::Technique *getTechnique() const { return planet->getBestTechnique(); }
			const Ogre::MaterialPtr &getMaterial(void) const { return planet->getMaterialRef(); }
			void getWorldTransforms(Ogre::Matrix4* xform) const { *xform = planet->_getParentNodeFullTransform(); }
			bool castsShadows(void) const { return false; }

		private:
			SimplePlanet *planet;

			//Rendering data for this chunk
			SimplePlanetMesh *meshData;
		};

		//Renderable instance
		SimplePlanetRenderable *renderable;
	};

}


#endif
