
#include "Stdafx.h"
#include "CameraListener.h"

//------------------------------------------------------------------------------------- 
CameraListener::CameraListener(RenderWindow* win, Camera* cam)
	: character(0),
	  mExCamera(0),
	  mMode(0)
{	
}

//------------------------------------------------------------------------------------- 
void CameraListener::setCharacter (Character *character) { this->character = character; }

//------------------------------------------------------------------------------------- 
void CameraListener::setExtendedCamera (ThirdPersonCamera *cam) { mExCamera = cam; }

//------------------------------------------------------------------------------------- 
void CameraListener::instantUpdate()
{
	if (mExCamera && character) 
		mExCamera->instantUpdate (character->getCameraNode()->_getDerivedPosition(), 
		                          character->getSightNode()->_getDerivedPosition());
}

//------------------------------------------------------------------------------------- 
bool CameraListener::update(Ogre::Real elapsedTime)
{
	//mKeyboard->capture();
	if (character) 
	{
		//mChar->update (evt.timeSinceLastFrame, mKeyboard);
		if (mExCamera) 
		{
			if(mMode == 0) // 3rd person chase
			{
				
				mExCamera->update (elapsedTime, 
								   character->getCameraNode()->_getDerivedPosition(), 
								   character->getSightNode()->_getDerivedPosition(),
								   character->getBodyNode()->getOrientation());				
			}
			else if(mMode == 1) // 3rd person fixed
			{
				mExCamera->update (elapsedTime, 
								   Vector3 (0, 200, 0), 
								   character->getSightNode()->_getDerivedPosition(),
								   character->getBodyNode()->getOrientation());
			}
			else if(mMode == 2) // 1st person
			{
				mExCamera->update (elapsedTime, 
								   character->getWorldPosition(), 
								   character->getSightNode()->_getDerivedPosition(),
								   character->getBodyNode()->getOrientation());

			}
		}
	}

	/*
	// 3rd Person - Chase Camera
	if (mKeyboard->isKeyDown (OIS::KC_F1)) 
	{
		mMode = 0;
		if (mChar)
			//mChar->setVisible(true);
		if (mExCamera) {
			if (mChar)
				mExCamera->instantUpdate (mChar->getCameraNode ()->_getDerivedPosition(), mChar->getSightNode ()->_getDerivedPosition());
			mExCamera->setTightness (Ogre::Vector3(0.01f));
		}
	}
	// 3rd Person - Fixed Camera
	if (mKeyboard->isKeyDown (OIS::KC_F2)) {
		mMode = 1;
		if (mChar)
			//mChar->setVisible(true);
		if (mExCamera) {
			if (mChar)
				mExCamera->instantUpdate (Vector3 (0, 200, 0), mChar->getSightNode ()->_getDerivedPosition());
			mExCamera->setTightness (Ogre::Vector3(0.01f));
		}
	}
	// 1st Person
	if (mKeyboard->isKeyDown (OIS::KC_F3))  {
		mMode = 2;
		if (mChar)
			//mChar->setVisible(false);
		if (mExCamera) {
			if (mChar)
				mExCamera->instantUpdate (mChar->getWorldPosition (), mChar->getSightNode ()->_getDerivedPosition());
			mExCamera->setTightness (Ogre::Vector3(0.01f));
		}
	}
	*/

	// Exit if we press ESC
	//if(mKeyboard->isKeyDown (OIS::KC_ESCAPE)) return false;

	return true;
}


