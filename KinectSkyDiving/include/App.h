

#ifndef __App_h_
#define __App_h_

#include "BaseApplication.h"
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#include "../res/resource.h"
#endif

#include "Core.h"
#include "Universe.h"
#include "Planet.h"
//#include "RayCastCollision.h"
#include "GameSystem.h"

class App : public BaseApplication
{
public:
    App(void);
    virtual ~App(void);

	/** Start the system and init planet rendering engine */
	virtual void go(void);

protected:
	GalaxyEngine::Core *planetEngine;				// planet rendering engine
	//RayCastCollision* collisionDetector;			// simple planet collision engine
	//Ogre::Real colissionDelay;					// collision engine isn't optimized so make it run every 1/60 second
	GameSystem* gameSystem;							// game play system

protected:
	/** Check planet collision, similar to RaySceneQuery */
	//void checkPlanetColission(Ogre::Real timeElapsed);
    virtual void createScene(void);
	virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);

	virtual bool mouseMoved( const OIS::MouseEvent &arg );
	virtual bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
	virtual bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );

	virtual bool keyPressed( const OIS::KeyEvent &arg );
	virtual bool keyReleased( const OIS::KeyEvent &arg );
};

#endif // #ifndef __App_h_
