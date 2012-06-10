
#include "Stdafx.h"

#include "SimplePlanet.h"
#include "Utility.h"
#include "PlanetMath.h"


//using namespace Ogre;

namespace GalaxyEngine
{
	std::vector<SimplePlanetMesh*> SimplePlanet::loadedMeshes;

	SimplePlanet::SimplePlanet(float radius, Ogre::uint32 subdivision, const Ogre::String planetName)
		: withinFarDistance(false),
		minDistanceSquared(0),
		bestTechnique(NULL),
		sunLight(NULL)
	{
		Ogre::String prefix = "";
		if (planetName != "") prefix = planetName + "_";

		SimplePlanet::subdivision = subdivision;
		SimplePlanet::radius = radius;
		bounds.setMinimum(-radius, -radius, -radius);
		bounds.setMaximum(radius, radius, radius);

		material = Ogre::MaterialManager::getSingleton().create(Utility::getUniqueID(), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		Ogre::Pass *pass = material->getTechnique(0)->getPass(0);
		pass->createTextureUnitState(prefix + "norm_atlas.dds");
		pass->createTextureUnitState(prefix + "color_atlas.dds");
		pass->setVertexProgram("SimplePlanet_vs");
		pass->setFragmentProgram("SimplePlanet_ps");

		SimplePlanetMesh *mesh = loadSphereMesh(subdivision);
		renderable = new SimplePlanetRenderable(this, mesh);
	}

	SimplePlanet::~SimplePlanet()
	{
		delete renderable;
	}

	void SimplePlanet::_notifyCurrentCamera(Ogre::Camera *cam)
	{
		//Calculate camera distance
		Ogre::Vector3 camVec = cam->getDerivedPosition() - getParentSceneNode()->_getDerivedPosition();
		Ogre::Real centerDistanceSquared = camVec.squaredLength();

		if (getRenderingDistance() == 0) {
			withinFarDistance = true;
		} else {
			minDistanceSquared = std::max(0.0f, centerDistanceSquared - (radius * radius));
			//Note: centerDistanceSquared measures the distance between the camera and the center of the planet,
			//while minDistanceSquared measures the closest distance between the camera and the closest vertex of
			//the planet.

			//Determine whether the planet is within the far rendering distance
			withinFarDistance = minDistanceSquared <= Ogre::Math::Sqr(getRenderingDistance());
		}

		//Calculate the best material technique
		bestTechnique = material->getBestTechnique(material->getLodIndex(centerDistanceSquared));
		if (!bestTechnique){
			material->load();
			bestTechnique = material->getBestTechnique(material->getLodIndex(centerDistanceSquared));
		}

		//Update the planet's lighting calculations
		if (sunLight) 
		{
			updateLighting(sunLight, getParentSceneNode());
		}
	}

	void SimplePlanet::updateLighting(Ogre::Light *light, Ogre::SceneNode *planet)
	{
		Ogre::Vector3 lightDir = light->getDerivedPosition() - planet->_getDerivedPosition();
		lightDir.normalise();
		std::swap(lightDir.x, lightDir.z);
		lightDir = planet->_getDerivedOrientation() * lightDir;

		Ogre::Pass *pass = material->getTechnique(0)->getPass(0);
		Ogre::GpuProgramParametersSharedPtr params = pass->getFragmentProgramParameters();
		params->setNamedConstant("lightDirection", lightDir);
	}

	bool SimplePlanet::isVisible()
	{
		return mVisible && withinFarDistance;
	}

	void SimplePlanet::_updateRenderQueue(Ogre::RenderQueue *queue)
	{
		//Scale the parent node to achieve the correct planet radius
		getParentSceneNode()->setScale(radius, radius, radius);

		//Render
		queue->addRenderable(renderable);
	}

	SimplePlanetMesh *SimplePlanet::loadSphereMesh(Ogre::uint32 resolution)
	{
		//Search for an existing mesh that is identical to the requested one
		std::vector<SimplePlanetMesh*>::iterator it;
		for (it = loadedMeshes.begin(); it != loadedMeshes.end(); ++it) {
			SimplePlanetMesh *mesh = (*it);
			if (mesh->resolution == resolution)
				return mesh;
		}

		//If none exists, create a new one
		SimplePlanetMesh *mesh = new SimplePlanetMesh(resolution, 1.0f);
		loadedMeshes.push_back(mesh);
		return mesh;
	}

	void SimplePlanet::freeCachedMeshes()
	{
		std::vector<SimplePlanetMesh*>::iterator it;
		for (it = loadedMeshes.begin(); it != loadedMeshes.end(); ++it) {
			SimplePlanetMesh *mesh = (*it);
			delete mesh;
		}
	}


	SimplePlanet::SimplePlanetRenderable::SimplePlanetRenderable(SimplePlanet *planet, SimplePlanetMesh *mesh)
	{
		this->planet = planet;
		meshData = mesh;
	}

	void SimplePlanet::SimplePlanetRenderable::getRenderOperation(Ogre::RenderOperation& op)
	{
		op.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
		op.srcRenderable = this;
		op.useIndexes = true;
		op.vertexData = &(meshData->vertexData);
		op.indexData = &(meshData->indexData);
	}

	Ogre::Real SimplePlanet::SimplePlanetRenderable::getSquaredViewDepth(const Ogre::Camera* cam) const
	{
		Ogre::Vector3 camVec = cam->getDerivedPosition() - planet->getParentSceneNode()->_getDerivedPosition();
		return camVec.squaredLength();
	}

	const Ogre::LightList& SimplePlanet::SimplePlanetRenderable::getLights(void) const
	{
		return planet->queryLights();
	}


	SimplePlanetMesh::SimplePlanetMesh(Ogre::uint32 resolution, float radius)
	{
		this->resolution = resolution;
		this->radius = radius;
		float invRes = 1.0f / resolution;

		//Load vertex buffer
		Ogre::uint32 vertCount = 6 * (resolution+1) * (resolution+1);
		assert(vertCount < 0xFFFF);
		size_t offset = 0;
		vertexData.vertexDeclaration->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
		offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
		vertexData.vertexDeclaration->addElement(0, offset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES);
		Ogre::HardwareVertexBufferSharedPtr vertBuff = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
			vertexData.vertexDeclaration->getVertexSize(0), vertCount, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);

		float *vBuff = static_cast<float*>(vertBuff->lock(Ogre::HardwareBuffer::HBL_DISCARD));

		for (Ogre::uint32 face = 0; face < 6; ++face) {
			float faceNormalOffsetV = (face % 2) / 2.0f;
			float faceNormalOffsetU = (face / 2) / 4.0f;

			for (Ogre::uint32 y = 0; y <= resolution; ++y){
				for (Ogre::uint32 x = 0; x <= resolution; ++x){
					//Calculate UVs and terrain height
					Ogre::Real tx = x * invRes;
					Ogre::Real ty = y * invRes;

					//Calculate vertex position
					Ogre::Vector3 pos = PlanetMath::mapCubeToUnitSphere(mapPlaneToCube(tx, ty, (PlanetMath::CubeFace)face)) * radius;

					//Add vertex
					*vBuff++ = pos.x; *vBuff++ = pos.y; *vBuff++ = pos.z;
					*vBuff++ = faceNormalOffsetU + tx * 0.25f; *vBuff++ = faceNormalOffsetV + ty * 0.5f;
				}
			}
		}

		vertBuff->unlock();
		vertexData.vertexStart = 0;
		vertexData.vertexCount = vertCount;
		vertexData.vertexBufferBinding->setBinding(0, vertBuff);

		//Load index buffer
		Ogre::uint32 indexCount = 6 * 6 * resolution * resolution;
		Ogre::HardwareIndexBufferSharedPtr indexBuff = Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(
			Ogre::HardwareIndexBuffer::IT_16BIT, indexCount, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);

		Ogre::uint16 *iBuff = static_cast<Ogre::uint16*>(indexBuff->lock(Ogre::HardwareBuffer::HBL_DISCARD));

		//Index the chunk grid
		for (Ogre::uint32 face = 0; face < 6; ++face) {
			Ogre::uint32 faceOffset = face * ((resolution+1) * (resolution+1));
			for (Ogre::uint32 y = 0; y < resolution; ++y){
				for (Ogre::uint32 x = 0; x < resolution; ++x){
					Ogre::uint16 vTopLeft = faceOffset + (y * (resolution+1) + x);
					Ogre::uint16 vBottomLeft = faceOffset + ((y+1) * (resolution+1) + x);
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

}
