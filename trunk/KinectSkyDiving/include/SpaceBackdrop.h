#ifndef _SPACEBACKDROP_H__
#define _SPACEBACKDROP_H__

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
	class SpaceBackdrop: public Ogre::MovableObject
	{
	public:
		SpaceBackdrop(float radius, Ogre::uint32 subdivision);
		~SpaceBackdrop();

		inline static SpaceBackdrop &getSingleton() { return *singletonPtr; }
		inline static SpaceBackdrop *getSingletonPtr() { return singletonPtr; }

		void setBackdropImage(const Ogre::String &fileName);
		void setRotation(const Ogre::Quaternion &rot) { node->setOrientation(rot); }

		Ogre::uint32 getSubdivision() { return subdivision; }
		Ogre::MaterialPtr getMaterial() { return material; }
		Ogre::Technique *getBestTechnique() { return bestTechnique; }

		void _notifyCurrentCamera(Ogre::Camera *cam);
		void _updateRenderQueue(Ogre::RenderQueue *queue);
		bool isVisible();
		const Ogre::AxisAlignedBox &getBoundingBox(void) const { return bounds; }
		Ogre::Real getBoundingRadius(void) const { return radius; }
		const Ogre::String &getMovableType(void) const { static Ogre::String t = "SpaceBackdrop"; return t; }
		void visitRenderables(Ogre::Renderable::Visitor* visitor, bool debugRenderables) {}

	private:
		static SpaceBackdrop *singletonPtr;

		Ogre::SceneNode *node;

		Ogre::Real radius;
		Ogre::AxisAlignedBox bounds;

		Ogre::uint32 subdivision;
		Ogre::MaterialPtr material;
		Ogre::Technique *bestTechnique;	//This is recalculated every frame

		bool withinFarDistance;
		Ogre::Real minDistanceSquared;

		class SpaceBackdropRenderable: public Ogre::Renderable
		{
		public:
			SpaceBackdropRenderable(SpaceBackdrop *backdrop);
			~SpaceBackdropRenderable();

			void getRenderOperation(Ogre::RenderOperation& op);
			Ogre::Real getSquaredViewDepth(const Ogre::Camera* cam) const;
			const Ogre::LightList& getLights(void) const;

			Ogre::Technique *getTechnique() const { return backdrop->getBestTechnique(); }
			const Ogre::MaterialPtr &getMaterial(void) const { return material; }
			void getWorldTransforms(Ogre::Matrix4* xform) const { *xform = backdrop->_getParentNodeFullTransform(); }
			bool castsShadows(void) const { return false; }

		private:
			SpaceBackdrop *backdrop;

			//Rendering data for this chunk
			Ogre::VertexData vertexData;
			Ogre::IndexData indexData;
			Ogre::MaterialPtr material;
		};

		//Renderable instance
		SpaceBackdropRenderable *renderable;
	};

}


#endif
