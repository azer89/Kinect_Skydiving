
/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2011 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
-----------------------------------------------------------------------------
*/
/*
-----------------------------------------------------------------------------
Filename:    ExampleLoadingBar.h
Description: Defines an example loading progress bar which you can use during 
startup, level changes etc to display loading progress. 
IMPORTANT: Note that this progress bar relies on you having the OgreCore.zip 
package already added to a resource group called 'Bootstrap' - this provides
the basic resources required for the progress bar and will be loaded automatically.
*/
#include "OgreResourceGroupManager.h"
#include "OgreException.h"
#include "OgreOverlay.h"
#include "OgreOverlayManager.h"
#include "OgreRenderWindow.h"
#pragma once
#include "Hikari.h"
//using namespace Ogre;

/** Defines an example loading progress bar which you can use during 
	startup, level changes etc to display loading progress. 
@remarks
	Basically you just need to create an instance of this class, call start()
	before loading and finish() afterwards. You may also need to stop areas of
	your scene rendering in between since this method will call 
	RenderWindow::update() to update the display of the bar - we advise using
	SceneManager's 'special case render queues' for this, see
	SceneManager::addSpecialCaseRenderQueue for details.
@note 
	This progress bar relies on you having the OgreCore.zip package already 
	added to a resource group called 'Bootstrap' - this provides the basic 
	resources required for the progress bar and will be loaded automatically.
*/
class ExampleLoadingBar : public Ogre::ResourceGroupListener
{
protected:
	Ogre::RenderWindow* mWindow;
	Ogre::Overlay* mLoadOverlay;
	Ogre::Real mInitProportion;
	unsigned short mNumGroupsInit;
	unsigned short mNumGroupsLoad;
	Ogre::Real mProgressBarMaxSize;
	Ogre::Real mProgressBarScriptSize;
	Ogre::Real mProgressBarInc;
	Ogre::OverlayElement* mLoadingBarElement;
	Ogre::OverlayElement* mLoadingDescriptionElement;
	Ogre::OverlayElement* mLoadingCommentElement;
	int frame;

	

public:
	Hikari::HikariManager* mHikariMgr;
	Hikari::FlashControl* screen;
	ExampleLoadingBar() {}
	virtual ~ExampleLoadingBar(){}

	/** Show the loading bar and start listening.
	@param window The window to update 
	@param numGroupsInit The number of groups you're going to be initialising
	@param numGroupsLoad The number of groups you're going to be loading
	@param initProportion The proportion of the progress which will be taken
		up by initialisation (ie script parsing etc). Defaults to 0.7 since
		script parsing can often take the majority of the time.
	*/
	virtual void start(Ogre::RenderWindow* window, 
		unsigned short numGroupsInit = 1, 
		unsigned short numGroupsLoad = 1, 
		Ogre::Real initProportion = 0.70f)
	{
		mWindow = window;
		frame = 0;
		mNumGroupsInit = numGroupsInit;
		mNumGroupsLoad = numGroupsLoad;
		mInitProportion = initProportion;
		// We need to pre-initialise the 'Bootstrap' group so we can use
		// the basic contents in the loading screen

		using namespace Hikari;
		mHikariMgr = new HikariManager("..\\..\\media\\UI");
		screen = mHikariMgr->createFlashOverlay("UI",mWindow->getViewport(0),  mWindow->getViewport(0)->getActualWidth(), mWindow->getViewport(0)->getActualHeight(), Position(TopLeft));
		screen->load("loader.swf");
		screen->setDraggable(false);
		screen->setTransparent(true, true);
		mHikariMgr->update();

		//Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("Bootstrap");

		//Ogre::OverlayManager& omgr = Ogre::OverlayManager::getSingleton();
		//mLoadOverlay = (Ogre::Overlay*)omgr.getByName("Core/LoadOverlay");
		//if (!mLoadOverlay)
		//{
		//	OGRE_EXCEPT(Ogre::Exception::ERR_ITEM_NOT_FOUND, 
		//		"Cannot find loading overlay", "ExampleLoadingBar::start");
		//}
		//mLoadOverlay->show();

		// Save links to the bar and to the loading text, for updates as we go
		//mLoadingBarElement = omgr.getOverlayElement("Core/LoadPanel/Bar/Progress");
		//mLoadingCommentElement = omgr.getOverlayElement("Core/LoadPanel/Comment");
		//mLoadingDescriptionElement = omgr.getOverlayElement("Core/LoadPanel/Description");

		//Ogre::OverlayElement* barContainer = omgr.getOverlayElement("Core/LoadPanel/Bar");
		//mProgressBarMaxSize = barContainer->getWidth();
		//mLoadingBarElement->setWidth(0);

		// self is listener
		Ogre::ResourceGroupManager::getSingleton().addResourceGroupListener(this);



	}

	/** Hide the loading bar and stop listening. 
	*/
	virtual void finish(void)
	{
		// hide loading screen
		//mLoadOverlay->hide();
		screen->hide();
		delete(mHikariMgr);
		mHikariMgr = NULL;
		// Unregister listener
		Ogre::ResourceGroupManager::getSingleton().removeResourceGroupListener(this);

	}


	// ResourceGroupListener callbacks
	void resourceGroupScriptingStarted(const Ogre::String& groupName, size_t scriptCount)
	{
		update();
	}
	void scriptParseStarted(const Ogre::String& scriptName, bool &skipThisScript)
	{
		update();
	}
	void scriptParseEnded(const Ogre::String& scriptName, bool skipped)
	{
		update();
	}
	void resourceGroupScriptingEnded(const Ogre::String& groupName)
	{
		update();
	}
	void resourceGroupLoadStarted(const Ogre::String& groupName, size_t resourceCount)
	{
		update();
	}
	void resourceLoadStarted(const Ogre::ResourcePtr& resource)
	{
		update();
	}
	void resourceLoadEnded(void)
	{
		update();
	}
	void worldGeometryStageStarted(const Ogre::String& description)
	{
		update();
	}
	void worldGeometryStageEnded(void)
	{
		update();
	}
	void resourceGroupLoadEnded(const Ogre::String& groupName)
	{
		update();
	}

	Ogre::DataStreamPtr resourceLoading(const Ogre::String &name, const Ogre::String &group, Ogre::Resource *resource)
	{
		update();
		return Ogre::DataStreamPtr();
	}

	bool resourceCollision(Ogre::ResourcePtr &resource, Ogre::ResourceManager *resourceManager)
	{
		return false;
	}
	void update()
	{
		screen->callFunction("update");
		frame++;
		screen->gotoFrame(frame);
		mWindow->update();
		mHikariMgr->update();
	}

};


