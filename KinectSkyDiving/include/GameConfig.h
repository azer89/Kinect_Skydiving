

#ifndef __ConfigDetector_h_
#define __ConfigDetector_h_

#include "Stdafx.h"
#include "tinyxml.h"

class GameConfig
{
public:
	GameConfig(const Ogre::String &SceneName, const Ogre::String &groupName);
	virtual ~GameConfig(void);

	inline static GameConfig &getSingleton(){ return *singletonPtr; }
	inline static GameConfig *getSingletonPtr() { return singletonPtr; }

	inline Ogre::Vector3 getCharacterPosition()       { return characterPosition; }
	inline Ogre::Vector3 getCharacterScale()          { return characterScale; }
	inline Ogre::Real    getCharacterMaxSpeed()       { return characterMaxSpeed; }
	inline Ogre::Real    getGravity()                 { return gravity; }
	inline Ogre::Real    getGravityAcceleration()     { return gravityAcceleration; }
	inline Ogre::Vector3 getSightNodePosition()       { return sightNodePosition; }
	inline Ogre::Vector3 getCameraNodePosition()      { return cameraNodePosition; }
	inline Ogre::Vector3 getSightNodePosition2()      { return sightNodePosition2; }
	inline Ogre::Vector3 getCameraNodePosition2()     { return cameraNodePosition2; }
	inline Ogre::Vector3 getPlanetObjectScaling()     { return planetObjectScaling; }
	inline Ogre::Vector3 getPlanetObjectTranslation() { return planetObjectTranslation; }
	inline Ogre::Real    getMCameraManSpeed()         { return mCameraManSpeed; }
	inline Ogre::Vector3 getTargetPosition()          { return targetPosition; }
	inline Ogre::Vector3 getCircleScale()			  { return circleScale; }
	inline Ogre::String  getVillageSceneName()        { return villageSceneName; }
	inline Ogre::String  getCircleSceneName()		  { return circleSceneName; }
	inline Ogre::String  getGGBirdSceneName()		  { return ggBirdSceneName; }
	inline Ogre::Real    getCloudHighestElevation()   { return cloudHighestElevation; }
	inline Ogre::Real    getCharacterAcceleration()   { return characterAcceleration; }
	inline Ogre::Real    getCharacterRotationSpeed()  { return characterRotationSpeed; }
	inline Ogre::Vector3 getCameraTightness()		  { return cameraTightness; }
	inline Ogre::Vector3 getGGBirdScale()			  { return ggBirdScale; }
	inline Ogre::Real    getGGBirdTraceSpeed()		  { return ggBirdTraceSpeed; }
	inline Ogre::Real getOpenParachuteDelay()		  { return openParachuteDelay; }
		
	bool getKinectStatus(void)
	{
		if(kinectActivation <= 0) return false;
		return true;
	}

	bool isMainMenuEnabled(void)
	{
		if(enableMainMenu <= 0) return false;
		return true;
	}

	bool isCameraMovementEnabled(void)
	{
		if(enableCameraMovement <= 0) return false;
		return true;
	}

private:
	Ogre::Vector3 parseVector3(TiXmlElement *XMLNode);
	//Ogre::Quaternion parseQuaternion(TiXmlElement *XMLNode);
	Ogre::Real parseReal(TiXmlElement *XMLNode);

private:
	static GameConfig *singletonPtr;

	Ogre::Vector3 characterPosition;
	Ogre::Vector3 characterScale;
	Ogre::Real characterMaxSpeed;
	Ogre::Real gravity;
	Ogre::Real gravityAcceleration;
	Ogre::Vector3 sightNodePosition;
	Ogre::Vector3 cameraNodePosition;
	Ogre::Vector3 sightNodePosition2;
	Ogre::Vector3 cameraNodePosition2;
	Ogre::Vector3 planetObjectScaling;
	Ogre::Vector3 planetObjectTranslation;
	Ogre::Real mCameraManSpeed;
	Ogre::Vector3 targetPosition;
	Ogre::Vector3 circleScale;
	Ogre::String villageSceneName;
	Ogre::String circleSceneName;
	Ogre::String ggBirdSceneName;
	Ogre::Real cloudHighestElevation;
	Ogre::Real characterAcceleration;
	Ogre::Real characterRotationSpeed;
	Ogre::Vector3 cameraTightness;
	Ogre::Vector3 ggBirdScale;
	Ogre::Real ggBirdTraceSpeed;
	int kinectActivation;
	int enableMainMenu;
	int enableCameraMovement;
	Ogre::Real openParachuteDelay;

};

#endif // #ifndef __ConfigDetector_h_

