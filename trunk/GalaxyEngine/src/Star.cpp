
#include "Stdafx.h"

#include "Star.h"
#include "Utility.h"
#include "PlanetMath.h"
#include "Exception.h"

//using namespace Ogre;

namespace GalaxyEngine
{
	bool Star::noiseMapGenerated = false;

	Star::Star(Ogre::uint32 subdivision, const Ogre::String &starColorMap)
		: withinFarDistance(false),
		minDistanceSquared(0),
		bestTechnique(NULL)
	{
		Star::subdivision = subdivision;
		Star::radius = 1;
		bounds.setMinimum(-radius, -radius, -radius);
		bounds.setMaximum(radius, radius, radius);

		material = Ogre::MaterialManager::getSingleton().create(Utility::getUniqueID(), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		Ogre::Pass *pass = material->getTechnique(0)->getPass(0);
		pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
		pass->setDepthWriteEnabled(false);

		_generateNoiseMap();
		pass->createTextureUnitState("star_noise");

		pass->setVertexProgram("Star_vs");
		pass->setFragmentProgram("Star_ps");

		color = Ogre::ColourValue::White;
		this->animationPos = 0.0f;

		renderable = new StarRenderable(this);
	}

	Star::~Star()
	{
		delete renderable;
	}

	void Star::_generateNoiseMap()
	{
		const int cubeSize = 32; //32 x 32 x 32
		
		if (!noiseMapGenerated) {
			noiseMapGenerated = true;
		
			const size_t byteCount = cubeSize * cubeSize * cubeSize;
			char *bytes = new char[byteCount];
			char *ptr = bytes;
			for (int z = 0; z < cubeSize; ++z) {
				for (int y = 0; y < cubeSize; ++y) {
					for (int x = 0; x < cubeSize; ++x) {
						*ptr++ = rand() % 256;
					}
				}
			}

			Ogre::TexturePtr tex = Ogre::TextureManager::getSingleton().createManual("star_noise", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
				Ogre::TEX_TYPE_3D, cubeSize, cubeSize, cubeSize, 0, Ogre::PF_L8);
			char *texBytes = (char*)tex->getBuffer()->lock(0, byteCount, Ogre::HardwareBuffer::HBL_DISCARD);
			memcpy(texBytes, bytes, byteCount);
			tex->getBuffer()->unlock();

			delete[] bytes;
		}
	}

	void Star::_notifyCurrentCamera(Ogre::Camera *cam)
	{
		//Calculate camera distance
		Ogre::Vector3 camVec = cam->getDerivedPosition() - getParentSceneNode()->_getDerivedPosition();
		Ogre::Real centerDistanceSquared = camVec.squaredLength();

		if (getRenderingDistance() == 0) {
			withinFarDistance = true;
		} else {
			minDistanceSquared = std::max(0.0f, centerDistanceSquared - (radius * radius));
			//Note: centerDistanceSquared measures the distance between the camera and the center of the star,
			//while minDistanceSquared measures the closest distance between the camera and the closest vertex of
			//the star.

			//Determine whether the star is within the far rendering distance
			withinFarDistance = minDistanceSquared <= Ogre::Math::Sqr(getRenderingDistance());
		}

		//Calculate the best material technique
		//bestTechnique = material->getBestTechnique(material->getLodIndex(centerDistanceSquared));
		bestTechnique = material->getBestTechnique(material->getLodIndex(centerDistanceSquared));
		if (!bestTechnique){
			material->load();
			//bestTechnique = material->getBestTechnique(material->getLodIndex(centerDistanceSquared));
			bestTechnique = material->getBestTechnique(material->getLodIndex(centerDistanceSquared));
		}

		_updateShader(cam);
	}

	void Star::_updateShader(Ogre::Camera *cam)
	{
		float time = (float)timer.getMilliseconds() / 1000.0f;
		float time2 = time + 0.2f;

		time *= 1.0f;
		time2 *= 1.0f;

		Ogre::Pass *pass = material->getTechnique(0)->getPass(0);
		Ogre::GpuProgramParametersSharedPtr params = pass->getFragmentProgramParameters();
		params->setNamedConstant("sinTime1", 0.2f * (Ogre::Math::Sin(time) + Ogre::Math::Cos(time * 0.1f) * 8));
		params->setNamedConstant("cosTime1", 0.2f * (Ogre::Math::Cos(time) + Ogre::Math::Sin(time * 0.1f) * 8));
		params->setNamedConstant("sinTime2", 0.2f * (Ogre::Math::Sin(time2) + Ogre::Math::Cos(time2 * 0.1f) * 8));
		params->setNamedConstant("cosTime2", 0.2f * (Ogre::Math::Cos(time2) + Ogre::Math::Sin(time2 * 0.1f) * 8));

		Ogre::Vector3 camDir = cam->getDerivedPosition() - getParentSceneNode()->_getDerivedPosition();
		camDir.normalise();
		camDir = getParentSceneNode()->_getDerivedOrientation().Inverse() * camDir;
		params->setNamedConstant("camDirection", camDir);
	}

	bool Star::isVisible()
	{
		return mVisible && withinFarDistance;
	}

	void Star::_updateRenderQueue(Ogre::RenderQueue *queue)
	{
		queue->addRenderable(renderable);
	}

	Star::StarRenderable::StarRenderable(Star *star)
	{
		//Save misc. variables
		this->star = star;
		Ogre::uint32 tiles = star->getSubdivision();
		float invTiles = 1.0f / tiles;

		//Set material
		material = star->getMaterial();

		//Load vertex buffer
		Ogre::uint32 vertCount = 6 * (tiles+1) * (tiles+1);
		assert(vertCount < 0xFFFF);
		size_t offset = 0;
		vertexData.vertexDeclaration->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
		offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
		Ogre::HardwareVertexBufferSharedPtr vertBuff = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
			vertexData.vertexDeclaration->getVertexSize(0), vertCount, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);

		float *vBuff = static_cast<float*>(vertBuff->lock(Ogre::HardwareBuffer::HBL_DISCARD));

		for (Ogre::uint32 face = 0; face < 6; ++face) {
			float faceNormalOffsetU = (face % 3) / 4.0f;
			float faceNormalOffsetV = ((Ogre::uint32)face / (Ogre::uint32)3) / 2.0f;

			for (Ogre::uint32 y = 0; y <= tiles; ++y){
				for (Ogre::uint32 x = 0; x <= tiles; ++x){
					//Calculate UVs and terrain height
					Ogre::Real tx = x * invTiles;
					Ogre::Real ty = y * invTiles;

					//Calculate vertex position / normal
					Ogre::Vector3 pos = PlanetMath::mapCubeToUnitSphere(mapPlaneToCube(tx, ty, (PlanetMath::CubeFace)face));

					//Add vertex
					*vBuff++ = pos.x; *vBuff++ = pos.y; *vBuff++ = pos.z;
				}
			}
		}

		vertBuff->unlock();
		vertexData.vertexStart = 0;
		vertexData.vertexCount = vertCount;
		vertexData.vertexBufferBinding->setBinding(0, vertBuff);

		//Load index buffer
		Ogre::uint32 indexCount = 6 * 6 * tiles * tiles;
		Ogre::HardwareIndexBufferSharedPtr indexBuff = Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(
			Ogre::HardwareIndexBuffer::IT_16BIT, indexCount, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);

		Ogre::uint16 *iBuff = static_cast<Ogre::uint16*>(indexBuff->lock(Ogre::HardwareBuffer::HBL_DISCARD));

		//Index the chunk grid
		for (Ogre::uint32 face = 0; face < 6; ++face) {
			Ogre::uint32 faceOffset = face * ((tiles+1) * (tiles+1));
			for (Ogre::uint32 y = 0; y < tiles; ++y){
				for (Ogre::uint32 x = 0; x < tiles; ++x){
					Ogre::uint16 vTopLeft = faceOffset + (y * (tiles+1) + x);
					Ogre::uint16 vBottomLeft = faceOffset + ((y+1) * (tiles+1) + x);
					*iBuff++ = vBottomLeft;
					*iBuff++ = vTopLeft+1;
					*iBuff++ = vTopLeft;
					*iBuff++ = vBottomLeft;
					*iBuff++ = vBottomLeft+1;
					*iBuff++ = vTopLeft+1;
				}
			}
		}

		indexBuff->unlock();
		indexData.indexStart = 0;
		indexData.indexCount = indexCount;
		indexData.indexBuffer = indexBuff;
	}

	Star::StarRenderable::~StarRenderable()
	{
		vertexData.vertexStart = 0;
		vertexData.vertexCount = 0;
		vertexData.vertexDeclaration->removeAllElements();
		vertexData.vertexBufferBinding->unsetAllBindings();

		indexData.indexStart = 0;
		indexData.indexCount = 0;
		indexData.indexBuffer.setNull();
	}

	void Star::StarRenderable::getRenderOperation(Ogre::RenderOperation& op)
	{
		op.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
		op.srcRenderable = this;
		op.useIndexes = true;
		op.vertexData = &vertexData;
		op.indexData = &indexData;
	}

	Ogre::Real Star::StarRenderable::getSquaredViewDepth(const Ogre::Camera* cam) const
	{
		Ogre::Vector3 camVec = cam->getDerivedPosition() - star->getParentSceneNode()->_getDerivedPosition();
		return camVec.squaredLength();
	}

	const Ogre::LightList& Star::StarRenderable::getLights(void) const
	{
		return star->queryLights();
	}

}
