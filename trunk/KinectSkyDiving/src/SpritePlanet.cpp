
#include "Stdafx.h"

#include "SpritePlanet.h"
#include "Utility.h"
#include "Exception.h"

using namespace Ogre;

namespace GalaxyEngine
{
	SpritePlanet::SpritePlanet(float radius, const String planetName)
		: withinFarDistance(false),
		minDistanceSquared(0),
		bestTechnique(NULL),
		sunLight(NULL)
	{
		String prefix = "";
		if (planetName != "") prefix = planetName + "_";
		loadedPlanetName = planetName;

		SpritePlanet::radius = radius;
		bounds.setMinimum(-radius, -radius, -radius);
		bounds.setMaximum(radius, radius, radius);

		material = MaterialManager::getSingleton().create(Utility::getUniqueID(), ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		Pass *pass = material->getTechnique(0)->getPass(0);
		pass->createTextureUnitState(prefix + "sprite.dds");
		pass->setSceneBlending(SBT_TRANSPARENT_ALPHA);
		pass->setVertexProgram("SpritePlanet_vs");
		pass->setFragmentProgram("SpritePlanet_ps");

		renderable = new SpritePlanetRenderable(this);
	}

	SpritePlanet::~SpritePlanet()
	{
		delete renderable;
	}

	void SpritePlanet::_notifyCurrentCamera(Camera *cam)
	{
		SceneNode *node = getParentSceneNode();

		//Calculate camera distance
		Vector3 camVec = cam->getDerivedPosition() - node->_getDerivedPosition();
		Real centerDistanceSquared = camVec.squaredLength();

		if (getRenderingDistance() == 0) {
			withinFarDistance = true;
		} else {
			minDistanceSquared = std::max(0.0f, centerDistanceSquared - (radius * radius));
			//Note: centerDistanceSquared measures the distance between the camera and the center of the planet,
			//while minDistanceSquared measures the closest distance between the camera and the closest vertex of
			//the planet.

			//Determine whether the planet is within the far rendering distance
			withinFarDistance = minDistanceSquared <= Math::Sqr(getRenderingDistance());
		}

		//Calculate the best material technique
		//bestTechnique = material->getBestTechnique(material->getLodIndex(centerDistanceSquared));
		bestTechnique = material->getBestTechnique(material->getLodIndex(centerDistanceSquared));
		if (!bestTechnique){
			material->load();
			//bestTechnique = material->getBestTechnique(material->getLodIndex(centerDistanceSquared));
			bestTechnique = material->getBestTechnique(material->getLodIndex(centerDistanceSquared));
		}

		//Update the planet's lighting calculations
		if (sunLight) {
			Vector3 lightDir = sunLight->getDerivedPosition() - node->_getDerivedPosition();
			lightDir.normalise();
			lightDir = cam->getDerivedOrientation().Inverse() * lightDir;
			std::swap(lightDir.x, lightDir.z);

			Pass *pass = material->getTechnique(0)->getPass(0);
			GpuProgramParametersSharedPtr params = pass->getFragmentProgramParameters();
			params->setNamedConstant("lightDirection", lightDir);
		}

		//Update the sprite to face the camera
		matrix.makeTransform(node->_getDerivedPosition(), node->_getDerivedScale(), cam->getDerivedOrientation());
	}

	bool SpritePlanet::isVisible()
	{
		return mVisible && withinFarDistance;
	}

	void SpritePlanet::_updateRenderQueue(RenderQueue *queue)
	{
		//Requires unit scale
		getParentSceneNode()->setScale(Vector3::UNIT_SCALE);

		//Render
		queue->addRenderable(renderable);
	}

	void SpritePlanet::generateNormalMap()
	{
		String prefix = "";
		if (loadedPlanetName != "")
			prefix = loadedPlanetName + "_";
		String normalMapFile = prefix + "sprite.png";

		int halfWidth = 64;
		int height = 64;
		int width = halfWidth * 2;

		//First try loading an existing sprite image
		FreeImage_Initialise(false);
		FIBITMAP *normalMap = FreeImage_Load(FreeImage_GetFIFFromFilename(normalMapFile.c_str()), normalMapFile.c_str());
		if (!normalMap) {
			//If the image doesn't exist, create it
			normalMap = FreeImage_Allocate(width, height, 32, 0x0000FF, 0x00FF00, 0xFF0000);
		}
		BYTE *bits = FreeImage_GetBits(normalMap);

		for (int y = height-1; y >= 0; --y){
			for (int x = 0; x < halfWidth; ++x){
				Vector3 vertexNormal;

				vertexNormal.x = (2.0f * (float)x / halfWidth) - 1.0f;
				vertexNormal.y = (2.0f * (float)y / height) - 1.0f;
				float sq = 1.0f - vertexNormal.x*vertexNormal.x - vertexNormal.y*vertexNormal.y;
				if (sq > 0.0f) {
					vertexNormal.z = Math::Sqrt(sq);
				} else {
					vertexNormal.z = 0.0f;
					vertexNormal.normalise();
				}

				*bits++ = (vertexNormal.x + 1.0f) * 0.5f * 0xFF;
				*bits++ = (vertexNormal.y + 1.0f) * 0.5f * 0xFF;
				*bits++ = (vertexNormal.z + 1.0f) * 0.5f * 0xFF;
				*bits++ = 0xFF;
			}
			bits += halfWidth * 4;
		}

		BOOL success = FreeImage_Save(FreeImage_GetFIFFromFilename(normalMapFile.c_str()), normalMap, normalMapFile.c_str());
		FreeImage_Unload(normalMap);
		FreeImage_DeInitialise();
	}

	SpritePlanet::SpritePlanetRenderable::SpritePlanetRenderable(SpritePlanet *planet)
	{
		//Save misc. variables
		this->planet = planet;
		float radius = planet->getPlanetRadius();

		//Set material
		material = planet->getMaterial();

		//Load vertex buffer
		size_t offset = 0;
		vertexData.vertexDeclaration->addElement(0, offset, VET_FLOAT3, VES_POSITION);
		offset += VertexElement::getTypeSize(VET_FLOAT3);
		vertexData.vertexDeclaration->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES);
		HardwareVertexBufferSharedPtr vertBuff = HardwareBufferManager::getSingleton().createVertexBuffer(
			vertexData.vertexDeclaration->getVertexSize(0), 4, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);

		float *vBuff = static_cast<float*>(vertBuff->lock(HardwareBuffer::HBL_DISCARD));

		//Add vertex
		*vBuff++ = -radius; *vBuff++ = -radius; *vBuff++ = 0.0f;
		*vBuff++ = 0.0f; *vBuff++ = 0.0f;
		//Add vertex
		*vBuff++ = radius; *vBuff++ = -radius; *vBuff++ = 0.0f;
		*vBuff++ = 1.0f; *vBuff++ = 0.0f;
		//Add vertex
		*vBuff++ = -radius; *vBuff++ = radius; *vBuff++ = 0.0f;
		*vBuff++ = 0.0f; *vBuff++ = 1.0f;
		//Add vertex
		*vBuff++ = radius; *vBuff++ = radius; *vBuff++ = 0.0f;
		*vBuff++ = 1.0f; *vBuff++ = 1.0f;

		vertBuff->unlock();
		vertexData.vertexStart = 0;
		vertexData.vertexCount = 4;
		vertexData.vertexBufferBinding->setBinding(0, vertBuff);

		//Load index buffer
		HardwareIndexBufferSharedPtr indexBuff = HardwareBufferManager::getSingleton().createIndexBuffer(
			HardwareIndexBuffer::IT_16BIT, 6, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);

		uint16 *iBuff = static_cast<uint16*>(indexBuff->lock(HardwareBuffer::HBL_DISCARD));

		*iBuff++ = 0;
		*iBuff++ = 1;
		*iBuff++ = 2;
		*iBuff++ = 2;
		*iBuff++ = 1;
		*iBuff++ = 3;

		indexBuff->unlock();
		indexData.indexStart = 0;
		indexData.indexCount = 6;
		indexData.indexBuffer = indexBuff;
	}

	SpritePlanet::SpritePlanetRenderable::~SpritePlanetRenderable()
	{
		vertexData.vertexStart = 0;
		vertexData.vertexCount = 0;
		vertexData.vertexDeclaration->removeAllElements();
		vertexData.vertexBufferBinding->unsetAllBindings();

		indexData.indexStart = 0;
		indexData.indexCount = 0;
		indexData.indexBuffer.setNull();
	}

	void SpritePlanet::SpritePlanetRenderable::getRenderOperation(Ogre::RenderOperation& op)
	{
		op.operationType = RenderOperation::OT_TRIANGLE_LIST;
		op.srcRenderable = this;
		op.useIndexes = true;
		op.vertexData = &vertexData;
		op.indexData = &indexData;
	}

	Ogre::Real SpritePlanet::SpritePlanetRenderable::getSquaredViewDepth(const Ogre::Camera* cam) const
	{
		Vector3 camVec = cam->getDerivedPosition() - planet->getParentSceneNode()->_getDerivedPosition();
		return camVec.squaredLength();
	}

	const Ogre::LightList& SpritePlanet::SpritePlanetRenderable::getLights(void) const
	{
		return planet->queryLights();
	}

}
