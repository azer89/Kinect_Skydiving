
#include "Stdafx.h"
#include "App.h"
#include "Interface.h"

//-------------------------------------------------------------------------------------
App::App(void)
	: planetEngine(0),
	  gameSystem(0),
	  gameConfig(0),
	  UI(0),
	  isGameStarted(false)
{
}

//-------------------------------------------------------------------------------------
App::~App(void)
{	
	if(gameSystem != 0)		delete gameSystem;
	if(planetEngine != 0)	delete planetEngine;
	if(gameConfig != 0)		delete gameConfig;
	if(UI != 0)				delete UI;
}

//-------------------------------------------------------------------------------------
void App::createScene(void)
{	
	gameConfig = new GameConfig("game.config", "Popular");

	mCameraMan->setTopSpeed(GameConfig::getSingletonPtr()->getMCameraManSpeed());

	planetEngine = new GalaxyEngine::Core("../../media", this->mSceneMgr, this->mWindow, this->mCamera->getViewport(), this->mCamera, this->mRoot);
	gameSystem = new GameSystem();
	this->universe = planetEngine->getUniverse();

	mLoadingBar->update();

	gameSystem->initSystem(mRoot, mCamera, mSceneMgr, mMouse, mKeyboard, mWindow, planetEngine, mLoadingBar);
	gameSystem->createScene();

	mLoadingBar->update();

	UI = new Interface(this);
	UI->setupHikari();
	UI->getStartMenu()->hide();			// debug
	UI->getGameDisplay()->show();		// debug
	gameSystem->startGame();			// debug

	/*Ogre::ColourValue fadeColour(0, 168.0/255.0, 1.0);
	mSceneMgr->setFog(Ogre::FOG_LINEAR, fadeColour, 0.0, 0.001, 10000);
	mWindow->getViewport(0)->setBackgroundColour(fadeColour);*/
}

//--------------------------------------------------------------------------------------
bool App::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
	if(!BaseApplication::frameRenderingQueued(evt)) { return false; }

	//if(mLoadingBar->isLoadingFinished() && !UI->getStartMenu()->getVisibility())
	//{
	//	UI->showMainMenu();
	//}

	if(UI != 0) UI->getHikariManager()->update();
	gameSystem->update(evt.timeSinceLastEvent);

	UI->updateArrow(gameSystem->getArrowDirection());
	UI->updateScore(gameSystem->getScore());

	bool isUpsideTarget = gameSystem->isUpsideTarget();
	if(isUpsideTarget && UI->getArrowVisibility()) UI->hideArrow();
	else if(!isUpsideTarget && !UI->getArrowVisibility()) UI->showArrow();

	this->universe->update();

	return true;
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
	if(!BaseApplication::mouseMoved(arg)) return false;	
	bool result = true;
	if(UI != 0) result = UI->getHikariManager()->injectMouseMove(arg.state.X.abs, arg.state.Y.abs);
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
	if(UI != 0) result = UI->getHikariManager()->injectMouseDown(id);
	return result;
}

//--------------------------------------------------------------------------------------
void App::injectMouseMove(float x, float y)
{
	if(UI != 0) UI->getHikariManager()->injectMouseMove(x,y);
}

//--------------------------------------------------------------------------------------
void App::startGame(void)
{
	isGameStarted = true;
	gameSystem->startGame();
}

