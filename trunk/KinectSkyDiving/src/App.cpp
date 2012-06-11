
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
	if(gameSystem != 0) delete gameSystem;
	if(planetEngine != 0) delete planetEngine;
}

//-------------------------------------------------------------------------------------
void App::createScene(void)
{	
	new GameConfig("game.config", "Popular");

	mCameraMan->setTopSpeed(GameConfig::getSingletonPtr()->getMCameraManSpeed());

	planetEngine = new GalaxyEngine::Core("../../media", this->mSceneMgr, this->mWindow, this->mCamera->getViewport(), this->mCamera, this->mRoot);
	gameSystem = new GameSystem();

	mLoadingBar->update();

	gameSystem->initSystem(mRoot, mCamera, mSceneMgr, mMouse, mKeyboard, mWindow, planetEngine, mLoadingBar);
	gameSystem->createScene();

	mLoadingBar->update();

	/*Ogre::ColourValue fadeColour(0, 168.0/255.0, 1.0);
	mSceneMgr->setFog(Ogre::FOG_LINEAR, fadeColour, 0.0, 0.001, 10000);
	mWindow->getViewport(0)->setBackgroundColour(fadeColour);*/
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
	if(gameSystem != 0) planetEngine->runSimulation();		//	Run the planet simulation	
	BaseApplication::destroyScene();		
}

//--------------------------------------------------------------------------------------
 bool App::keyPressed( const OIS::KeyEvent &arg )
{	
	if(arg.key == OIS::KC_UP || 
		arg.key == OIS::KC_DOWN || 
		arg.key == OIS::KC_LEFT || 
		arg.key == OIS::KC_RIGHT || 
		arg.key == OIS::KC_ESCAPE ||
		arg.key == OIS::KC_G ||
		arg.key == OIS::KC_R)
		BaseApplication::keyPressed( arg );

	gameSystem->keyPressed(arg);
	return true;
}

//--------------------------------------------------------------------------------------
bool App::keyReleased( const OIS::KeyEvent &arg )
{
	bool result = BaseApplication::keyReleased( arg );
	gameSystem->keyReleased(arg);
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

