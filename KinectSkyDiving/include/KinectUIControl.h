
#ifndef _KinectUIControl_h_
#define _KinectUIControl_h_

#include "Stdafx.h"

class App;

class KinectUIControl
{

public:
	KinectUIControl(App* main)
	{
		this->main = main;
	}
	virtual ~KinectUIControl(void);
	App* main;

	void moveCursor(Ogre::Vector2 pos);

protected:
	void getScreenCoordinates(const Ogre::Vector3& position, Ogre::Real& x, Ogre::Real& y);
};

#endif
