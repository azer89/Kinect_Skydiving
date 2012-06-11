

#include "Stdafx.h"
#include "Character.h"

//-------------------------------------------------------------------------------------
Character::Character(void) : 
	state(Movement::NOTHING),
	//rotQ(Ogre::Quaternion::IDENTITY),
	degreeRotation(0.0f),
	gravity(0.0f),
	isLanding(false),
	maxSpeed(GameConfig::getSingletonPtr()->getCharacterMaxSpeed()),
	currentSpeed(0.0f)
{
}

//-------------------------------------------------------------------------------------
Character::~Character(void)
{
}

//-------------------------------------------------------------------------------------
/**  Set up Node, Entity, etc.*/
void Character::setup(Ogre::SceneManager* mSceneManager, 
					  Ogre::Vector3 position, 
					  Ogre::Vector3 scale, 
					  Ogre::Quaternion orientation)
{
	this->mSceneManager = mSceneManager;

	this->entityName = "MainBodyCharacter";

	this->bodyEntity = mSceneManager->createEntity(entityName, "girl.mesh");
	this->mMainNode = mSceneManager->getRootSceneNode()->createChildSceneNode();

	Ogre::SceneNode* innerNode = new Ogre::SceneNode(mSceneManager);
	innerNode->attachObject(bodyEntity);
	Ogre::Quaternion q1 = Ogre::Quaternion::IDENTITY;
	Ogre::Quaternion q2 = Ogre::Quaternion::IDENTITY;
	q1.FromAngleAxis(Ogre::Degree(180), Ogre::Vector3::UNIT_Y);
	//q2.FromAngleAxis(Ogre::Degree(90), Ogre::Vector3::UNIT_X);
	innerNode->setOrientation(q1 * q2);
	this->mMainNode->addChild(innerNode);

	this->mMainNode->setPosition(position);
	this->mMainNode->setScale(scale);
	this->mMainNode->setOrientation(orientation);

	Ogre::Vector3 sightV = GameConfig::getSingletonPtr()->getSightNodePosition();
	Ogre::Vector3 camV = GameConfig::getSingletonPtr()->getCameraNodePosition();

	Ogre::Vector3 orientX = this->mMainNode->getOrientation() * Ogre::Vector3::UNIT_Y;
	Ogre::Vector3 orientY = this->mMainNode->getOrientation() * Ogre::Vector3::UNIT_Y;
	Ogre::Vector3 orientZ = this->mMainNode->getOrientation() * Ogre::Vector3::UNIT_Z;
	Ogre::Vector3 sight =  (orientX * sightV.x) + (orientY * sightV.y) + (orientZ * sightV.z);
	Ogre::Vector3 cam =    (orientX * camV.x) + (orientY * camV.y) + (orientZ * camV.z);

	mSightNode = this->mMainNode->createChildSceneNode("sightNode", sight);
	mCameraNode = this->mMainNode->createChildSceneNode("cameraNode", cam);	

	/* // Ribbon Trail
	Ogre::NameValuePairList params;
	params["numberOfChains"] = "2";
	params["maxElements"] = "20";
	Ogre::RibbonTrail* mTrail = (Ogre::RibbonTrail*)mSceneManager->createMovableObject("RibbonTrail", &params);
	mTrail->setMaterialName("Examples/LightRibbonTrail");
	mTrail->setTrailLength(2.0);
	mSceneManager->getRootSceneNode()->attachObject(mTrail);
	
	for (int i = 0; i < 2; i++)
	{
		mTrail->setInitialColour(i, 0.75, 0.75, 0.75, 0.05);
		mTrail->setColourChange(i, 0.75, 0.75, 0.75, 0.025);
		mTrail->setWidthChange(i, 0.5);
		mTrail->setInitialWidth(i, 0.1);
	}
	
	Ogre::SceneNode* trail01Node = this->mMainNode->createChildSceneNode("Trail01", Ogre::Vector3( 1.6, 2.8, -1.0));
	Ogre::SceneNode* trail02Node = this->mMainNode->createChildSceneNode("Trail02", Ogre::Vector3(-1.6, 2.8, -1.0));

	mTrail->addNode(trail01Node);
	mTrail->addNode(trail02Node);*/
}

//--------------------------------------------------------------------------------------
void Character::update(Ogre::Real elapsedTime)
{
	
	Ogre::Vector3 upVector = this->mMainNode->_getDerivedPosition();
	upVector.normalise();
	Ogre::Quaternion qA;
	qA.FromAngleAxis(Ogre::Degree(degreeRotation), upVector);
	Ogre::Quaternion qB = Ogre::Vector3::UNIT_Y.getRotationTo(upVector);
	this->mMainNode->setOrientation(qA * qB);

	//std::cout << this->mMainNode->_getDerivedPosition() << "\n";

	if(!isLanding)
	{		
		fallDown(elapsedTime);
		moveCharacter(elapsedTime);
	}

	//btVector3 btPos = mCCPhysics->getPosition();
	//std::cout << btPos.x() << " " << btPos.y() << " " << btPos.z() << "\n";
}

//--------------------------------------------------------------------------------------
void Character::setState(Movement m)
{
	this->state = m;
}

//--------------------------------------------------------------------------------------
void Character::fallDown(Ogre::Real elapsedTime)
{
	Ogre::Vector3 upVector = this->mMainNode->_getDerivedPosition();
	upVector.normalise();

	mMainNode->translate(-upVector * gravity * elapsedTime);

	//Ogre::Vector3 moveDirection(-upVector * gravity * elapsedTime);
	//mCCPhysics->setWalkDirection(moveDirection.x, moveDirection.y, moveDirection.z);
}

//--------------------------------------------------------------------------------------
void Character::moveCharacter(Ogre::Real elapsedTime)
{
	
	Ogre::Vector3 direction = Ogre::Vector3::ZERO;

	if(state == Movement::MOVE_BACK)
	{
		direction = Ogre::Vector3::UNIT_Z;
	}
	else if(state == Movement::MOVE_FRONT)
	{
		direction = Ogre::Vector3::NEGATIVE_UNIT_Z;
	}
	else if(state == Movement::MOVE_LEFT)
	{
		direction = Ogre::Vector3::NEGATIVE_UNIT_X;
	}
	else if(state == Movement::MOVE_RIGHT)
	{
		direction = Ogre::Vector3::UNIT_X;
	}
	else if(state == Movement::ROTATE_LEFT)
	{
		degreeRotation += 200 * elapsedTime;
	}
	else if (state == Movement::ROTATE_RIGHT)
	{
		degreeRotation -= 200 * elapsedTime;
	}

	if(direction != Ogre::Vector3::ZERO)
	{
		Ogre::Vector3 trans = direction;
		Ogre::Quaternion q = mMainNode->getOrientation();
		trans = trans * 15 * elapsedTime;
		trans = q * trans;
			
		this->mMainNode->translate(trans);
	}
}

