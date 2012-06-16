

#include "Stdafx.h"
#include "GameConfig.h"

GameConfig *GameConfig::singletonPtr = NULL;

GameConfig::GameConfig(const Ogre::String &SceneName, const Ogre::String &groupName)
{
	TiXmlDocument   *XMLDoc = 0;
	TiXmlElement   *XMLRoot;

	// Strip the path
	Ogre::String basename, path;
	Ogre::StringUtil::splitFilename(SceneName, basename, path);
	Ogre::DataStreamPtr pStream = Ogre::ResourceGroupManager::getSingleton().openResource( basename, groupName );

	Ogre::String data = pStream->getAsString();
	XMLDoc = new TiXmlDocument();
	XMLDoc->Parse( data.c_str() );
	pStream->close();
	pStream.setNull();

	XMLRoot = XMLDoc->RootElement();
	TiXmlElement *pElement = XMLRoot->FirstChildElement("data");

	while(pElement)
	{
		Ogre::String dataName = pElement->Attribute("name");
		TiXmlElement *vElement =  pElement->FirstChildElement("value");

		if(dataName == "characterPosition")
		{
			this->characterPosition = this->parseVector3(vElement);
		}
		else if(dataName == "characterScale")
		{
			this->characterScale = this->parseVector3(vElement);
		}
		else if(dataName == "characterMaxSpeed")
		{
			this->characterMaxSpeed = this->parseReal(vElement);
		}
		else if(dataName == "gravity")
		{
			this->gravity = this->parseReal(vElement);
		}
		else if(dataName == "sightNodePosition")
		{
			this->sightNodePosition = this->parseVector3(vElement);
		}
		else if(dataName == "cameraNodePosition")
		{
			this->cameraNodePosition = this->parseVector3(vElement);
		}
		else if(dataName == "planetObjectScaling")
		{
			this->planetObjectScaling = this->parseVector3(vElement);
		}
		else if(dataName == "planetObjectTranslation")
		{
			this->planetObjectTranslation = this->parseVector3(vElement);
		}
		else if(dataName == "mCameraManSpeed")
		{
			this->mCameraManSpeed = this->parseReal(vElement);
		}
		else if(dataName == "targetPosition")
		{
			this->targetPosition = this->parseVector3(vElement);
		}
		else if(dataName == "circleScale")
		{
			this->circleScale = this->parseVector3(vElement);
		}
		else if(dataName == "villageSceneName")
		{
			this->villageSceneName = vElement->Attribute("s");
		}
		else if(dataName == "circleSceneName")
		{
			this->circleSceneName = vElement->Attribute("s");
		}
		else if(dataName == "ggBirdSceneName")
		{
			this->ggBirdSceneName = vElement->Attribute("s");
		}
		else if(dataName == "cloudHighestElevation")
		{
			this->cloudHighestElevation = this->parseReal(vElement);
		}
		else if(dataName == "characterAcceleration")
		{
			this->characterAcceleration = this->parseReal(vElement);
		}
		else if(dataName == "characterRotationSpeed")
		{
			this->characterRotationSpeed = this->parseReal(vElement);
		}
		else if(dataName == "cameraTightness")
		{
			this->cameraTightness = this->parseVector3(vElement);
		}
		else if(dataName == "ggBirdScale")
		{
			this->ggBirdScale = this->parseVector3(vElement);
		}

		pElement = pElement->NextSiblingElement("data");
	}

	singletonPtr = this;

	//delete pElement;
	//delete XMLRoot;
	delete XMLDoc;
	
}

GameConfig::~GameConfig(void)
{
	singletonPtr = NULL;
}

Ogre::Vector3 GameConfig::parseVector3(TiXmlElement *XMLNode)
{
	return Ogre::Vector3(
		Ogre::StringConverter::parseReal(XMLNode->Attribute("x")),
		Ogre::StringConverter::parseReal(XMLNode->Attribute("y")),
		Ogre::StringConverter::parseReal(XMLNode->Attribute("z"))
		);
}

//Ogre::Quaternion GameConfig::parseQuaternion(TiXmlElement *XMLNode)
//{
//}

Ogre::Real GameConfig::parseReal(TiXmlElement *XMLNode)
{
	return Ogre::StringConverter::parseReal(XMLNode->Attribute("f"));
}
