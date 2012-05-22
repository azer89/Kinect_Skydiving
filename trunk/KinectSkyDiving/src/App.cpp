
#include "Stdafx.h"

#include "App.h"

//-------------------------------------------------------------------------------------
App::App(void)
	: planetEngine(0),
	  gameSystem(0)
{
}
//-------------------------------------------------------------------------------------
App::~App(void)
{
	if(planetEngine != 0) delete planetEngine;
	if(gameSystem != 0) delete gameSystem;
}

//-------------------------------------------------------------------------------------
void App::createScene(void)
{	
	planetEngine = new GalaxyEngine::Core("../../media", this->mSceneMgr, this->mWindow, this->mCamera->getViewport(), this->mCamera, this->mRoot);
	gameSystem = new GameSystem();
	gameSystem->initSystem(mRoot, mCamera, mSceneMgr, mMouse, mKeyboard, planetEngine);
	gameSystem->createScene();

	/*
	Ogre::Entity* ent = mSceneMgr->createEntity("sdawd", "bomberman.mesh");
	Ogre::SceneNode* nodee = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	nodee->attachObject(ent);*/
}

//--------------------------------------------------------------------------------------
bool App::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
	if(!BaseApplication::frameRenderingQueued(evt)) { return false; }	
	gameSystem->update(evt.timeSinceLastEvent);
	return true;
}

//--------------------------------------------------------------------------------------
/** Run the system, called on main() */
void App::go(void)
{		
	BaseApplication::go();		
	planetEngine->runSimulation();		//	Run the planet simulation	
	BaseApplication::destroyScene();		
}

//--------------------------------------------------------------------------------------
 bool App::keyPressed( const OIS::KeyEvent &arg )
{
	bool result = BaseApplication::keyPressed( arg );
	return result;
}

//--------------------------------------------------------------------------------------
bool App::keyReleased( const OIS::KeyEvent &arg )
{
	bool result = BaseApplication::keyReleased( arg );
	return result;
}

//--------------------------------------------------------------------------------------
bool App::mouseMoved( const OIS::MouseEvent &arg )
{
	bool result = BaseApplication::mouseMoved(arg);
	return result;
}

//--------------------------------------------------------------------------------------
bool App::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
	bool result = BaseApplication::mouseReleased(arg, id);
	return result;
}

//--------------------------------------------------------------------------------------
bool App::mousePressed(const OIS::MouseEvent& arg, OIS::MouseButtonID id)
{
	bool result = BaseApplication::mousePressed(arg, id);
	return result;
}

