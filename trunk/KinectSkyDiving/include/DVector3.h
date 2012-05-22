#ifndef _DVECTOR3_H__
#define _DVECTOR3_H__

#include <OgreVector3.h>
#include <OgreQuaternion.h>
#include <math.h>

namespace GalaxyEngine
{
	//A simple 3D vector class, using double precision variables
	struct DVector3
	{
		inline DVector3() {}

		inline DVector3(const DVector3 &vec)
			: x(vec.x), y(vec.y), z(vec.z) {}

		inline DVector3(const Ogre::Vector3 &vec)
			: x(vec.x), y(vec.y), z(vec.z) {}

		inline DVector3(const double ix, const double iy, const double iz)
			: x(ix), y(iy), z(iz) {}

		inline operator Ogre::Vector3() const {
			return Ogre::Vector3(x, y, z);
		}

		inline DVector3& operator=(const DVector3 &vec) {
			x = vec.x; y = vec.y; z = vec.z;
			return *this;
		}

		inline double distance(const Ogre::Vector3 &v) const
		{
			double dx = v.x - x, dy = v.y - y, dz = v.z - z;
			return sqrt(dx * dx + dy * dy + dz * dz);
		}

		inline double distanceSquared(const Ogre::Vector3 &v) const
		{
			double dx = v.x - x, dy = v.y - y, dz = v.z - z;
			return (dx * dx + dy * dy + dz * dz);
		}

		double x, y, z;
	};

}



#endif
