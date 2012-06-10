#ifndef _PLANETHEIGHTMAPLOADER_H__
#define _PLANETHEIGHTMAPLOADER_H__

#include "Planet.h"
#include <OgreResourceBackgroundQueue.h>
#include <OgreLight.h>
#include <list>

namespace GalaxyEngine
{
	class PlanetLoader_Hybrid: public PlanetLoader
	{
	public:
		PlanetLoader_Hybrid(Ogre::Real radius, Ogre::Real maxTerrainHeight, Ogre::uint32 chunkResolution = 32);
		~PlanetLoader_Hybrid();
		
		Ogre::Real getInnerRadius() { return innerRadius; }
		Ogre::Real getOuterRadius() { return outerRadius; }
		void loadPlanet(const Ogre::String &planetName = "");
		bool isFinishedLoading();
		void setHeightMap(const PlanetMath::CubeFace cubeFace, const Ogre::String &fileName);
		void setPropertyMap(const PlanetMath::CubeFace cubeFace, const Ogre::String &fileName);
		void setNormalMap(const PlanetMath::CubeFace cubeFace, const Ogre::String &fileName);
		void setColorMap(const PlanetMath::CubeFace cubeFace, const Ogre::String &fileName);		
		void setVirtualResolution(const Ogre::uint32 mapSize);		
		void setLightingEffects(bool highResNormals, bool ambientOcclusion) 
		{
			this->highResNormals = highResNormals;
			this->ambientOcclusion = ambientOcclusion;
			_generateShaders();
		}
		void updateLighting(Ogre::Light *light, Ogre::SceneNode *planet);		
		void loadChunkMesh(ChunkMesh &mesh, Planet::ChunkNode *chunk, Planet *planet);
		void unloadChunkMesh(Planet::ChunkNode *chunk, Planet *planet);
		Ogre::uint32 getMaxLevels() { return maxLevels; }
		void generateNormalMaps();

		friend class HybridChunkData;
		class HybridChunkData
		{
		public:
			HybridChunkData(PlanetLoader_Hybrid *loader, Planet::ChunkNode *chunkNode);
			~HybridChunkData();
			inline float getTerrainHeight(Ogre::uint32 x, Ogre::uint32 y) {
				return vertexHeights[(y * vertexArrayWidth) + x];
			}

		private:
			//Floating point vertex heightmap for this chunk. The size of this array
			//will be (chunkRes+1) x (chunkRes+1), where chunkRes is the width/height
			//of a single terrain chunk (PlanetLoader_Hybrid::chunkRes)
			float *vertexHeights;
			Ogre::uint32 vertexArrayWidth;
			//The memory taken up by these arrays is trivial compared to the height map
			//and property map data, so they are full floats. Even if there are 100
			//chunks of 33x33xfloat, that would only take about 400 KB total.
		};

	private:
		struct ByteMap
		{
			ByteMap() : width(0), height(0), Bpp(0), data(NULL) {}
			~ByteMap() { deallocate(); }

			void allocate(Ogre::uint32 width, Ogre::uint32 height, Ogre::uint32 bytesPerPixel = 1) {
				assert(data == NULL);
				this->width = width; this->height = height; Bpp = bytesPerPixel;
				data = new Ogre::uint8[width * height * Bpp];
			}
			void deallocate() {
				if (data) delete[] data;
				data = NULL;
				width = 0; height = 0; Bpp = 0;
			}

			inline bool isAllocated() const { return (data != NULL); }

			//For byte maps with one byte per pixel
			inline Ogre::uint8 getByteAt(Ogre::uint32 x, Ogre::uint32 y) const {
				assert(x < width && y < height);
				assert(data); assert(Bpp == 1);
				return data[y * width + x];
			}
			
			//For byte maps with multiple bytes per pixel
			inline Ogre::uint8 *getBytesAt(Ogre::uint32 x, Ogre::uint32 y) {
				assert(x < width && y < height);
				assert(data); assert(Bpp > 1);
				return &data[y * width * Bpp + x];
			}

			//Returns the total number of bytes used by this ByteMap (equivalent to width*height*bytesPerPixel)
			inline Ogre::uint32 getTotalBytes() const { return width * height * Bpp; }

			Ogre::uint32 width, height, Bpp;
			Ogre::uint8 *data;
		};

		void _generateShaders();

		Ogre::String loadedPlanetName;
		std::list<Ogre::BackgroundProcessTicket> ticketList;	//A list of all active background loading processes

		//Materials
		Ogre::MaterialPtr material[6], material_highResNormals[6];

		//Height map / property map
		ByteMap heightMap[6], propertyMap[6];

		//Shading options
		bool highResNormals;
		bool ambientOcclusion;

		//Normal generation
		Ogre::Vector3 *normalBuffer;		//Normal buffer (square 2D)
		Ogre::uint32 normalBufferSize;		//width/height value

		//Radius
		Ogre::Real innerRadius, outerRadius;

		//LOD data
		Ogre::uint32 chunkRes;
		Ogre::uint32 hmapLevels, maxLevels;

		Ogre::IndexData sharedIndexData;
	};

}



#endif