
#include "Stdafx.h"

#include "PlanetLoader_Hybrid.h"
#include "Planet.h"
#include "Exception.h"

using namespace Ogre;

#include <list>
using namespace std;

namespace GalaxyEngine
{
	PlanetLoader_Hybrid::PlanetLoader_Hybrid(Real radius, Real maxTerrainHeight, uint32 chunkResolution)
	{
		//Vertex and pixel shaders are required
		const RenderSystemCapabilities *caps = Root::getSingleton().getRenderSystem()->getCapabilities();
		if (!caps->hasCapability(RSC_VERTEX_PROGRAM))
			EXCEPTION("Your video card does not support shaders. Vertex and pixel shaders are necessary to render planets.", "PlanetLoader_Hybrid");
		if (!caps->hasCapability(RSC_FRAGMENT_PROGRAM))
			EXCEPTION("Your video card does not support pixel shaders. Vertex and pixel shaders are necessary to render planets.", "PlanetLoader_Hybrid");
		loadedPlanetName = "NewPlanet";

		//Calculate the inner and outer radius. The outer radius is the maximum height any planet vertex can have
		innerRadius = radius;
		outerRadius = radius + maxTerrainHeight;

		//Setup materials for all 6 faces of the planet
		for (uint32 i = 0; i < 6; ++i){
			heightMap[i].deallocate();

			material[i] = MaterialManager::getSingleton().create("PlanetMat" + StringConverter::toString(i), "Planet");
			Pass *pass = material[i]->getTechnique(0)->getPass(0);
			pass->createTextureUnitState();
			pass->createTextureUnitState();
		}

		//Default maximum subdivision configuration
		maxLevels = 5;
		hmapLevels = 5;
		chunkRes = chunkResolution;

		//Default shading features
		highResNormals = false;
		ambientOcclusion = false;
		_generateShaders();

		//Reset normal map buffer
		normalBuffer = NULL;
		normalBufferSize = 0;

		//Pre-generate chunk indexes. All terrain chunks are almost identical (same number or vertexes, same
		//triangle configuration, etc.), with the exception of the positions of the vertexes. Because of this,
		//a single index buffer can be used for all terrain chunks.

		//Allocate hardware index buffer
		uint32 indexCount = (6 * chunkRes * chunkRes) + 6*(2*chunkRes + 2*chunkRes) + 12*3;
		HardwareIndexBufferSharedPtr indexBuff = HardwareBufferManager::getSingleton().createIndexBuffer(
			HardwareIndexBuffer::IT_16BIT, indexCount, HardwareBuffer::HBU_WRITE_ONLY, false);

		uint16 *iBuff = static_cast<uint16*>(indexBuff->lock(HardwareBuffer::HBL_DISCARD));

		//Index the chunk grid
		for (uint32 y = 0; y < chunkRes; ++y){
			for (uint32 x = 0; x < chunkRes; ++x){
				uint16 vTopLeft = y * (chunkRes+1) + x;
				uint16 vBottomLeft = (y+1) * (chunkRes+1) + x;
				*iBuff++ = vBottomLeft;
				*iBuff++ = vTopLeft+1;
				*iBuff++ = vTopLeft;
				*iBuff++ = vBottomLeft;
				*iBuff++ = vBottomLeft+1;
				*iBuff++ = vTopLeft+1;
			}
		}

		//Index the chunk skirt
		uint16 skirtStart = (chunkRes+1) * (chunkRes+1);

		{
			uint32 y = 0;
			for (uint32 x = 0; x < chunkRes; ++x){
				uint16 vTopLeft = y * (chunkRes+1) + x;
				uint16 vBottomLeft = skirtStart++;
				*iBuff++ = vTopLeft+1;
				*iBuff++ = vBottomLeft;
				*iBuff++ = vTopLeft;
				*iBuff++ = vBottomLeft+1;
				*iBuff++ = vBottomLeft;
				*iBuff++ = vTopLeft+1;
			}
			++skirtStart;

			y = chunkRes;
			for (uint32 x = 0; x < chunkRes; ++x){
				uint16 vTopLeft = y * (chunkRes+1) + x;
				uint16 vBottomLeft = skirtStart++;
				*iBuff++ = vBottomLeft;
				*iBuff++ = vTopLeft+1;
				*iBuff++ = vTopLeft;
				*iBuff++ = vBottomLeft;
				*iBuff++ = vBottomLeft+1;
				*iBuff++ = vTopLeft+1;
			}
			++skirtStart;
		}

		{
			uint32 x = 0;
			for (uint32 y = 0; y < chunkRes; ++y){
				uint16 vTopLeft = y * (chunkRes+1) + x;
				uint16 vTopRight = (y+1) * (chunkRes+1) + x;
				uint16 vBottomLeft = skirtStart++;
				*iBuff++ = vBottomLeft;
				*iBuff++ = vTopRight;
				*iBuff++ = vTopLeft;
				*iBuff++ = vBottomLeft;
				*iBuff++ = vBottomLeft+1;
				*iBuff++ = vTopRight;
			}
			++skirtStart;

			x = chunkRes;
			for (uint32 y = 0; y < chunkRes; ++y){
				uint16 vTopLeft = y * (chunkRes+1) + x;
				uint16 vTopRight = (y+1) * (chunkRes+1) + x;
				uint16 vBottomLeft = skirtStart++;
				*iBuff++ = vTopRight;
				*iBuff++ = vBottomLeft;
				*iBuff++ = vTopLeft;
				*iBuff++ = vBottomLeft+1;
				*iBuff++ = vBottomLeft;
				*iBuff++ = vTopRight;
			}
			++skirtStart;
		}

		indexBuff->unlock();
		sharedIndexData.indexStart = 0;
		sharedIndexData.indexCount = indexCount;
		sharedIndexData.indexBuffer = indexBuff;
	}

	PlanetLoader_Hybrid::~PlanetLoader_Hybrid()
	{
		if (normalBuffer)
			delete[] normalBuffer;

		for (uint32 i = 0; i < 6; ++i){
			heightMap[i].deallocate();
			material[i]->unload();
			
			const String matName = material[i]->getName();
			material[i].setNull();
			MaterialManager::getSingleton().remove(matName);
		}
	}

	void PlanetLoader_Hybrid::loadPlanet(const String &planetName)
	{
		//This function loads the planet's 6 normal maps and color map, and applies them.
		const String faceName[6] = { "front", "back", "right", "left", "top", "bottom" };
		
		String prefix = "";
		if (planetName != "")
			prefix = planetName + "_";
		loadedPlanetName = planetName;

		for (uint32 i = 0; i < 6; ++i) {
			setHeightMap((PlanetMath::CubeFace)i, prefix + "height_" + faceName[i] + ".png");
		}

		for (uint32 i = 0; i < 6; ++i) {
			setPropertyMap((PlanetMath::CubeFace)i, prefix + "prop_" + faceName[i] + ".jpg");
		}

		for (uint32 i = 0; i < 6; ++i) {
			//setNormalMap((PlanetMath::CubeFace)i, prefix + "norm_" + faceName[i] + ".dds");
			setNormalMap((PlanetMath::CubeFace)i, prefix + "norm_" + faceName[i] + ".png");
		}

		for (uint32 i = 0; i < 6; ++i) {
			setColorMap((PlanetMath::CubeFace)i, prefix + "color_" + faceName[i] + ".dds");
		}
	}

