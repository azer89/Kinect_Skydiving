
#include "Stdafx.h"

#include "App.h"

//-------------------------------------------------------------------------------------
App::App(void)
	: //colissionDelay(0),
	  planetEngine(0),
	  //collisionDetector(0),
	  gameSystem(0)
{
}
//-------------------------------------------------------------------------------------
App::~App(void)
{
	if(planetEngine != 0) delete planetEngine;
	//if(collisionDetector != 0) delete collisionDetector;
	if(gameSystem != 0) delete gameSystem;
}

//-------------------------------------------------------------------------------------
void App::createScene(void)
{	
}

//--------------------------------------------------------------------------------------
bool App::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
	if(!BaseApplication::frameRenderingQueued(evt)) { return false; }	
	//checkPlanetColission(evt.timeSinceLastFrame);	
	gameSystem->update(evt.timeSinceLastEvent);

	return true;
}

//--------------------------------------------------------------------------------------
/*
void App::checkPlanetColission(Ogre::Real timeElapsed)
{
	colissionDelay += timeElapsed;
	if(colissionDelay < 0.0166f) return;		// 60 fps
	colissionDelay = 0;							// reset

	Ogre::Vector3 camPos = mCamera->getDerivedPosition();
	Ogre::Real distance1 = Ogre::Math::Abs(Ogre::Vector3::ZERO.distance(camPos));	// distance of the position to center of planet
	GalaxyEngine::Planet* planet = planetEngine->getFirstPlanet();					// we only have one planet
	Ogre::Real radius = planet->getBoundingRadius();								// radius of bounding of the planet

	if(distance1 < radius)
	{		
		Ogre::Vector3 result = Ogre::Vector3::ZERO;		// resulted intersection
		Ogre::Vector3 camToCenter = -camPos;			// direction to the center
		camToCenter.normalise();						
		collisionDetector->getPlanetIntersection(planet, camPos, camToCenter, result);
		Ogre::Real distance2 = Ogre::Math::Abs(Ogre::Vector3::ZERO.distance(result));
		Ogre::Real halfRadius = radius * 0.5f;

		if(distance2 < (radius - halfRadius)  || distance2 > (radius + halfRadius))		// don't know why but sometimes it's invalid
		{
		}
		else if(distance1 < (distance2 + 2.0f))
		{
			Ogre::Vector3 pointUp = -camToCenter;
			mCamera->setPosition(camPos + (pointUp * 0.5f));
			//std::cout << distance2 << "\n";
		}

	}
}
*/
//--------------------------------------------------------------------------------------
/** Run the system, called on main() */
void App::go(void)
{	
	BaseApplication::go();	
	planetEngine = new GalaxyEngine::Core("../../media", this->mSceneMgr, this->mWindow, this->mCamera->getViewport(), this->mCamera);

	gameSystem = new GameSystem();
	gameSystem->initSystem(mRoot, mCamera, mSceneMgr, mMouse, mKeyboard, planetEngine);

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

