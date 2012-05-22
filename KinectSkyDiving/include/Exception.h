#ifndef _EXCEPTION_H__
#define _EXCEPTION_H__

#include <OgreString.h>

namespace GalaxyEngine
{
	#define EXCEPTION(desc, src) throw GalaxyEngine::Exception(desc, src, __FILE__)

	class Exception: public std::exception
	{
	public:
		Exception(const Ogre::String &description, const Ogre::String &source, const char *file);
		~Exception() throw() {}
		
		const Ogre::String &getFullDescription() const;
		const Ogre::String &getSource() const { return source; }
		const Ogre::String &getFile() const { return file; }
		const Ogre::String &getDescription(void) const { return description; }

		const char* what() const throw() { return getFullDescription().c_str(); }

	private:
		Ogre::String description;
		Ogre::String source;
		Ogre::String file;
		mutable Ogre::String fullDescription;
	};

}




#endif