	void PlanetLoader_Hybrid::setHeightMap(const PlanetMath::CubeFace cubeFace, const String &fileName)
	{
		ByteMap &map = heightMap[cubeFace];
		map.deallocate();

		//Load heightmap image
		Image img;
		img.load(fileName, "Planet");
		if (img.getFormat() != PF_L8) {
			EXCEPTION("Wrong pixel format for heightmap image (must be 8-bit greyscale)", "PlanetLoader_Hybrid::setHeightMap()");
		}
		uint32 width = (uint32)img.getWidth();
		uint32 height = (uint32)img.getHeight();
		if (width != height) {
			EXCEPTION("Heightmap images must be square", "PlanetLoader_Hybrid::setHeightMap()");
		}

		//Calculate max. levels
		hmapLevels = (Math::Log(width / chunkRes) / Math::Log(2));

		//Copy into the ByteMap
		map.allocate(width, height);
		memcpy(map.data, img.getData(), map.getTotalBytes());
	}

	void PlanetLoader_Hybrid::setPropertyMap(const PlanetMath::CubeFace cubeFace, const String &fileName)
	{
		ByteMap &map = propertyMap[cubeFace];
		map.deallocate();

		//Load property map image
		Image img;
		img.load(fileName, "Planet");
		if (img.getFormat() != PF_R8G8B8) {
			EXCEPTION("Wrong pixel format for property map image (must be 24-bit RGB)", "PlanetLoader_Hybrid::setPropertyMap()");
		}
		uint32 width = (uint32)img.getWidth();
		uint32 height = (uint32)img.getHeight();
		if (width != height) {
			EXCEPTION("Property map images must be square", "PlanetLoader_Hybrid::setPropertyMap()");
		}

		//Copy into the ByteMap
		map.allocate(width, height, 3);
		memcpy(map.data, img.getData(), map.getTotalBytes());
	}

	void PlanetLoader_Hybrid::setNormalMap(const PlanetMath::CubeFace cubeFace, const String &fileName)
	{
		BackgroundProcessTicket t = ResourceBackgroundQueue::getSingleton().load(TextureManager::getSingleton().getResourceType(), fileName, "Planet");
		ticketList.push_back(t);

		TextureUnitState *tex = material[cubeFace]->getTechnique(0)->getPass(0)->getTextureUnitState(0);
		tex->setTextureName(fileName);
		tex->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);

