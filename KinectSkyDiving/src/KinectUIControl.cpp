
#include "Stdafx.h"
#include "KinectUIControl.h"

//-------------------------------------------------------------------------------------
KinectUiControl::~KinectUiControl(void)
{
}

//-------------------------------------------------------------------------------------
void KinectUiControl::MoveCursor(Ogre::Vector3 pos)
{
	float x, y;
	getScreenCoordinates(pos.reflect(Ogre::Vector3(1, 0, 0)) * 2000, x, y);
	y += 0.3;
	y *= 2.2;
	x -= 0.5;
	x *= 2.2;
	x += 0.5;
	if(x >= 0 && x <= 1 && y >= 0 && y <= 1)
	{
		Ogre::OverlayContainer* cursor = main->getTrayManager()->getCursorContainer();
		x = x * main->getCamera()->getViewport()->getActualWidth();
		cursor->setLeft(x);
		y = y * main->getCamera()->getViewport()->getActualHeight();
		cursor->setTop(y);

		main->injectMouseMove(x, y);
	}

}

//-------------------------------------------------------------------------------------
void KinectUiControl::getScreenCoordinates(const Ogre::Vector3& position, Ogre::Real& x, Ogre::Real& y) 
{ 
	Ogre::Vector3 hcsPosition = main->getCamera()->getProjectionMatrix() * (main->getCamera()->getViewMatrix() * position); 
	x = ((hcsPosition.x * 0.5f) + 0.5f);			// 0 <= x <= 1 // left := 0,right := 1
	y = 1.0f - ((hcsPosition.y * 0.5f) + 0.5f);		// 0 <= y <= 1 // bottom := 0,top := 1
}
