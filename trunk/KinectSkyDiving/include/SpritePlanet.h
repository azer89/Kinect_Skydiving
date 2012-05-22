#ifndef _SPRITEPLANET_H__
#define _SPRITEPLANET_H__

#include "Utility.h"
#include "PlanetMath.h"

#include <OgreSceneNode.h>
#include <OgreMovableObject.h>
#include <OgreRenderable.h>
#include <OgreRenderQueue.h>
#include <OgreCamera.h>
#include <OgreTimer.h>
#include <OgreLight.h>

namespace GalaxyEngine
{
	class SpritePlanet: public Ogre::MovableObject
	{
	public:
		SpritePlanet(float radius, const Ogre::String planetName = "");
		~SpritePlanet();

		void setSunLight(Ogre::Light *light) { sunLight = light; }
		
		void generateNormalMap();

		float getPlanetRadius() { return radius; }
		Ogre::MaterialPtr getMaterial() { return material; }
		Ogre::Technique *getBestTechnique() { return bestTechnique; }
		const Ogre::Matrix4 &getTransform() { return matrix; }

		void _notifyCurrentCamera(Ogre::Camera *cam);
		void _updateRenderQueue(Ogre::RenderQueue *queue);
		bool isVisible();
		const Ogre::AxisAlignedBox &getBoundingBox(void) const { return bounds; }
		Ogre::Real getBoundingRadius(void) const { return radius; }
		const Ogre::String &getMovableType(void) const { static Ogre::String t = "SpritePlanet"; return t; }
		void visitRenderables(Ogre::Renderable::Visitor* visitor, bool debugRenderables) {}

		Ogre::Vector3 mapCubeToPlanet(const Ogre::Vector3 &cubeCoord)
		{
			return mapCubeToSphere(cubeCoord) * radius;
		}

		static Ogre::Vector3 mapCubeToSphere(const Ogre::Vector3 &cubeCoord);
		static Ogre::Quaternion getCubeFaceTranslation(const PlanetMath::CubeFace cubeFace);

	private:
		Ogre::String loadedPlanetName;
		Ogre::Real radius;
		Ogre::AxisAlignedBox bounds;

		Ogre::MaterialPtr material;
		Ogre::Technique *bestTechnique;	//This is recalculated every frame
		Ogre::Matrix4 matrix;

		bool withinFarDistance;
		Ogre::Real minDistanceSquared;

		Ogre::Light *sunLight;
		void updateLighting(Ogre::Light *light, Ogre::SceneNode *planet);

		class SpritePlanetRenderable: public Ogre::Renderable
		{
		public:
			SpritePlanetRenderable(SpritePlanet *planet);
			~SpritePlanetRenderable();

			void getRenderOperation(Ogre::RenderOperation& op);
			Ogre::Real getSquaredViewDepth(const Ogre::Camera* cam) const;
			const Ogre::LightList& getLights(void) const;

			Ogre::Technique *getTechnique() const { return planet->getBestTechnique(); }
			const Ogre::MaterialPtr &getMaterial(void) const { return material; }
			void getWorldTransforms(Ogre::Matrix4* xform) const { *xform = planet->getTransform(); }
			bool castsShadows(void) const { return false; }

		private:
			SpritePlanet *planet;
			Ogre::Quaternion rot;
			Ogre::MaterialPtr material;

			//Rendering data for this chunk
			Ogre::VertexData vertexData;
			Ogre::IndexData indexData;
		};

		//Renderable instance
		SpritePlanetRenderable *renderable;
	};


}

#endif