		if (!material_highResNormals[cubeFace].isNull()) {
			TextureUnitState *tex = material_highResNormals[cubeFace]->getTechnique(0)->getPass(0)->getTextureUnitState(0);
			tex->setTextureName(fileName);
			tex->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);
		}
	}

	void PlanetLoader_Hybrid::setColorMap(const PlanetMath::CubeFace cubeFace, const String &fileName)
	{
		BackgroundProcessTicket t = ResourceBackgroundQueue::getSingleton().load(TextureManager::getSingleton().getResourceType(), fileName, "Planet");
		ticketList.push_back(t);

		TextureUnitState *tex = material[cubeFace]->getTechnique(0)->getPass(0)->getTextureUnitState(1);
		tex->setTextureName(fileName);
		tex->setTextureAddressingMode(TextureUnitState::TAM_WRAP);

		if (!material_highResNormals[cubeFace].isNull()) {
			TextureUnitState *tex = material_highResNormals[cubeFace]->getTechnique(0)->getPass(0)->getTextureUnitState(1);
			tex->setTextureName(fileName);
			tex->setTextureAddressingMode(TextureUnitState::TAM_WRAP);
		}
	}

	void PlanetLoader_Hybrid::setVirtualResolution(const uint32 mapSize)
	{
		//Calculate max. levels
		maxLevels = (Math::Log(mapSize / chunkRes) / Math::Log(2));		
	}

	bool PlanetLoader_Hybrid::isFinishedLoading()
	{
		if (ticketList.empty())
			//If no background processes are running, loading is complete
			return true;
		else {
			//If background processes were running, check if they still are
			std::list<BackgroundProcessTicket>::iterator i = ticketList.begin();
			while (i != ticketList.end()){
				BackgroundProcessTicket t = (*i);
				if (ResourceBackgroundQueue::getSingleton().isProcessComplete(t))
					i = ticketList.erase(i);
				else
					++i;
			}
			//If all background processes have completed, loading is now complete
			if (ticketList.empty())
				return true;
		}
		//Otherwise loading is not complete
		return false;
	}

	void PlanetLoader_Hybrid::_generateShaders()
	{
		//High res normals require a second set of materials/shaders
		if (highResNormals) {
			for (uint32 i = 0; i < 6; ++i){
				if (material_highResNormals[i].isNull())
					material_highResNormals[i] = material[i]->clone("PlanetMatN" + StringConverter::toString(i));
			}
		} else {
			for (uint32 i = 0; i < 6; ++i){
				if (!material_highResNormals[i].isNull())
					material_highResNormals[i].setNull();
			}
		}

		//Select shader types based on planet rendering options
		{
			//Get non high res normal shaders
			StringUtil::StrStreamType tmpName;
			tmpName << "Planet_";
			if (ambientOcclusion)
				tmpName << "ambientOcclusion_";
			const String vsName = tmpName.str() + "vs";
			const String psName = tmpName.str() + "ps";
			
			//Apply the shader
			for (uint32 i = 0; i < 6; ++i){
				MaterialPtr mat = material[i];

				Pass *pass = mat->getTechnique(0)->getPass(0);
				pass->setVertexProgram(vsName);
				pass->setFragmentProgram(psName);
			}
		}
		if (highResNormals) {
			//Get high res normal shaders
			StringUtil::StrStreamType tmpName;
			tmpName << "Planet_";
			if (highResNormals)
				tmpName << "highResNormals_";
			if (ambientOcclusion)
				tmpName << "ambientOcclusion_";
			const String vsName = tmpName.str() + "vs";
			const String psName = tmpName.str() + "ps";
			
			//Apply the shader
			for (uint32 i = 0; i < 6; ++i){
				MaterialPtr mat = material_highResNormals[i];

				Pass *pass = mat->getTechnique(0)->getPass(0);
				pass->setVertexProgram(vsName);
				pass->setFragmentProgram(psName);
			}
		}
	}

	void PlanetLoader_Hybrid::updateLighting(Light *light, SceneNode *planet)
	{
		Vector3 lightDir = light->getDerivedPosition() - planet->_getDerivedPosition();
		lightDir.normalise();
		std::swap(lightDir.x, lightDir.z);
		lightDir = planet->_getDerivedOrientation() * lightDir;

		for (uint32 i = 0; i < 6; ++i){
			Pass *pass;
			GpuProgramParametersSharedPtr params;

			pass = material[i]->getTechnique(0)->getPass(0);
			params = pass->getFragmentProgramParameters();
			params->setNamedConstant("lightDirection", lightDir);

			if (!material_highResNormals[i].isNull()) {
				pass = material_highResNormals[i]->getTechnique(0)->getPass(0);
				params = pass->getFragmentProgramParameters();
				params->setNamedConstant("lightDirection", lightDir);
				params = pass->getVertexProgramParameters();
				params->setNamedConstant("lightDirection", lightDir);
			}
		}		
	}

	void PlanetLoader_Hybrid::loadChunkMesh(ChunkMesh &mesh, Planet::ChunkNode *chunk, Planet *planet)
	{
		//----------------------------------- Apply Material ------------------------------------
		bool calculateNormals = highResNormals && (chunk->getChunkLevel() > hmapLevels);
		if (calculateNormals)
			mesh.material = material_highResNormals[chunk->getCubeFace()];
		else
			mesh.material = material[chunk->getCubeFace()];
		
		//------------------------------------ Mesh Indexes -------------------------------------
		//Use pregenerated index data
		mesh.indexData->indexStart = sharedIndexData.indexStart;
		mesh.indexData->indexCount = sharedIndexData.indexCount;
		mesh.indexData->indexBuffer = sharedIndexData.indexBuffer;

		//----------------------------------- LOD Calculations ----------------------------------
		//Calculate the approximate size (width or height) of a single tile
		float tileSize = (Math::PI * 0.25f * outerRadius) * (chunk->getDataBounds().width() / chunkRes);

		//Estimate the maximum vertex error (extremely crude estimate - to be replaced later)
		mesh.maxVertexError = tileSize * 0.5f;

		//Note: "skirtSize" is not used at unit scale - a skirtSize of 1 = (outerRadius - innerRadius) normal units
		float skirtSize = 0.5f * tileSize / (outerRadius - innerRadius);

		//------------------------- Load / Procedurally Generate Heightmap ----------------------
		assert(chunk->getUserData() == NULL);
		HybridChunkData *chunkDat = new HybridChunkData(this, chunk);	//The constructor loads/generates
		chunk->setUserData((void*)chunkDat);

		//Get bounds data
		float dataBoundsLeft = chunk->getDataBounds().left;
		float dataBoundsRight = chunk->getDataBounds().right;
		float dataBoundsTop = chunk->getDataBounds().top;
		float dataBoundsBottom = chunk->getDataBounds().bottom;
		float dataBoundsWidthDivRes = chunk->getDataBounds().width() / chunkRes;
		float dataBoundsHeightDivRes = chunk->getDataBounds().height() / chunkRes;
		PlanetMath::CubeFace cubeFace = chunk->getCubeFace();

		//----------------------------- Calculate vertex normals ----------------------------- 
		if (calculateNormals) {
			//Create a normal buffer array to temporarily store this chunk's normals as they are calculated
			//Also create 4 edge buffers which are used to help wrap edges of cube faces
			if (normalBuffer == NULL || normalBufferSize != chunkRes+1) {
				normalBufferSize = chunkRes+1;

				if (normalBuffer)
					delete[] normalBuffer;
				normalBuffer = new Vector3[normalBufferSize * normalBufferSize];
			}

			//---------- Main normal generation ----------
			for (uint32 y = 1; y < chunkRes; ++y){
				for (uint32 x = 1; x < chunkRes; ++x){
					float tx = dataBoundsLeft + x * dataBoundsWidthDivRes;
					float ty = dataBoundsTop + y * dataBoundsHeightDivRes;

					float hL, hR, hT, hB;
					hL = chunkDat->getTerrainHeight(x-1, y);
					hR = chunkDat->getTerrainHeight(x+1, y);
					hT = chunkDat->getTerrainHeight(x, y-1);
					hB = chunkDat->getTerrainHeight(x, y+1);

					Vector3 vL, vR, vT, vB;
					vL = planet->mapCubeToPlanet(PlanetMath::mapPlaneToCube(tx-dataBoundsWidthDivRes, ty, cubeFace), hL);
					vR = planet->mapCubeToPlanet(PlanetMath::mapPlaneToCube(tx+dataBoundsWidthDivRes, ty, cubeFace), hR);
					vT = planet->mapCubeToPlanet(PlanetMath::mapPlaneToCube(tx, ty-dataBoundsWidthDivRes, cubeFace), hT);
					vB = planet->mapCubeToPlanet(PlanetMath::mapPlaneToCube(tx, ty+dataBoundsWidthDivRes, cubeFace), hB);

					Vector3 vec1 = vR - vL;
					Vector3 vec2 = vT - vB;
					Vector3 normal = vec1.crossProduct(vec2);
					normal.normalise();
					normalBuffer[y * normalBufferSize + x] = normal;
				}
			}

			//---------- Edge normal generation (uses normals from heightmap) ----------
			//Get heightmap
			ByteMap *hMap = &heightMap[chunk->getCubeFace()];
			assert(hMap->width == hMap->height);	//Heightmap must be square

			//Get heightmap width/height
			uint32 hmapWidth0 = hMap->width-1, hmapHeight0 = hMap->height-1;	//1024x1024, for example
			uint32 hmapWidth1 = hMap->width, hmapHeight1 = hMap->height;		//1025x1025, for example

			//Calculate the section of the heightmap to use for this chunk
			TRect<uint32> chunkHmapBounds;	//Calculate chunk bounds in heightmap pixel-space coordinates
			chunkHmapBounds.left = hmapWidth0 * dataBoundsLeft;
			chunkHmapBounds.top = hmapHeight0 * dataBoundsTop;
			chunkHmapBounds.right = hmapWidth0 * dataBoundsRight;
			chunkHmapBounds.bottom = hmapHeight0 * dataBoundsBottom;
			if (chunkHmapBounds.left == chunkHmapBounds.right) ++chunkHmapBounds.right;
			if (chunkHmapBounds.top == chunkHmapBounds.bottom) ++chunkHmapBounds.top;
			float hmapPixelWidth = 1.0f / (float)hmapWidth0;
			float chunkHmapStep = (float)chunkHmapBounds.width() / (float)chunkRes;

			//Generate seamless normals
			if (chunkHmapBounds.top > 0 && chunkHmapBounds.bottom < hmapWidth0 && chunkHmapBounds.left > 0 && chunkHmapBounds.right < hmapWidth0) {
				//No wrapping
				uint32 x, y;
				y = 0;
				for (uint32 yi = chunkHmapBounds.top; yi <= chunkHmapBounds.bottom; yi += chunkHmapBounds.height()) {
					x = 0;
					uint32 oldindx = 0xFFFFFFFF;
					Vector3 normal;
					for (float xif = chunkHmapBounds.left; xif <= chunkHmapBounds.right; xif += chunkHmapStep) {
						uint32 xi = xif;

						if (xi != oldindx) {
							oldindx = xi;

							float hL, hR, hT, hB;
							hL = (float)hMap->getByteAt(xi-1, yi) / (float)0xFF;
							hR = (float)hMap->getByteAt(xi+1, yi) / (float)0xFF;
							hT = (float)hMap->getByteAt(xi, yi-1) / (float)0xFF;
							hB = (float)hMap->getByteAt(xi, yi+1) / (float)0xFF;

							float tx = xi * hmapPixelWidth;
							float ty = yi * hmapPixelWidth;

							Vector3 vL, vR, vT, vB;
							vL = planet->mapCubeToPlanet(PlanetMath::mapPlaneToCube(tx-hmapPixelWidth, ty, cubeFace), hL);
							vR = planet->mapCubeToPlanet(PlanetMath::mapPlaneToCube(tx+hmapPixelWidth, ty, cubeFace), hR);
							vT = planet->mapCubeToPlanet(PlanetMath::mapPlaneToCube(tx, ty-hmapPixelWidth, cubeFace), hT);
							vB = planet->mapCubeToPlanet(PlanetMath::mapPlaneToCube(tx, ty+hmapPixelWidth, cubeFace), hB);

							Vector3 vec1 = vR - vL;
							Vector3 vec2 = vT - vB;

							normal = vec1.crossProduct(vec2);
							normal.normalise();
						}

						normalBuffer[y * normalBufferSize + x] = normal;
						++x;
					}
					y += chunkRes;
				}

				x = 0;
				for (uint32 xi = chunkHmapBounds.left; xi <= chunkHmapBounds.right; xi += chunkHmapBounds.width()) {
					y = 0;
					uint32 oldindx = 0xFFFFFFFF;
					Vector3 normal;
					for (float yif = chunkHmapBounds.top; yif <= chunkHmapBounds.bottom; yif += chunkHmapStep) {
						uint32 yi = yif;

						if (yi != oldindx) {
							oldindx = yi;

							float hL, hR, hT, hB;
							hL = (float)hMap->getByteAt(xi-1, yi) / (float)0xFF;
							hR = (float)hMap->getByteAt(xi+1, yi) / (float)0xFF;
							hT = (float)hMap->getByteAt(xi, yi-1) / (float)0xFF;
							hB = (float)hMap->getByteAt(xi, yi+1) / (float)0xFF;

							float tx = xi * hmapPixelWidth;
							float ty = yi * hmapPixelWidth;

							Vector3 vL, vR, vT, vB;
							vL = planet->mapCubeToPlanet(PlanetMath::mapPlaneToCube(tx-hmapPixelWidth, ty, cubeFace), hL);
							vR = planet->mapCubeToPlanet(PlanetMath::mapPlaneToCube(tx+hmapPixelWidth, ty, cubeFace), hR);
							vT = planet->mapCubeToPlanet(PlanetMath::mapPlaneToCube(tx, ty-hmapPixelWidth, cubeFace), hT);
							vB = planet->mapCubeToPlanet(PlanetMath::mapPlaneToCube(tx, ty+hmapPixelWidth, cubeFace), hB);

							Vector3 vec1 = vR - vL;
							Vector3 vec2 = vT - vB;

							normal = vec1.crossProduct(vec2);
							normal.normalise();
						}

						normalBuffer[y * normalBufferSize + x] = normal;
						++y;
					}
					x += chunkRes;
				}
			} else {
				//Wrapping
				uint32 x, y;
				y = 0;
				for (uint32 yi = chunkHmapBounds.top; yi <= chunkHmapBounds.bottom; yi += chunkHmapBounds.height()) {
					x = 0;
					uint32 oldindx = 0xFFFFFFFF;
					Vector3 normal;
					for (float xif = chunkHmapBounds.left; xif <= chunkHmapBounds.right; xif += chunkHmapStep) {
						uint32 xi = xif;

						if (yi != oldindx) {
							oldindx = yi;

							Vector3 vL, vR, vT, vB;
							int tmpX, tmpY;
							PlanetMath::CubeFace tmpFace;
							float tmpH, tx, ty;

							tmpX = xi-1; tmpY = yi;
							tmpFace = wrapCubeFaceIndexes(tmpX, tmpY, hmapWidth0, cubeFace);
							tmpH = (float)heightMap[tmpFace].getByteAt(tmpX, tmpY) / (float)0xFF;
							tx = tmpX * hmapPixelWidth; ty = tmpY * hmapPixelWidth;
							vL = planet->mapCubeToPlanet(PlanetMath::mapPlaneToCube(tx, ty, tmpFace), tmpH);

							tmpX = xi+1; tmpY = yi;
							tmpFace = wrapCubeFaceIndexes(tmpX, tmpY, hmapWidth0, cubeFace);
							tmpH = (float)heightMap[tmpFace].getByteAt(tmpX, tmpY) / (float)0xFF;
							tx = tmpX * hmapPixelWidth; ty = tmpY * hmapPixelWidth;
							vR = planet->mapCubeToPlanet(PlanetMath::mapPlaneToCube(tx, ty, tmpFace), tmpH);

							tmpX = xi; tmpY = yi-1;
							tmpFace = wrapCubeFaceIndexes(tmpX, tmpY, hmapWidth0, cubeFace);
							tmpH = (float)heightMap[tmpFace].getByteAt(tmpX, tmpY) / (float)0xFF;
							tx = tmpX * hmapPixelWidth; ty = tmpY * hmapPixelWidth;
							vT = planet->mapCubeToPlanet(PlanetMath::mapPlaneToCube(tx, ty, tmpFace), tmpH);

							tmpX = xi; tmpY = yi+1;
							tmpFace = wrapCubeFaceIndexes(tmpX, tmpY, hmapWidth0, cubeFace);
							tmpH = (float)heightMap[tmpFace].getByteAt(tmpX, tmpY) / (float)0xFF;
							tx = tmpX * hmapPixelWidth; ty = tmpY * hmapPixelWidth;
							vB = planet->mapCubeToPlanet(PlanetMath::mapPlaneToCube(tx, ty, tmpFace), tmpH);

							Vector3 vec1 = vR - vL;
							Vector3 vec2 = vT - vB;

							normal = vec1.crossProduct(vec2);
							normal.normalise();
						}

						normalBuffer[y * normalBufferSize + x] = normal;
						++x;
					}
					y += chunkRes;
				}

				x = 0;
				for (uint32 xi = chunkHmapBounds.left; xi <= chunkHmapBounds.right; xi += chunkHmapBounds.width()) {
					y = 0;
					uint32 oldindx = 0xFFFFFFFF;
					Vector3 normal;
					for (float yif = chunkHmapBounds.top; yif <= chunkHmapBounds.bottom; yif += chunkHmapStep) {
						uint32 yi = yif;

						if (yi != oldindx) {
							oldindx = yi;

							Vector3 vL, vR, vT, vB;
							int tmpX, tmpY;
							PlanetMath::CubeFace tmpFace;
							float tmpH, tx, ty;

							tmpX = xi-1; tmpY = yi;
							tmpFace = wrapCubeFaceIndexes(tmpX, tmpY, hmapWidth0, cubeFace);
							tmpH = (float)heightMap[tmpFace].getByteAt(tmpX, tmpY) / (float)0xFF;
							tx = tmpX * hmapPixelWidth; ty = tmpY * hmapPixelWidth;
							vL = planet->mapCubeToPlanet(PlanetMath::mapPlaneToCube(tx, ty, tmpFace), tmpH);

							tmpX = xi+1; tmpY = yi;
							tmpFace = wrapCubeFaceIndexes(tmpX, tmpY, hmapWidth0, cubeFace);
							tmpH = (float)heightMap[tmpFace].getByteAt(tmpX, tmpY) / (float)0xFF;
							tx = tmpX * hmapPixelWidth; ty = tmpY * hmapPixelWidth;
							vR = planet->mapCubeToPlanet(PlanetMath::mapPlaneToCube(tx, ty, tmpFace), tmpH);

							tmpX = xi; tmpY = yi-1;
							tmpFace = wrapCubeFaceIndexes(tmpX, tmpY, hmapWidth0, cubeFace);
							tmpH = (float)heightMap[tmpFace].getByteAt(tmpX, tmpY) / (float)0xFF;
							tx = tmpX * hmapPixelWidth; ty = tmpY * hmapPixelWidth;
							vT = planet->mapCubeToPlanet(PlanetMath::mapPlaneToCube(tx, ty, tmpFace), tmpH);

							tmpX = xi; tmpY = yi+1;
							tmpFace = wrapCubeFaceIndexes(tmpX, tmpY, hmapWidth0, cubeFace);
							tmpH = (float)heightMap[tmpFace].getByteAt(tmpX, tmpY) / (float)0xFF;
							tx = tmpX * hmapPixelWidth; ty = tmpY * hmapPixelWidth;
							vB = planet->mapCubeToPlanet(PlanetMath::mapPlaneToCube(tx, ty, tmpFace), tmpH);

							Vector3 vec1 = vR - vL;
							Vector3 vec2 = vT - vB;

							normal = vec1.crossProduct(vec2);
							normal.normalise();
						}

						normalBuffer[y * normalBufferSize + x] = normal;
						++y;
					}
					x += chunkRes;
				}
			}
		}

		//----------------------------- Allocate & Lock Vertex Buffer ---------------------------
		//Allocate hardware vertex buffer
		VertexData &vertexData = *mesh.vertexData;
		uint32 vertCount = (chunkRes+1) * (chunkRes+1) + (2*(chunkRes+1) + 2*(chunkRes+1));
		assert(vertCount < 0xFFFF);
		size_t offset = 0;

		vertexData.vertexDeclaration->addElement(0, offset, VET_FLOAT3, VES_POSITION);
		offset += VertexElement::getTypeSize(VET_FLOAT3);

		if (calculateNormals) {
			vertexData.vertexDeclaration->addElement(0, offset, VET_FLOAT3, VES_NORMAL);
			offset += VertexElement::getTypeSize(VET_FLOAT3);
		}

		vertexData.vertexDeclaration->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES);

		HardwareVertexBufferSharedPtr vertBuff = HardwareBufferManager::getSingleton().createVertexBuffer(
			vertexData.vertexDeclaration->getVertexSize(0), vertCount, HardwareBuffer::HBU_WRITE_ONLY, false);

		float *vBuff = static_cast<float*>(vertBuff->lock(HardwareBuffer::HBL_DISCARD));

		//---------------------------------- Vertex Generation ----------------------------------
		//Initialize bounds info
		float minX = Math::POS_INFINITY, minY = Math::POS_INFINITY, minZ = Math::POS_INFINITY;
		float maxX = Math::NEG_INFINITY, maxY = Math::NEG_INFINITY, maxZ = Math::NEG_INFINITY;
		float minElevation = Math::POS_INFINITY, maxElevation = Math::NEG_INFINITY;

		//Create chunk grid
		for (uint32 y = 0; y <= chunkRes; ++y) {
			for (uint32 x = 0; x <= chunkRes; ++x) {
				//Calculate UVs and terrain height
				float tx = dataBoundsLeft + x * dataBoundsWidthDivRes;
				float ty = dataBoundsTop + y * dataBoundsHeightDivRes;
				float elevation = chunkDat->getTerrainHeight(x, y);

				//Calculate vertex position
				Vector3 pos = planet->mapCubeToPlanet(PlanetMath::mapPlaneToCube(tx, ty, cubeFace), elevation);

				//Add vertex
				*vBuff++ = pos.x; *vBuff++ = pos.y; *vBuff++ = pos.z;
				if (calculateNormals) {
					const Vector3 &normal = normalBuffer[y * normalBufferSize + x];
					*vBuff++ = normal.z; *vBuff++ = normal.y; *vBuff++ = normal.x;
				}
				*vBuff++ = tx; *vBuff++ = ty;

				//Update bounds info
				if (pos.x < minX) minX = pos.x; if (pos.x > maxX) maxX = pos.x;
				if (pos.y < minY) minY = pos.y; if (pos.y > maxY) maxY = pos.y;
				if (pos.z < minZ) minZ = pos.z; if (pos.z > maxZ) maxZ = pos.z;
				if (elevation < minElevation) minElevation = elevation; if (elevation > maxElevation) maxElevation = elevation;
			}
		}

		//Create chunk skirt
		for (uint32 y = 0; y <= chunkRes; y += chunkRes) {
			for (uint32 x = 0; x <= chunkRes; ++x) {
				//Calculate UVs and terrain height
				float tx = dataBoundsLeft + x * dataBoundsWidthDivRes;
				float ty = dataBoundsTop + y * dataBoundsHeightDivRes;
				float elevation = chunkDat->getTerrainHeight(x, y);

				//Calculate vertex position
				Vector3 pos = planet->mapCubeToPlanet(PlanetMath::mapPlaneToCube(tx, ty, cubeFace), elevation - skirtSize);

				//Add vertex
				*vBuff++ = pos.x; *vBuff++ = pos.y; *vBuff++ = pos.z;
				if (calculateNormals) {
					const Vector3 &normal = normalBuffer[y * normalBufferSize + x];
					*vBuff++ = normal.x; *vBuff++ = normal.y; *vBuff++ = normal.z;
				}
				*vBuff++ = tx; *vBuff++ = ty;	
			}
		}


		for (uint32 x = 0; x <= chunkRes; x += chunkRes) {
			for (uint32 y = 0; y <= chunkRes; ++y) {
				//Calculate UVs and terrain height
				float tx = dataBoundsLeft + x * dataBoundsWidthDivRes;
				float ty = dataBoundsTop + y * dataBoundsHeightDivRes;
				float elevation = chunkDat->getTerrainHeight(x, y);

				//Calculate vertex position
				Vector3 pos = planet->mapCubeToPlanet(PlanetMath::mapPlaneToCube(tx, ty, cubeFace), elevation - skirtSize);

				//Add vertex
				*vBuff++ = pos.x; *vBuff++ = pos.y; *vBuff++ = pos.z;
				if (calculateNormals) {
					const Vector3 &normal = normalBuffer[y * normalBufferSize + x];
					*vBuff++ = normal.x; *vBuff++ = normal.y; *vBuff++ = normal.z;
				}
				*vBuff++ = tx; *vBuff++ = ty;	
			}
		}

		//Save bounding box
		mesh.boundingBox = AxisAlignedBox(minX, minY, minZ, maxX, maxY, maxZ);

		//Calculate center point & radius
		float tx, ty, tempR;
		tx = chunk->getDataBounds().left + (chunk->getDataBounds().width() * 0.5f);
		ty = chunk->getDataBounds().top + (chunk->getDataBounds().height() * 0.5f);
		float midElevation = (minElevation + maxElevation) * 0.5f;
		mesh.center = planet->mapCubeToPlanet(PlanetMath::mapPlaneToCube(tx, ty, cubeFace), midElevation);

		mesh.radius = 0;
		tempR = mesh.boundingBox.getMaximum().distance(mesh.center);
		if (tempR > mesh.radius) mesh.radius = tempR;

		tempR = mesh.boundingBox.getMinimum().distance(mesh.center);
		if (tempR > mesh.radius) mesh.radius = tempR;

		//Unlock vertex buffer & finalize
		vertBuff->unlock();
		vertexData.vertexStart = 0;
		vertexData.vertexCount = vertCount;
		vertexData.vertexBufferBinding->setBinding(0, vertBuff);

		//---------------------------------- Vertex Reading ----------------------------------
		chunk->vertex_size = vertexData.vertexCount;
		chunk->vertices = new Ogre::Vector3[chunk->vertex_size];
		const Ogre::VertexElement* posElem = vertexData.vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);
		Ogre::HardwareVertexBufferSharedPtr vbuf = vertexData.vertexBufferBinding->getBuffer(posElem->getSource());
		unsigned char* vertex = static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));

		float* pReal;

		for( size_t j = 0; j < vertexData.vertexCount; ++j, vertex += vbuf->getVertexSize())
		{
			posElem->baseVertexPointerToElement(vertex, &pReal);

			Ogre::Vector3 pt(pReal[0], pReal[1], pReal[2]);
			//chunk->vertices.push_back(pt);
			chunk->vertices[j] = pt;
		}

		vbuf->unlock();

		//-------------------------------- Index Reading --------------------------------------
		chunk->index_size = mesh.indexData->indexCount;
		chunk->indices = new unsigned long[chunk->index_size];
		size_t numTris = mesh.indexData->indexCount / 3;
		Ogre::HardwareIndexBufferSharedPtr ibuf = mesh.indexData->indexBuffer;
		if( !ibuf.isNull() )
		{
			bool use32bitindexes = (ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);

			unsigned long*  pLong = static_cast<unsigned long*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
			unsigned short* pShort = reinterpret_cast<unsigned short*>(pLong);

			size_t index_start = mesh.indexData->indexStart;
			size_t last_index = numTris*3 + index_start;

			if (use32bitindexes)
			{
				int offset = 0;
				for (size_t k = index_start; k < last_index; ++k)
				{
					//chunk->indices.push_back(pLong[k]);
					chunk->indices[offset++] = pLong[k];
				}
			}
			else
			{
				int offset = 0;
				for (size_t k = index_start; k < last_index; ++k)
				{
					//chunk->indices.push_back(static_cast<unsigned long>(pShort[k]));
					chunk->indices[offset++] = static_cast<unsigned long>(pShort[k]);
				}
			}

				ibuf->unlock();
		}
	}

	void PlanetLoader_Hybrid::unloadChunkMesh(Planet::ChunkNode *chunk, Planet *planet)
	{
		HybridChunkData *chunkDat = (HybridChunkData*)chunk->getUserData();
		if (chunkDat != NULL)
			delete chunkDat;
	}

	void PlanetLoader_Hybrid::generateNormalMaps()
	{
		const String faceName[6] = { "front", "back", "right", "left", "top", "bottom" };
		
		String prefix = "";
		if (loadedPlanetName != "")
			prefix = loadedPlanetName + "_";

		//Calculate normals for each cube face
		for (uint32 cubeFace = 0; cubeFace < 6; ++cubeFace) {
			String normalMapFile = prefix + "norm_" + faceName[cubeFace] + ".png";

			const ByteMap &hmap = heightMap[cubeFace];
			uint32 hmapWidth = heightMap[cubeFace].width;
			uint32 hmapHeight = heightMap[cubeFace].height;
			
			int width = hmapWidth - 1;
			int height = hmapHeight - 1;

			FreeImage_Initialise(false);
			FIBITMAP *normalMap = FreeImage_Allocate(width, height, 24, 0x0000FF, 0x00FF00, 0xFF0000);
			BYTE *bits = FreeImage_GetBits(normalMap);

			for (int y = height-1; y >= 0; --y){
				for (int x = 0; x < width; ++x){
					Vector3 vertexNormal;
					Vector3 vpos[3][3];

					int x2, y2;
					if (x < width-1 && y < height-1) {
						x2 = x;
						y2 = y;
					} else {
						x2 = x + 1;
						y2 = y + 1;
					}

					//Calculate surrounding vertex positions, wrapping seams
					for (int yy = -1; yy <= 1; ++yy){
						for (int xx = -1; xx <= 1; ++xx){
							if ((xx == -1 && yy == -1) || (xx == 1 && yy == 1))
								continue;

							int px = x2 + xx;
							int py = y2 + yy;

							PlanetMath::CubeFace tmpFace = PlanetMath::wrapCubeFaceIndexes(px, py, width, (PlanetMath::CubeFace)cubeFace);
							float elevation = (float)heightMap[tmpFace].getByteAt(px, py) / (float)0xFF;

							//Get vertex position
							float tx = (float)px / hmapWidth;
							float ty = (float)py / hmapHeight;

							float heightScale = innerRadius + (elevation * (outerRadius - innerRadius));
							vpos[xx+1][yy+1] = PlanetMath::mapCubeToUnitSphere(PlanetMath::mapPlaneToCube(tx, ty, tmpFace)) * heightScale;
						}
					}

					//Calculate the face normals
					Vector3 n1 = (vpos[0][2] - vpos[1][1]).crossProduct((vpos[0][1] - vpos[1][1]));
					Vector3 n2 = (vpos[0][1] - vpos[1][1]).crossProduct((vpos[1][0] - vpos[1][1]));
					Vector3 n3 = (vpos[1][0] - vpos[1][1]).crossProduct((vpos[2][0] - vpos[1][1]));
					Vector3 n4 = (vpos[2][0] - vpos[1][1]).crossProduct((vpos[2][1] - vpos[1][1]));
					Vector3 n5 = (vpos[2][1] - vpos[1][1]).crossProduct((vpos[1][2] - vpos[1][1]));
					Vector3 n6 = (vpos[1][2] - vpos[1][1]).crossProduct((vpos[0][2] - vpos[1][1]));

					//Average the normals
					vertexNormal = (n1+n2+n3+n4+n5+n6);
					vertexNormal.normalise();
					vertexNormal = -vertexNormal;

					*bits++ = (vertexNormal.x + 1.0f) * 0.5f * 0xFF;
					*bits++ = (vertexNormal.y + 1.0f) * 0.5f * 0xFF;
					*bits++ = (vertexNormal.z + 1.0f) * 0.5f * 0xFF;
				}
			}
			
			BOOL success = FreeImage_Save(FreeImage_GetFIFFromFilename(normalMapFile.c_str()), normalMap, normalMapFile.c_str());
			FreeImage_Unload(normalMap);
			FreeImage_DeInitialise();

			if (!success)
				EXCEPTION("Failed to save computed planet normal map (FreeImage error).", "PlanetLoader_Hybrid::generateNormalMaps()");
		}
	}


	PlanetLoader_Hybrid::HybridChunkData::HybridChunkData(PlanetLoader_Hybrid *loader, Planet::ChunkNode *chunkNode)
	{
		//Allocate vertex height array for this chunk
		vertexArrayWidth = loader->chunkRes + 1;
		vertexHeights = new float[vertexArrayWidth * vertexArrayWidth];	//square

		//Fill the height array with heightmap data if no procedural detail is needed at this level
		float div255 = 1.0f / (float)0xFF;
		if (chunkNode->getChunkLevel() <= loader->hmapLevels) {
			//Get heightmap
			const ByteMap &heightMap = loader->heightMap[chunkNode->getCubeFace()];
			assert(heightMap.width == heightMap.height);	//Heightmap must be square
			
			//Get heightmap width/height
			uint32 hmapWidth0 = heightMap.width-1, hmapHeight0 = heightMap.height-1;	//1024x1024, for example
			uint32 hmapWidth1 = heightMap.width, hmapHeight1 = heightMap.height;		//1025x1025, for example

			//Calculate the section of the heightmap to use for this chunk
			FloatRect unitBounds = chunkNode->getDataBounds();	//Chunk bounds within a (0,0) - (1,1) square
			TRect<uint32> hmapBounds;	//Calculate chunk bounds in heightmap pixel-space coordinates
			hmapBounds.left = hmapWidth0 * unitBounds.left;
			hmapBounds.top = hmapHeight0 * unitBounds.top;
			hmapBounds.right = hmapWidth0 * unitBounds.right;
			hmapBounds.bottom = hmapHeight0 * unitBounds.bottom;

			//Calculate height pixel sample step size
			uint32 stepSize = hmapBounds.width() / loader->chunkRes;
			assert((hmapBounds.width() / stepSize) == loader->chunkRes);

			//Load the height pixels into the vertex height array
			float *buffPtr = vertexHeights;
			for (uint32 y = hmapBounds.top; y <= hmapBounds.bottom; y += stepSize) {
				for (uint32 x = hmapBounds.left; x <= hmapBounds.right; x += stepSize) {
					//Load the heightmap pixel into the vertex height array, converting from 8-bit height value to a 0-1 float
					*buffPtr = (float)heightMap.getByteAt(x, y) * div255;
					++buffPtr;
				}
			}
		}

		//Otherwise fill the height array with procedural data based on the previous level if procedural data is needed at this level
		else {
			//Get the parent chunk's chunk data
			Planet::ChunkNode *parentChunk = chunkNode->getParent(); assert(parentChunk != NULL);
			HybridChunkData *parentChunkDat = (HybridChunkData*)parentChunk->getUserData();
			assert(parentChunkDat->vertexArrayWidth == vertexArrayWidth);

			//This sub-chunk will be based on a sub-area of that data. Calculate the section of it's height array to use.
			FloatRect parentBounds = parentChunk->getDataBounds();
			FloatRect thisBounds = chunkNode->getDataBounds();
			FloatRect relativeBounds;
			relativeBounds.left = (thisBounds.left - parentBounds.left) / parentBounds.width();
			relativeBounds.top = (thisBounds.top - parentBounds.top) / parentBounds.height();
			relativeBounds.right = (thisBounds.right - parentBounds.left) / parentBounds.width();
			relativeBounds.bottom = (thisBounds.bottom - parentBounds.top) / parentBounds.height();

			//Convert relativeBounds to pixel-space
			TRect<uint32> hmapBounds;
			hmapBounds.left = loader->chunkRes * relativeBounds.left;
			hmapBounds.top = loader->chunkRes * relativeBounds.top;
			hmapBounds.right = loader->chunkRes * relativeBounds.right;
			hmapBounds.bottom = loader->chunkRes * relativeBounds.bottom;

			//Get property map
			ByteMap *propertyMap = &loader->propertyMap[chunkNode->getCubeFace()];
			assert(propertyMap->width == propertyMap->height);	//Property map must be square

			//Calculate the section of the property map that applies to this chunk
			TRect<uint32> pmapBounds;
			pmapBounds.left = propertyMap->width * thisBounds.left;
			pmapBounds.top = propertyMap->height * thisBounds.top;
			pmapBounds.right = propertyMap->width * thisBounds.right;
			pmapBounds.bottom = propertyMap->height * thisBounds.bottom;

			//Calculate map scaling factor. This should always be 2 since this chunk node's resolution should always be
			//exactly twice as high (or 4x the area) as the parent's (due to the quadtree based planet chunk system).
			uint32 mapScale = loader->chunkRes / hmapBounds.width();
			assert(mapScale == 2);

			//Calculate the size of a single non-procedural tile
			float tileSize = (Math::PI * 0.25 * loader->outerRadius) / loader->heightMap[chunkNode->getCubeFace()].width;

			//-------------------------- Procedural Terrain Generation --------------------------
			//This code fills the chunk's height array with heights based on a subset of the parent's
			//heights, with resolution enhanced using a midpoint displacement fractal. It's an inherently
			//recursive algorithm, and fits well into the planet renderer because the quadtree LOD system will
			//automatically cause this function to generate exactly the level of procedural detail needed (if any).
			//The code below performs a single iteration of midpoint displacement, since each sub chunk is exactly
			//twice the resolution of it's parent.

			//Seed random number generator based on chunk location
			uint32 seed = (thisBounds.top * 2000) * 2000 + (thisBounds.left * 2000);
			srand(seed);

			//Prepare property map access
			uint32 pmapStepX = pmapBounds.width() / hmapBounds.width();
			uint32 pmapStepY = pmapBounds.height() / hmapBounds.height();
			uint32 pmapY = pmapBounds.top;
			uint32 lastPmapX = 0xFFFFFFFF, lastPmapY = 0xFFFFFFFF;
			int subLevel = chunkNode->getChunkLevel() - loader->hmapLevels;

			//Procedural generation
			float rndMin = 0.0f, rndMax = 0.0f;
			float magScale = tileSize * loader->chunkRes;
			uint32 oy = 0, ox = 0;
			for (uint32 iy = hmapBounds.top; iy < hmapBounds.bottom; ++iy) {
				ox = 0;
				uint32 pmapX = pmapBounds.left;
				for (uint32 ix = hmapBounds.left; ix < hmapBounds.right; ++ix) {
					//If a re-sample of the property map is needed...
					if (pmapX != lastPmapX || pmapY != lastPmapY) {
						uint8 *bytes = propertyMap->getBytesAt(pmapX, pmapY);
						
						//byte[0] = peaks/hills (0 = full hills, 128 = neutral, 255 = full peaks).
						//Converted into random number range shift
						float rangeShift = 2.0f * (((float)(0xFF - bytes[0]) * div255) - 0.5f);
						
						//byte[1] = roughness. Converted into an amount to divide the random variance by each sub-level
						float division = (5.0f * ((float)(0xFF - bytes[1]) * div255)) + 1.0f;
						
						//byte[2] = magnitude. The random variance for the first level
						float magnitude = ((float)bytes[2] * div255) * magScale;

						//Calculate random variance for this level at this pixel
						float randomVariance = magnitude / Math::Pow(division, subLevel-1);

						//Calculate random min and max
						rndMin = -randomVariance + (rangeShift * randomVariance);
						rndMax = randomVariance + (rangeShift * randomVariance);
					}

					float h00 = parentChunkDat->vertexHeights[iy * vertexArrayWidth + ix];
					float h20 = parentChunkDat->vertexHeights[iy * vertexArrayWidth + ix+1];
					float h02 = parentChunkDat->vertexHeights[(iy+1) * vertexArrayWidth + ix];
					float h22 = parentChunkDat->vertexHeights[(iy+1) * vertexArrayWidth + ix+1];

					float h10 = (h00 + h20) * 0.5f;
					float h01 = (h00 + h02) * 0.5f;
					float h11 = (h00 + h20 + h02 + h22) * 0.25f;

					h11 += Math::RangeRandom(rndMin, rndMax);
					h01 += Math::RangeRandom(rndMin, rndMax) * 0.5f;
					h10 += Math::RangeRandom(rndMin, rndMax) * 0.5f;

					vertexHeights[oy * vertexArrayWidth + ox] = h00;
					vertexHeights[oy * vertexArrayWidth + (ox+1)] = h10;
					vertexHeights[(oy+1) * vertexArrayWidth + ox] = h01;
					vertexHeights[(oy+1) * vertexArrayWidth + (ox+1)] = h11;

					ox += 2;
					pmapX += pmapStepX;
				}
				oy += 2;
				pmapY += pmapStepY;
			}

			//Top edge (y = top, x = left to right-1)
			uint32 iy = hmapBounds.top;
			ox = 0; oy = 0;
			for (uint32 ix = hmapBounds.left; ix < hmapBounds.right; ++ix) {
				float h00 = parentChunkDat->vertexHeights[iy * vertexArrayWidth + ix];
				float h20 = parentChunkDat->vertexHeights[iy * vertexArrayWidth + ix+1];
				float h10 = (h00 + h20) * 0.5f;
				vertexHeights[oy * vertexArrayWidth + ox] = h00;
				vertexHeights[oy * vertexArrayWidth + (ox+1)] = h10;
				ox += 2;
			}

			//Bottom edge (y = bottom, x = left to right-1)
			iy = hmapBounds.bottom;
			oy = vertexArrayWidth-1; ox = 0;
			for (uint32 ix = hmapBounds.left; ix < hmapBounds.right; ++ix) {
				float h00 = parentChunkDat->vertexHeights[iy * vertexArrayWidth + ix];
				float h20 = parentChunkDat->vertexHeights[iy * vertexArrayWidth + ix+1];
				float h10 = (h00 + h20) * 0.5f;
				vertexHeights[oy * vertexArrayWidth + ox] = h00;
				vertexHeights[oy * vertexArrayWidth + (ox+1)] = h10;
				ox += 2;
			}

			//Left edge (x = left, y = top to bottom-1)
			uint32 ix = hmapBounds.left;
			ox = 0; oy = 0;
			for (uint32 iy = hmapBounds.top; iy < hmapBounds.bottom; ++iy) {
				float h00 = parentChunkDat->vertexHeights[iy * vertexArrayWidth + ix];
				float h02 = parentChunkDat->vertexHeights[(iy+1) * vertexArrayWidth + ix];
				float h01 = (h00 + h02) * 0.5f;
				vertexHeights[oy * vertexArrayWidth + ox] = h00;
				vertexHeights[(oy+1) * vertexArrayWidth + ox] = h01;
				oy += 2;
			}

			//Right edge (x = right, y = top to bottom-1)
			ix = hmapBounds.right;
			ox = vertexArrayWidth-1; oy = 0;
			for (uint32 iy = hmapBounds.top; iy < hmapBounds.bottom; ++iy) {
				float h00 = parentChunkDat->vertexHeights[iy * vertexArrayWidth + ix];
				float h02 = parentChunkDat->vertexHeights[(iy+1) * vertexArrayWidth + ix];
				float h01 = (h00 + h02) * 0.5f;
				vertexHeights[oy * vertexArrayWidth + ox] = h00;
				vertexHeights[(oy+1) * vertexArrayWidth + ox] = h01;
				oy += 2;
			}

			//Bottom-right pixel (x = right, y = bottom)
			float tmp = parentChunkDat->vertexHeights[hmapBounds.bottom * vertexArrayWidth + hmapBounds.right];
			vertexHeights[vertexArrayWidth * vertexArrayWidth - 1] = tmp;
		}
	}

	PlanetLoader_Hybrid::HybridChunkData::~HybridChunkData()
	{
		delete[] vertexHeights;
	}
}