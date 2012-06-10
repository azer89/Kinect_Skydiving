
#include "Stdafx.h"

#include "Utility.h"

//using namespace Ogre;

#include <stdio.h>
#include <ddraw.h>


namespace GalaxyEngine
{
	namespace Utility
	{
		unsigned long GUID;

		void saveVolumeToDDS(const Ogre::String &filename, int width, int height, int depth, const char *colors)
		{
			FILE *file = fopen(filename.c_str(), "w");
			if (file == NULL)
				return;

			//Magic value
			fwrite("DDS ", 1, 4, file);

			//Surface format header
			DDSURFACEDESC2 header;
			memset((void*)&header, 0, sizeof(header));
			header.dwSize = 124;
			header.dwFlags = DDSD_CAPS | DDSD_PIXELFORMAT | DDSD_WIDTH | DDSD_HEIGHT | DDSD_DEPTH;
			header.dwWidth = width;
			header.dwHeight = height;
			header.dwDepth = depth;
			header.dwMipMapCount = 0;
			header.ddpfPixelFormat.dwSize = sizeof(header.ddpfPixelFormat);
			header.ddpfPixelFormat.dwFlags = DDPF_RGB;
			header.ddpfPixelFormat.dwRGBBitCount = 24;
			header.ddpfPixelFormat.dwRBitMask = 0xFF0000;
			header.ddpfPixelFormat.dwGBitMask = 0x00FF00;
			header.ddpfPixelFormat.dwBBitMask = 0x0000FF;
			header.ddpfPixelFormat.dwRGBAlphaBitMask = 0;
			header.ddsCaps.dwCaps = DDSCAPS_TEXTURE;
			header.ddsCaps.dwCaps2 = DDSCAPS2_VOLUME;
			header.ddsCaps.dwVolumeDepth = depth;
			fwrite((char*)&header, 1, sizeof(header), file);

			size_t byteCount = width * height * depth * 3;
			fwrite(colors, 1, byteCount, file);

			fclose(file);
		}

	}
}
