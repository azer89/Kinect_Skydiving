
#ifndef __App_h_
#define __App_h_

#include "BaseApplication.h"
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#include "../res/resource.h"
#endif

#include "CompositorSample.h"
#include "Core.h"
#include "Universe.h"
#include "Planet.h"
#include "GameSystem.h"
#include "GameConfig.h"

// forward declaration
class Interface;
class KinectUIControl;

class App : public BaseApplication
{
public:
    App(void);
    virtual ~App(void);
	
	inline Ogre::Camera* getCamera(void) { return this->mCamera; }
	inline OgreBites::SdkTrayManager* getTrayManager(void) { return this->mTrayMgr; }
	void shutdown(void) { this->mShutDown = true; }
	
	void startGame(void);
	void injectMouseMove(float x, float y);

public:	

protected:
	GalaxyEngine::Core* planetEngine;				// planet rendering engine
	GalaxyEngine::Universe* universe;
	GameSystem* gameSystem;							// game play system, put your code inside this class
	GameConfig* gameConfig;	
	Interface* UI;
	KinectUIControl* kinectUIControl;
	CompositorSample* compSample;
	bool isGameStarted;
	bool enableCameraMovement;
	bool isKinectEnabled;

protected:
    virtual void createScene(void);
	virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);

	virtual bool mouseMoved( const OIS::MouseEvent &arg );
	virtual bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
	virtual bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );

	virtual bool keyPressed( const OIS::KeyEvent &arg );
	virtual bool keyReleased( const OIS::KeyEvent &arg );
};

#endif // #ifndef __App_h_
