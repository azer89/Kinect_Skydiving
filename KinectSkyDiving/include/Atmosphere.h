#ifndef _ATMOSPHERE_H__
#define _ATMOSPHERE_H__

#include "Utility.h"

#include <OgreSceneNode.h>
#include <OgreMovableObject.h>
#include <OgreRenderable.h>
#include <OgreRenderQueue.h>
#include <OgreCamera.h>
#include <OgreTimer.h>
#include <OgreLight.h>

namespace GalaxyEngine
{
	class Atmosphere: public Ogre::MovableObject
	{
	public:
		Atmosphere(float radius, Ogre::uint32 highRes, Ogre::uint32 mediumRes, Ogre::uint32 lowRes, float maxPixelError, const Ogre::String &atmosphereMap = "");
		~Atmosphere();

		void setSunLight(Ogre::Light *light) { sunLight = light; }

		float getRadius() { return radius; }
		Ogre::MaterialPtr getMaterial() { return material; }
        Ogre::MaterialPtr &getMaterialRef() { return material; }
		Ogre::Technique *getBestTechnique() { return bestTechnique; }

		void _notifyCurrentCamera(Ogre::Camera *cam);
		void _updateRenderQueue(Ogre::RenderQueue *queue);
		bool isVisible();
		const Ogre::AxisAlignedBox &getBoundingBox(void) const { return bounds; }
		Ogre::Real getBoundingRadius(void) const { return radius; }
		const Ogre::String &getMovableType(void) const { static Ogre::String t = "Atmosphere"; return t; }
		void visitRenderables(Ogre::Renderable::Visitor* visitor, bool debugRenderables) {}

	private:
		Ogre::Real radius;
		Ogre::AxisAlignedBox bounds;

		Ogre::MaterialPtr material;
		Ogre::Technique *bestTechnique;	//This is recalculated every frame

		Ogre::Vector3 camPos;
		bool withinFarDistance;
		Ogre::Real minDistanceSquared;
		float maxPixelError;
        float perspectiveScalingFactor;

		Ogre::Light *sunLight;
		void updateLighting(Ogre::Light *light, Ogre::Camera *cam, Ogre::SceneNode *node);

		class AtmosphereRenderable: public Ogre::Renderable
		{
		public:
			AtmosphereRenderable(Atmosphere *atmosphere, Ogre::uint32 resolution);
			~AtmosphereRenderable();

			void getRenderOperation(Ogre::RenderOperation& op);
			Ogre::Real getSquaredViewDepth(const Ogre::Camera* cam) const;
			const Ogre::LightList& getLights(void) const;

			Ogre::Technique *getTechnique() const { return atmosphere->getBestTechnique(); }
			const Ogre::MaterialPtr &getMaterial(void) const { return atmosphere->getMaterialRef(); }
			void getWorldTransforms(Ogre::Matrix4* xform) const { *xform = atmosphere->_getParentNodeFullTransform(); }
			bool castsShadows(void) const { return false; }

		private:
			Atmosphere *atmosphere;

			//Rendering data for this chunk
			Ogre::VertexData vertexData;
			Ogre::IndexData indexData;
		};

		//Renderable instances (3 LODs)
		AtmosphereRenderable *lowMesh, *mediumMesh, *highMesh;
		Ogre::uint32 lowMeshError, mediumMeshError, highMeshError;
	};

}


#endif
