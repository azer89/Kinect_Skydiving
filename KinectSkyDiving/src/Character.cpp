

#include "Stdafx.h"
#include "Character.h"

//-------------------------------------------------------------------------------------
Character::Character(void) : 
	state(Movement::NOTHING),
	//rotQ(Ogre::Quaternion::IDENTITY),
	degreeRotation(0.0f),
	gravity(0.0f),
	isLanding(false)
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

	Ogre::Vector3 orient01 = this->mMainNode->getOrientation() * Ogre::Vector3::UNIT_Y;
	Ogre::Vector3 orient02 = this->mMainNode->getOrientation() * Ogre::Vector3::UNIT_Z;
	Ogre::Vector3 sight =  orient01 * 15.0f;
	Ogre::Vector3 cam = orient01 * 20.0f + orient02 * 10.0f;

	mSightNode = this->mMainNode->createChildSceneNode("sightNode", sight);
	mCameraNode = this->mMainNode->createChildSceneNode("cameraNode", cam);	
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
		trans = trans * 10 * elapsedTime;
		trans = q * trans;
			
		this->mMainNode->translate(trans);
	}
}

