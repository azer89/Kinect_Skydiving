
#include "Stdafx.h"

#include "SimplePlanet.h"
#include "Utility.h"
#include "PlanetMath.h"


using namespace Ogre;

namespace GalaxyEngine
{
	std::vector<SimplePlanetMesh*> SimplePlanet::loadedMeshes;

	SimplePlanet::SimplePlanet(float radius, uint32 subdivision, const String planetName)
		: withinFarDistance(false),
		minDistanceSquared(0),
		bestTechnique(NULL),
		sunLight(NULL)
	{
		String prefix = "";
		if (planetName != "") prefix = planetName + "_";

		SimplePlanet::subdivision = subdivision;
		SimplePlanet::radius = radius;
		bounds.setMinimum(-radius, -radius, -radius);
		bounds.setMaximum(radius, radius, radius);

		material = MaterialManager::getSingleton().create(Utility::getUniqueID(), ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		Pass *pass = material->getTechnique(0)->getPass(0);
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

	void SimplePlanet::_notifyCurrentCamera(Camera *cam)
	{
		//Calculate camera distance
		Vector3 camVec = cam->getDerivedPosition() - getParentSceneNode()->_getDerivedPosition();
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

	void SimplePlanet::updateLighting(Light *light, SceneNode *planet)
	{
		Vector3 lightDir = light->getDerivedPosition() - planet->_getDerivedPosition();
		lightDir.normalise();
		std::swap(lightDir.x, lightDir.z);
		lightDir = planet->_getDerivedOrientation() * lightDir;

		Pass *pass = material->getTechnique(0)->getPass(0);
		GpuProgramParametersSharedPtr params = pass->getFragmentProgramParameters();
		params->setNamedConstant("lightDirection", lightDir);
	}

	bool SimplePlanet::isVisible()
	{
		return mVisible && withinFarDistance;
	}

	void SimplePlanet::_updateRenderQueue(RenderQueue *queue)
	{
		//Scale the parent node to achieve the correct planet radius
		getParentSceneNode()->setScale(radius, radius, radius);

		//Render
		queue->addRenderable(renderable);
	}

	SimplePlanetMesh *SimplePlanet::loadSphereMesh(uint32 resolution)
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

	void SimplePlanet::SimplePlanetRenderable::getRenderOperation(RenderOperation& op)
	{
		op.operationType = RenderOperation::OT_TRIANGLE_LIST;
		op.srcRenderable = this;
		op.useIndexes = true;
		op.vertexData = &(meshData->vertexData);
		op.indexData = &(meshData->indexData);
	}

	Real SimplePlanet::SimplePlanetRenderable::getSquaredViewDepth(const Camera* cam) const
	{
		Vector3 camVec = cam->getDerivedPosition() - planet->getParentSceneNode()->_getDerivedPosition();
		return camVec.squaredLength();
	}

	const LightList& SimplePlanet::SimplePlanetRenderable::getLights(void) const
	{
		return planet->queryLights();
	}


	SimplePlanetMesh::SimplePlanetMesh(uint32 resolution, float radius)
	{
		this->resolution = resolution;
		this->radius = radius;
		float invRes = 1.0f / resolution;

		//Load vertex buffer
		uint32 vertCount = 6 * (resolution+1) * (resolution+1);
		assert(vertCount < 0xFFFF);
		size_t offset = 0;
		vertexData.vertexDeclaration->addElement(0, offset, VET_FLOAT3, VES_POSITION);
		offset += VertexElement::getTypeSize(VET_FLOAT3);
		vertexData.vertexDeclaration->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES);
		HardwareVertexBufferSharedPtr vertBuff = HardwareBufferManager::getSingleton().createVertexBuffer(
			vertexData.vertexDeclaration->getVertexSize(0), vertCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);

		float *vBuff = static_cast<float*>(vertBuff->lock(HardwareBuffer::HBL_DISCARD));

		for (uint32 face = 0; face < 6; ++face) {
			float faceNormalOffsetV = (face % 2) / 2.0f;
			float faceNormalOffsetU = (face / 2) / 4.0f;

			for (uint32 y = 0; y <= resolution; ++y){
				for (uint32 x = 0; x <= resolution; ++x){
					//Calculate UVs and terrain height
					Real tx = x * invRes;
					Real ty = y * invRes;

					//Calculate vertex position
					Vector3 pos = PlanetMath::mapCubeToUnitSphere(mapPlaneToCube(tx, ty, (PlanetMath::CubeFace)face)) * radius;

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
		uint32 indexCount = 6 * 6 * resolution * resolution;
		HardwareIndexBufferSharedPtr indexBuff = HardwareBufferManager::getSingleton().createIndexBuffer(
			HardwareIndexBuffer::IT_16BIT, indexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);

		uint16 *iBuff = static_cast<uint16*>(indexBuff->lock(HardwareBuffer::HBL_DISCARD));

		//Index the chunk grid
		for (uint32 face = 0; face < 6; ++face) {
			uint32 faceOffset = face * ((resolution+1) * (resolution+1));
			for (uint32 y = 0; y < resolution; ++y){
				for (uint32 x = 0; x < resolution; ++x){
					uint16 vTopLeft = faceOffset + (y * (resolution+1) + x);
					uint16 vBottomLeft = faceOffset + ((y+1) * (resolution+1) + x);
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
