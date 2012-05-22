
#include "Stdafx.h"

#include "PlanetMath.h"

#include <math.h>
using namespace Ogre;

namespace GalaxyEngine
{
	namespace PlanetMath
	{
		Vector3 mapCubeToUnitSphere(const Vector3 &cubeCoord)
		{
			Real x = cubeCoord.x;
			Real y = cubeCoord.y;
			Real z = cubeCoord.z;

			assert(x >= -1 && x <= 1 && y >= -1 && y <= 1 && z >= -1 && z <= 1);

			Vector3 sphereCoord;
			const Real div3 = 1.0f / 3.0f;
			sphereCoord.x = x * Math::Sqrt(1.0f - y * y * 0.5f - z * z * 0.5f + y * y * z * z * div3);
			sphereCoord.y = y * Math::Sqrt(1.0f - z * z * 0.5f - x * x * 0.5f + z * z * x * x * div3);
			sphereCoord.z = z * Math::Sqrt(1.0f - x * x * 0.5f - y * y * 0.5f + x * x * y * y * div3);

			return sphereCoord;
		}

		struct WrapParams
		{
			WrapParams(bool flip_X, bool flip_Y, bool swap_XY) 
				: swapXY(swap_XY), flipX(flip_X), flipY(flip_Y) {}

			bool swapXY;
			bool flipX, flipY;
		};

		CubeFace wrapCubeFaceIndexes(int &x, int &y, const int maxcoord, const CubeFace face)
		{
			//This array stores a bunch of WrapParam values, which indicate how the x/y coordinates need to
			//be transformed in order to be wrapped properly to the destination cube face.
			const bool T = true, F = false;
			static const WrapParams params[6][4] = {
				//Left, Right, Top, Bottom
				{ WrapParams(F, F, F), WrapParams(F, F, F), WrapParams(F, F, F), WrapParams(F, F, F) },	//Front
				{ WrapParams(F, F, F), WrapParams(F, F, F), WrapParams(T, T, F), WrapParams(T, T, F) },	//Back
				{ WrapParams(F, F, F), WrapParams(F, F, F), WrapParams(T, F, T), WrapParams(F, T, T) },	//Right
				{ WrapParams(F, F, F), WrapParams(F, F, F), WrapParams(F, T, T), WrapParams(T, F, T) },	//Left
				{ WrapParams(T, F, T), WrapParams(F, T, T), WrapParams(T, T, F), WrapParams(F, F, F) },	//Top
				{ WrapParams(F, T, T), WrapParams(T, F, T), WrapParams(F, F, F), WrapParams(T, T, F) }	//Bottom
			};

			Side neighbor = SIDE_None;

			//Initial X wrap
			if (x > maxcoord)
			{
				x -= maxcoord;
				neighbor = SIDE_Right;
			} 
			else if (x < 0) 
			{
				x += maxcoord;
				neighbor = SIDE_Left;
			}

			//Initial Y wrap
			if (y > maxcoord)
			{
				y -= maxcoord;
				neighbor = SIDE_Bottom;
			} 
			else if (y < 0) 
			{
				y += maxcoord;
				neighbor = SIDE_Top;
			}

			//If nothing was out of bounds (wrapped), no wrapping should occur
			if (neighbor == SIDE_None)
				return face;

			//Get wrapping parameters
			const WrapParams &wparam = params[face][neighbor];

			//Perform flips / swaps
			if (wparam.flipX)
				x = maxcoord - x;

			if (wparam.flipY)
				y = maxcoord - y;

			if (wparam.swapXY) 
			{
				int tmp = x;
				x = y;
				y = tmp;
			}

			//Return the new neighbor face
			return getNeighborCubeFace(face, neighbor);
		}

		/*CubeFace wrapCubeFaceIndexes2(int &x, int &y, const int maxcoord, const CubeFace face)
		{
		int xx = x, yy = y;
		CubeFace oldFace = face;
		CubeFace newFace = wrapCubeFaceIndexes(xx, yy, maxcoord, oldFace);
		while (newFace != oldFace) {
		oldFace = newFace;
		newFace = wrapCubeFaceIndexes(xx, yy, maxcoord, oldFace);
		}
		x = xx; y = yy;
		return newFace;
		}*/
	}
}

