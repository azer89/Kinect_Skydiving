
#include "Stdafx.h"

#define APPLICATION_TITLE "DreamStar"

//Include the GalaxyEngine core
#include "App.h"
#include "Core.h"
#include "Exception.h"


#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif


#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#ifdef _DEBUG
	int main()
#else
	INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#endif	
#else
	int main(int argc, char *argv[])
#endif
	{
        // Create application object
        App app;

        try {
            app.go();
        } catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
            MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
            std::cerr << "An exception has occured: " <<
                e.getFullDescription().c_str() << std::endl;
#endif
        }

        return 0;
    }

#ifdef __cplusplus
}
#endif
