
#ifndef _KinectUIControl_h_
#define _KinectUIControl_h_

#include "Stdafx.h"
#include "App.h"

class KinectUiControl
{

public:
	KinectUiControl(App* main)
	{
		this->main = main;
	}
	virtual ~KinectUiControl(void);
	App* main;

	void MoveCursor(Ogre::Vector3 pos);

protected:
	void getScreenCoordinates(const Ogre::Vector3& position, Ogre::Real& x, Ogre::Real& y);
};

#endif
