#ifndef _UTILITY_H__
#define _UTILITY_H__

#include <OgreString.h>
#include <OgreStringConverter.h>
#include <OgreTimer.h>

namespace GalaxyEngine
{
	namespace Utility
	{
		extern unsigned long GUID;

		class Timer
		{
		public:
			unsigned long getMilliseconds() { return t.getMilliseconds(); }
			void reset() { return t.reset(); }

		private:
			Ogre::Timer t;
		};

		//Gets a globally unique ID
		inline Ogre::String getUniqueID()
		{
			return Ogre::StringConverter::toString(++GUID) + "Glxy";
		}

		void saveVolumeToDDS(const Ogre::String &filename, int width, int height, int depth, const char *colors);

	}

}



#endif
