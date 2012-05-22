
#include "Stdafx.h"

#include "Core.h"
#include "Exception.h"
#include "ConfigScript.h"

using namespace Ogre;

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
		viewPort->setBackgroundColour(ColourValue(0, 0, 0));
		camera->setAspectRatio(Real(viewPort->getActualWidth()) / Real(viewPort->getActualHeight()));

		//Don't clear the screen color every frame, since the skybox will serve that purpose
		//viewPort->setClearEveryFrame(true, FBT_DEPTH);	
		/*
		camera->setPosition(0, 250.0f, 0);
		camera->lookAt(0, 0, 0);
		//camera->setDirection(0, -1, 0);
		Quaternion q;
		q.FromAngleAxis(Ogre::Degree(-90), Ogre::Vector3::UNIT_X);
		camera->setOrientation(q);
		camera->setNearClipDistance(0.001f);
		camera->setFarClipDistance(1000.0f);
		//camera->setFOVy(Degree(60));
		*/
		/*
		sceneMgr->setAmbientLight(Ogre::ColourValue(0.3, 0.3, 0.3));
		Ogre::Light* light = sceneMgr->createLight();
		light->setType(Ogre::Light::LT_DIRECTIONAL);
		light->setPosition(40, 0, 200);
		light->setDirection(Ogre::Vector3(40, 0, 0));
		light->setSpecularColour(Ogre::ColourValue::White);
		*/

		//running = false;
	}

	Core::~Core()
	{
		singletonPtr = NULL;
		delete ConfigScriptLoader::getSingletonPtr();

		//Destroy Ogre Graphics
		//delete Root::getSingletonPtr();
	}

	void Core::runSimulation()
	{
		universe = new Universe(Core::getSingleton().getCamera(), "TestUniverse");
		while(mRoot->renderOneFrame()) { universe->update(); }
	}

	//void Core::renderWorld()
	//{
	//	Ogre::Root::getSingleton().renderOneFrame();
	//}

	//void Core::updateWorld()
	//{
	//	universe->update();
	//}

	Planet* Core::getFirstPlanet()
	{
		PlanetProxy* planetProxy = universe->getFirstPlanet();	
		return planetProxy->getPlanet();
	}
}


