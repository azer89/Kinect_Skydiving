
#include "Stdafx.h"

#include "Exception.h"

#include <OgreString.h>
//using namespace Ogre;

namespace GalaxyEngine
{
	Exception::Exception(const Ogre::String &description, const Ogre::String &source, const char *file)
	{
		this->description = description;
		this->source = source;
		this->file = file;
		fullDescription = "";
	}

	const Ogre::String &Exception::getFullDescription() const
	{
		if (fullDescription.empty())
		{
			fullDescription = "GalaxyEngine exception at \"" + source + "\" in \"" + file + "\": " + description;
		}

		return fullDescription;
	}
}
