
#ifndef __CameraListener_h_
#define __CameraListener_h_

#include "Character.h"
#include "ThirdPersonCamera.h"
//#include "ExampleFrameListener.h"

using namespace Ogre;

// Class which holds third-person-camera and current character
class CameraListener
{
public:
	CameraListener(RenderWindow* win, Camera* cam);

	void setCharacter (Character *character);
	void setExtendedCamera (ThirdPersonCamera *cam);
	bool update(Ogre::Real elapsedTime);
	void instantUpdate();

protected:	
	Character *character;				// References to the main character and the camera
	ThirdPersonCamera *mExCamera;
	unsigned int mMode;				// Camera mode
};

#endif

