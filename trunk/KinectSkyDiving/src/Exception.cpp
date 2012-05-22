
#include "Stdafx.h"

#include "Exception.h"

#include <OgreString.h>
using namespace Ogre;

namespace GalaxyEngine
{
	Exception::Exception(const String &description, const String &source, const char *file)
	{
		this->description = description;
		this->source = source;
		this->file = file;
		fullDescription = "";
	}

	const String &Exception::getFullDescription() const
	{
		if (fullDescription.empty())
		{
			fullDescription = "GalaxyEngine exception at \"" + source + "\" in \"" + file + "\": " + description;
		}

		return fullDescription;
	}
}
