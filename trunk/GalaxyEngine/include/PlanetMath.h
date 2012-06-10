#ifndef _PLANETMATH_H__
#define _PLANETMATH_H__

#include <OgreString.h>
#include <OgreStringConverter.h>
#include <OgreTimer.h>
#include <OgreVector3.h>
#include <OgreQuaternion.h>
#include <math.h>

namespace GalaxyEngine
{
	namespace PlanetMath
	{
		//The 6 sides of a cube
		enum CubeFace {
			CUBEFACE_Front = 0,
			CUBEFACE_Back = 1,
			CUBEFACE_Right = 2,
			CUBEFACE_Left = 3,
			CUBEFACE_Top = 4,
			CUBEFACE_Bottom = 5,
			CUBEFACE_Size = 6,
			CUBEFACE_None = 255
		};

		//4 "quadrants" of a square divided into 4 smaller squares)
		enum Quadrant {
			QUADRANT_TopLeft = 0,
			QUADRANT_TopRight = 1,
			QUADRANT_BottomLeft = 2,
			QUADRANT_BottomRight = 3,
			QUADRANT_Size = 4,
			QUADRANT_None = 255
		};

		//4 sides of a 2D square (left, right, top, and bottom)
		enum Side 
		{
			SIDE_Left = 0,
			SIDE_Right = 1,
			SIDE_Top = 2,
			SIDE_Bottom = 3,
			SIDE_Size = 4,
			SIDE_None = 255
		};

		//Maps plane coordinates (where top left = [0,0] and bottom right = [1,1])
		//to a coordinates on a unit cube (corners at [-1,-1,-1] and [+1,+1,+1]), based on
		//which face of the cube the coordinates belong to.
		inline Ogre::Vector3 mapPlaneToCube(const float tx, const float ty, const CubeFace cubeFace)
		{
			Ogre::Vector3 pos;
			switch (cubeFace) {
					case CUBEFACE_Front:
						pos.x = 2*tx-1; pos.y = 1-2*ty; pos.z = 1;
						break;
					case CUBEFACE_Back:
						pos.x = 1-2*tx; pos.y = 1-2*ty; pos.z = -1;
						break;
					case CUBEFACE_Right:
						pos.x = 1; pos.y = 1-2*ty; pos.z = 1-2*tx;
						break;
					case CUBEFACE_Left:
						pos.x = -1; pos.y = 1-2*ty; pos.z = 2*tx-1;
						break;
					case CUBEFACE_Top:
						pos.x = 2*tx-1; pos.y = 1; pos.z = 2*ty-1;
						break;
					case CUBEFACE_Bottom:
						pos.x = 2*tx-1; pos.y = -1; pos.z = 1-2*ty;
						break;
			}
			return pos;
		}

		/*inline const Quaternion &getCubeFaceOrientation(const CubeFace cubeFace)
		{
		static const Quaternion quat[6] = {
		Quaternion::IDENTITY,
		Quaternion(Degree(180), Vector3::UNIT_Y),
		Quaternion(Degree(-90), Vector3::UNIT_Y),
		Quaternion(Degree(90), Vector3::UNIT_Y),
		Quaternion(Degree(90), Vector3::UNIT_X),
		Quaternion(Degree(-90), Vector3::UNIT_X)
		};
		return quat[cubeFace];
		}*/

		//Returns the neighboring cube face of the current face in the given direction.
		//For example, calling getNeighborCubeFace(CUBEFACE_Front, SIDE_Right) will return
		//CUBEFACE_Right, calling getNeighborCubeFace(CUBEFACE_Right, SIDE_Left) will return
		//CUBEFACE_Front, etc.
		inline CubeFace getNeighborCubeFace(const CubeFace currentFace, const Side neighbor)
		{
			static const CubeFace neighbors[6][4] = {
				//Left, Right, Top, Bottom
				{ CUBEFACE_Left, CUBEFACE_Right, CUBEFACE_Top, CUBEFACE_Bottom },	//Front
				{ CUBEFACE_Right, CUBEFACE_Left, CUBEFACE_Top, CUBEFACE_Bottom },	//Back
				{ CUBEFACE_Front, CUBEFACE_Back, CUBEFACE_Top, CUBEFACE_Bottom },	//Right
				{ CUBEFACE_Back, CUBEFACE_Front, CUBEFACE_Top, CUBEFACE_Bottom },	//Left
				{ CUBEFACE_Left, CUBEFACE_Right, CUBEFACE_Back, CUBEFACE_Front },	//Top
				{ CUBEFACE_Left, CUBEFACE_Right, CUBEFACE_Front, CUBEFACE_Back }	//Bottom
			};
			return neighbors[currentFace][neighbor];
		}

		//Wraps the given integer coordinates around to a neighboring cube face if they are out of their current
		//face's bounds. This is used when height data, etc. is needed beyond the current cube face. Returns the
		//cube face that the coordinates were wrapped to (if wrapping was not needed, "face" will be returned
		//and the x/y values will be unmodified). "maxcoord" should be set to the maximum value of x or y (0
		//is assumed to be the minimum). Note that if your x/y grid is 256x256, the maxcoord should be 255, not 256,
		//since 255 is the maximum index for x and y.
		CubeFace wrapCubeFaceIndexes(int &x, int &y, const int maxcoord, const CubeFace face);

		//A variation of wrapCubeFaceIndexes() that allows you to wrap diagonally (the other version only wraps
		//left/right/up/down).
		//CubeFace wrapCubeFaceIndexes2(int &x, int &y, const int maxcoord, const CubeFace face);

		//Maps a point on a unit cube (corners at [-1,-1,-1] and [+1,+1,+1]) to a
		//unit sphere (radius of 1.0) in a special way designed to eliminate distortion
		//near the cube face seams (unlike simply normalizing the cube coordinate vector).
		Ogre::Vector3 mapCubeToUnitSphere(const Ogre::Vector3 &cubeCoord);
	}
}



#endif
