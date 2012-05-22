
#include "Stdafx.h"

#include "SpaceBackdrop.h"
#include "Utility.h"
#include "PlanetMath.h"
#include "Exception.h"
#include "Core.h"


using namespace Ogre;

namespace GalaxyEngine
{
	SpaceBackdrop *SpaceBackdrop::singletonPtr = NULL;

	SpaceBackdrop::SpaceBackdrop(float radius, uint32 subdivision)
		: withinFarDistance(false),
		minDistanceSquared(0),
		bestTechnique(NULL)
	{
		//Only one instance allowed
		if (singletonPtr)
			EXCEPTION("Cannot create more than one instance of a singleton class", "SpaceBackdrop::SpaceBackdrop()");
		singletonPtr = this;

		SpaceBackdrop::subdivision = subdivision;
		SpaceBackdrop::radius = radius;
		bounds.setMinimum(-radius, -radius, -radius);
		bounds.setMaximum(radius, radius, radius);

		material = MaterialManager::getSingleton().create(Utility::getUniqueID(), ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

		renderable = new SpaceBackdropRenderable(this);

		node = Core::getSingleton().getSceneManager()->getRootSceneNode()->createChildSceneNode("SpaceBackdropNode");
		node->attachObject(this);

		//Ogre::Entity spaceBackdropEntity = Core::getSingleton().getSceneManager()->createEntity()
		
	}

	SpaceBackdrop::~SpaceBackdrop()
	{
		Core::getSingleton().getSceneManager()->destroySceneNode(node->getName());
		delete renderable;
	}

	void SpaceBackdrop::setBackdropImage(const String &fileName)
	{
		Pass *pass = material->getTechnique(0)->getPass(0);
		pass->setLightingEnabled(false);
		pass->setCullingMode(CULL_NONE);
		TextureUnitState *t = pass->createTextureUnitState(fileName);
		t->setNumMipmaps(0);
	}

	void SpaceBackdrop::_notifyCurrentCamera(Camera *cam)
	{
		//Calculate camera distance
		Vector3 camVec = cam->getDerivedPosition() - getParentSceneNode()->_getDerivedPosition();
		Real centerDistanceSquared = camVec.squaredLength();

		withinFarDistance = true;

		//Calculate the best material technique
		bestTechnique = material->getBestTechnique(material->getLodIndex(centerDistanceSquared));
		if (!bestTechnique){
			material->load();
			bestTechnique = material->getBestTechnique(material->getLodIndex(centerDistanceSquared));
		}

		//Center around the camera
		node->setPosition(cam->getDerivedPosition());
	}

	bool SpaceBackdrop::isVisible()
	{
		return mVisible && withinFarDistance;
	}

	void SpaceBackdrop::_updateRenderQueue(RenderQueue *queue)
	{
		queue->addRenderable(renderable);
	}

	SpaceBackdrop::SpaceBackdropRenderable::SpaceBackdropRenderable(SpaceBackdrop *backdrop)
	{
		//Save misc. variables
		this->backdrop = backdrop;
		uint32 tiles = backdrop->getSubdivision();
		float invTiles = 1.0f / tiles;

		//Set material
		material = backdrop->getMaterial();

		//Load vertex buffer
		uint32 vertCount = 6 * (tiles+1) * (tiles+1);
		assert(vertCount < 0xFFFF);
		size_t offset = 0;
		vertexData.vertexDeclaration->addElement(0, offset, VET_FLOAT3, VES_POSITION);
		offset += VertexElement::getTypeSize(VET_FLOAT3);
		vertexData.vertexDeclaration->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES);
		HardwareVertexBufferSharedPtr vertBuff = HardwareBufferManager::getSingleton().createVertexBuffer(
			vertexData.vertexDeclaration->getVertexSize(0), vertCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);

		float *vBuff = static_cast<float*>(vertBuff->lock(HardwareBuffer::HBL_DISCARD));

		for (uint32 face = 0; face < 6; ++face) {
			float faceNormalOffsetU = (face % 3) / 4.0f;
			float faceNormalOffsetV = ((uint32)face / (uint32)3) / 2.0f;

			for (uint32 y = 0; y <= tiles; ++y){
				for (uint32 x = 0; x <= tiles; ++x){
					//Calculate UVs and terrain height
					Real tx = x * invTiles;
					Real ty = y * invTiles;

					//Calculate vertex position
					Vector3 pos = PlanetMath::mapCubeToUnitSphere(mapPlaneToCube(tx, ty, (PlanetMath::CubeFace)face)) * backdrop->radius;

					//Add vertex
					*vBuff++ = pos.x; *vBuff++ = pos.y; *vBuff++ = pos.z;
					*vBuff++ = tx; *vBuff++ = ty;
				}
			}
		}

		vertBuff->unlock();
		vertexData.vertexStart = 0;
		vertexData.vertexCount = vertCount;
		vertexData.vertexBufferBinding->setBinding(0, vertBuff);

		//Load index buffer
		uint32 indexCount = 6 * 6 * tiles * tiles;
		HardwareIndexBufferSharedPtr indexBuff = HardwareBufferManager::getSingleton().createIndexBuffer(
			HardwareIndexBuffer::IT_16BIT, indexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);

		uint16 *iBuff = static_cast<uint16*>(indexBuff->lock(HardwareBuffer::HBL_DISCARD));

		//Index the chunk grid
		for (uint32 face = 0; face < 6; ++face) {
			uint32 faceOffset = face * ((tiles+1) * (tiles+1));
			for (uint32 y = 0; y < tiles; ++y){
				for (uint32 x = 0; x < tiles; ++x){
					uint16 vTopLeft = faceOffset + (y * (tiles+1) + x);
					uint16 vBottomLeft = faceOffset + ((y+1) * (tiles+1) + x);
					*iBuff++ = vTopLeft;
					*iBuff++ = vTopLeft+1;
					*iBuff++ = vBottomLeft;
					*iBuff++ = vTopLeft+1;
					*iBuff++ = vBottomLeft+1;
					*iBuff++ = vBottomLeft;
				}
			}
		}

		indexBuff->unlock();
		indexData.indexStart = 0;
		indexData.indexCount = indexCount;
		indexData.indexBuffer = indexBuff;
	}

	SpaceBackdrop::SpaceBackdropRenderable::~SpaceBackdropRenderable()
	{
		vertexData.vertexStart = 0;
		vertexData.vertexCount = 0;
		vertexData.vertexDeclaration->removeAllElements();
		vertexData.vertexBufferBinding->unsetAllBindings();

		indexData.indexStart = 0;
		indexData.indexCount = 0;
		indexData.indexBuffer.setNull();
	}

	void SpaceBackdrop::SpaceBackdropRenderable::getRenderOperation(Ogre::RenderOperation& op)
	{
		op.operationType = RenderOperation::OT_TRIANGLE_LIST;
		op.srcRenderable = this;
		op.useIndexes = true;
		op.vertexData = &vertexData;
		op.indexData = &indexData;
	}

	Ogre::Real SpaceBackdrop::SpaceBackdropRenderable::getSquaredViewDepth(const Ogre::Camera* cam) const
	{
		Vector3 camVec = cam->getDerivedPosition() - backdrop->getParentSceneNode()->_getDerivedPosition();
		return camVec.squaredLength();
	}

	const Ogre::LightList& SpaceBackdrop::SpaceBackdropRenderable::getLights(void) const
	{
		return backdrop->queryLights();
	}

}
