#ifndef _STAR_H__
#define _STAR_H__

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
	class Star: public Ogre::MovableObject
	{
	public:
		Star(Ogre::uint32 subdivision, const Ogre::String &starColorMap);
		~Star();

		float getRadius() { return radius; }
		Ogre::uint32 getSubdivision() { return subdivision; }
		Ogre::MaterialPtr getMaterial() { return material; }
		Ogre::Technique *getBestTechnique() { return bestTechnique; }

		void _notifyCurrentCamera(Ogre::Camera *cam);
		void _updateRenderQueue(Ogre::RenderQueue *queue);
		bool isVisible();
		const Ogre::AxisAlignedBox &getBoundingBox(void) const { return bounds; }
		Ogre::Real getBoundingRadius(void) const { return radius; }
		const Ogre::String &getMovableType(void) const { static Ogre::String t = "Star"; return t; }
		void visitRenderables(Ogre::Renderable::Visitor* visitor, bool debugRenderables) {}

	private:
		void _generateNoiseMap();
		void _updateShader(Ogre::Camera *cam);
		Utility::Timer timer;

		Ogre::Real radius;
		Ogre::AxisAlignedBox bounds;

		Ogre::uint32 subdivision;
		Ogre::MaterialPtr material;
		Ogre::Technique *bestTechnique;	//This is recalculated every frame

		bool withinFarDistance;
		Ogre::Real minDistanceSquared;

		static bool noiseMapGenerated;
		
		//Animation data
		Ogre::ColourValue color;
		float animationPos;

		class StarRenderable: public Ogre::Renderable
		{
		public:
			StarRenderable(Star *star);
			~StarRenderable();

			void getRenderOperation(Ogre::RenderOperation& op);
			Ogre::Real getSquaredViewDepth(const Ogre::Camera* cam) const;
			const Ogre::LightList& getLights(void) const;

			Ogre::Technique *getTechnique() const { return star->getBestTechnique(); }
			const Ogre::MaterialPtr &getMaterial(void) const { return material; }
			void getWorldTransforms(Ogre::Matrix4* xform) const { *xform = star->_getParentNodeFullTransform(); }
			bool castsShadows(void) const { return false; }

		private:
			Star *star;

			//Rendering data for this chunk
			Ogre::VertexData vertexData;
			Ogre::IndexData indexData;
			Ogre::MaterialPtr material;
		};

		//Renderable instance
		StarRenderable *renderable;
	};

}


#endif
