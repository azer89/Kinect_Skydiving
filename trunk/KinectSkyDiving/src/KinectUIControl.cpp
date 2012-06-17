
#include "Stdafx.h"
#include "KinectUIControl.h"
#include "App.h"

//-------------------------------------------------------------------------------------
KinectUIControl::~KinectUIControl(void)
{
}

//-------------------------------------------------------------------------------------
void KinectUIControl::moveCursor(Ogre::Vector2 pos)
{
	pos.x /= 320.0f;
	pos.y /= 240.0f;
	pos.y += 0.5;
	pos *= 1.3;
	pos.y -= 0.5;

	//printf("U: %f, V: %f\n",pos.x,pos.y);

	if(pos.x >= 0 && pos.x <= 1 && pos.y >= 0 && pos.y <= 1)
	{
		Ogre::OverlayContainer* cursor = main->getTrayManager()->getCursorContainer();
		pos.x = pos.x*main->getCamera()->getViewport()->getActualWidth();
		cursor->setLeft(pos.x);
		pos.y = pos.y*main->getCamera()->getViewport()->getActualHeight();
		cursor->setTop(pos.y);

		main->injectMouseMove(pos.x,pos.y);
	}
}

//-------------------------------------------------------------------------------------
void KinectUIControl::getScreenCoordinates(const Ogre::Vector3& position, Ogre::Real& x, Ogre::Real& y) 
{ 
	Ogre::Vector3 hcsPosition = main->getCamera()->getProjectionMatrix() * (main->getCamera()->getViewMatrix() * position); 
	x = ((hcsPosition.x * 0.5f) + 0.5f);			// 0 <= x <= 1 // left := 0,right := 1
	y = 1.0f - ((hcsPosition.y * 0.5f) + 0.5f);		// 0 <= y <= 1 // bottom := 0,top := 1
}
