
#include "Stdafx.h"

#include "Core.h"
#include "Exception.h"
#include "ConfigScript.h"

//using namespace Ogre;

namespace GalaxyEngine
{
	Core *Core::singletonPtr = NULL;

	Core::Core(Ogre::String mediaPath, 
		Ogre::SceneManager *sceneMgr, 
		Ogre::RenderWindow *window, 
		Ogre::Viewport *viewPort, 
		Ogre::Camera *camera,
		Ogre::Root *mRoot)
	{
		if (singletonPtr) EXCEPTION("Cannot create more than one instance of a singleton class", "Core::Core()");
		singletonPtr = this;

		Core::mediaPath = mediaPath;
		Core::mSceneMgr = sceneMgr;
		Core::mWindow = window;
		Core::mViewPort = viewPort;
		Core::mCamera = camera;
		Core::mRoot = mRoot;

		//Create the default main camera and viewport
		viewPort->setBackgroundColour(Ogre::ColourValue(0, 0, 0));
		camera->setAspectRatio(Ogre::Real(viewPort->getActualWidth()) / Ogre::Real(viewPort->getActualHeight()));
	}

	Core::~Core()
	{
		singletonPtr = NULL;
		delete ConfigScriptLoader::getSingletonPtr();
	}

	void Core::runSimulation()
	{
		universe = new Universe("TestUniverse");
		while(mRoot->renderOneFrame()) { universe->update(); }
	}

	Planet* Core::getFirstPlanet()
	{
		PlanetProxy* planetProxy = universe->getFirstPlanet();	
		return planetProxy->getPlanet();
	}
}


