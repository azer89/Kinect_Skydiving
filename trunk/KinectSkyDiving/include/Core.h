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
		Core(const Ogre::String &windowTitle, const Ogre::String &mediaPath);
		Core(Ogre::String mediaPath, Ogre::SceneManager *sceneMgr, 
			Ogre::RenderWindow *window, 
			Ogre::Viewport *viewPort, 
			Ogre::Camera *camera);
		~Core();

		inline static Core &getSingleton(){ return *singletonPtr; }
		inline static Core *getSingletonPtr() { return singletonPtr; }

		inline Ogre::String getMediaPath() { return mediaPath; }
		inline Ogre::SceneManager *getSceneManager() { return mSceneMgr; }
		inline Ogre::RenderWindow *getRenderWindow() { return mWindow; }
		inline Ogre::Viewport *getViewport() { return mViewPort; }
		inline Ogre::Camera *getCamera() { return mCamera; }

		void execute();

		void runSimulation();
		//void endSimulation() { running = false; }

		Planet* getFirstPlanet();

	private:
		//bool running;

		Universe *universe;

		static Core *singletonPtr;
		Ogre::String mediaPath;

		Ogre::SceneManager *mSceneMgr;
		Ogre::RenderWindow *mWindow;
		Ogre::Viewport *mViewPort;
		Ogre::Camera *mCamera;

	//private:

		//void updateWorld();
		//void renderWorld();
	};
}




#endif