#ifndef _CORE_H__
#define _CORE_H__

#include "Universe.h"

#include <OgreString.h>
#include <OgreRenderWindow.h>

namespace GalaxyEngine
{
	class Core
	{
	public:
		Core(Ogre::String mediaPath, 
			Ogre::SceneManager *sceneMgr, 
			Ogre::RenderWindow *window, 
			Ogre::Viewport *viewPort, 
			Ogre::Camera *camera,
			Ogre::Root* mRoot);
		~Core();

		inline static Core &getSingleton(){ return *singletonPtr; }
		inline static Core *getSingletonPtr() { return singletonPtr; }

		inline Ogre::String getMediaPath() { return mediaPath; }
		inline Ogre::SceneManager *getSceneManager() { return mSceneMgr; }
		inline Ogre::RenderWindow *getRenderWindow() { return mWindow; }
		inline Ogre::Viewport *getViewport() { return mViewPort; }
		inline Ogre::Camera *getCamera() { return mCamera; }
		inline Universe *getUniverse() { return universe; }

		void execute();

		Planet* getFirstPlanet();

	private:
		Universe *universe;
		static Core *singletonPtr;
		Ogre::String mediaPath;

		Ogre::Root *mRoot;
		Ogre::SceneManager *mSceneMgr;
		Ogre::RenderWindow *mWindow;
		Ogre::Viewport *mViewPort;
		Ogre::Camera *mCamera;
	};
}




#endif