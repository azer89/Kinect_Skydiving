
#include "Stdafx.h"
#include "Character.h"

const std::string Character::animNames01[] = 
	{"back",
	 "front",
	 "left",
	 "right",
	 };

const std::string Character::animNames02[] = 
	{"after_landing_wait",
	 "fly_with_parachute",
	 "land",
	 "open",
	 };

#define GIRL				Ogre::String("girl.mesh")
#define GIRL_PARACHUTE		Ogre::String("girl_with_parachute.mesh")
#define ANIM_FADE_SPEED		5.0f

//-------------------------------------------------------------------------------------
Character::Character(void) : 
	state(Movement::NOTHING),
	previousState(Movement::NOTHING),
	degreeRotation(0.0f),
	gravity(0.0f),
	isLanding(false),
	isParachuteOpen(false),
	maxSpeed(GameConfig::getSingletonPtr()->getCharacterMaxSpeed()),
	acceleration(GameConfig::getSingletonPtr()->getCharacterAcceleration()),
	rotationSpeed(GameConfig::getSingletonPtr()->getCharacterRotationSpeed()),
	baseAnimID01(AnimID01::NO_ANIM1),
	baseAnimID02(AnimID02::NO_ANIM2),
	prevAnimID01(AnimID01::NO_ANIM1),
	prevAnimID02(AnimID02::NO_ANIM2)
{
	for(int a = 0; a < 4 ; a++)
		currentSpeed[a] = 0.0f;
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

	this->bodyEntity01 = mSceneManager->createEntity(GIRL, GIRL);
	this->bodyEntity02 = mSceneManager->createEntity(GIRL_PARACHUTE, GIRL_PARACHUTE);

	this->mMainNode = mSceneManager->getRootSceneNode()->createChildSceneNode();

	innerNode = new Ogre::SceneNode(mSceneManager);
	innerNode->attachObject(bodyEntity01);
	Ogre::Quaternion q1 = Ogre::Quaternion::IDENTITY;
	Ogre::Quaternion q2 = Ogre::Quaternion::IDENTITY;
	q1.FromAngleAxis(Ogre::Degree(180), Ogre::Vector3::UNIT_Y);
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

	for (int i = 0; i < NUM_ANIM; i++)
	{
		mAnims01[i] = this->bodyEntity01->getAnimationState(this->animNames01[i]);
		mFadingIn01[i] = false;
		mFadingOut01[i] = false;
		mAnims01[i]->setLoop(true);

		mAnims02[i] = this->bodyEntity02->getAnimationState(this->animNames02[i]);
		mFadingIn02[i] = false;
		mFadingOut02[i] = false;
		mAnims02[i]->setLoop(true);
	}

	setAnimation(AnimID01::BACK);

	//this->mMainNode->showBoundingBox(true);

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
void Character::setLanding() 
{ 
	if(!isLanding)
	{		
		isLanding = true; 	
		if(isParachuteOpen) setParachuteAnimation(AnimID02::LAND, true);
	}
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

	for(int a = 0; a < 4; a++)
	{
		if(this->state == Movement::NOTHING && currentSpeed[a] > 0)
		{
			currentSpeed[a] -= acceleration;
			if(currentSpeed[a] < 0.0f) currentSpeed[a] = 0.0f;
		}
		else if(this->state == static_cast<Movement>(a) && currentSpeed[a] < maxSpeed)
		{
			currentSpeed[a] += acceleration;
			if(currentSpeed[a] > maxSpeed) currentSpeed[a] = maxSpeed;
		}
	}

	if(!isLanding)
	{		
		fallDown(elapsedTime);
		moveCharacter(elapsedTime);
	}

	updateAnimations(elapsedTime);
}

//--------------------------------------------------------------------------------------
void Character::fadeAnimations(Ogre::Real deltaTime)
{
	for (int i = 0; i < NUM_ANIM; i++)
	{
		if(!isParachuteOpen)		// free fall
		{
			if (mFadingIn01[i])
			{
				// slowly fade this animation in until it has full weight
				Ogre::Real newWeight = mAnims01[i]->getWeight() + deltaTime * ANIM_FADE_SPEED;
				mAnims01[i]->setWeight(Ogre::Math::Clamp<Ogre::Real>(newWeight, 0, 1));
				if (newWeight >= 1)
				{
					mFadingIn01[i] = false;
					mAnims01[i]->setWeight(1);
				}
			}
			else if (mFadingOut01[i])
			{
				// slowly fade this animation out until it has no weight, and then disable it
				Ogre::Real newWeight = mAnims01[i]->getWeight() - deltaTime * ANIM_FADE_SPEED;
				mAnims01[i]->setWeight(Ogre::Math::Clamp<Ogre::Real>(newWeight, 0, 1));
				if (newWeight <= 0)
				{
					mAnims01[i]->setWeight(0);
					mAnims01[i]->setEnabled(false);					
					mFadingOut01[i] = false;
				}
			}
		}
		else	// parachute opened
		{
			if (mFadingIn02[i])
			{
				// slowly fade this animation in until it has full weight
				Ogre::Real newWeight = mAnims02[i]->getWeight() + deltaTime * ANIM_FADE_SPEED;
				mAnims02[i]->setWeight(Ogre::Math::Clamp<Ogre::Real>(newWeight, 0, 1));
				if (newWeight >= 1)
				{
					mFadingIn02[i] = false;
					mAnims02[i]->setWeight(1);
				}
			}
			else if (mFadingOut02[i])
			{
				// slowly fade this animation out until it has no weight, and then disable it
				Ogre::Real newWeight = mAnims02[i]->getWeight() - deltaTime * ANIM_FADE_SPEED;
				mAnims02[i]->setWeight(Ogre::Math::Clamp<Ogre::Real>(newWeight, 0, 1));
				if (newWeight <= 0)
				{
					mAnims02[i]->setWeight(0);
					mAnims02[i]->setEnabled(false);
					mFadingOut02[i] = false;
				}
			}
		}
	}
}

//--------------------------------------------------------------------------------------
void Character::setAnimation(AnimID01 id, bool noTransition)
{
	if(id == baseAnimID01) return;	

	prevAnimID01 = baseAnimID01;
	baseAnimID01 = id;
	mTimer = 0.0f;

	mAnims01[baseAnimID01]->setEnabled(true);
	mAnims01[baseAnimID01]->setWeight(1);	

	if(noTransition)
	{
		mAnims01[prevAnimID01]->setEnabled(false);
		mAnims01[prevAnimID01]->setWeight(0);
	}
	else if(prevAnimID01 != AnimID01::NO_ANIM1)	// transition
	{
		mFadingIn01[prevAnimID01] = false;
		mFadingOut01[prevAnimID01] = true;

		mAnims01[baseAnimID01]->setWeight(0);
		mFadingOut01[baseAnimID01] = false;
		mFadingIn01[baseAnimID01] = true;
	}
}

//--------------------------------------------------------------------------------------
void Character::setParachuteAnimation(AnimID02 id, bool noTransition)
{
	if(id == baseAnimID02) return;	

	prevAnimID02 = baseAnimID02;
	baseAnimID02 = id;
	mTimer = 0.0f;
	
	mAnims02[baseAnimID02]->setEnabled(true);
	mAnims02[baseAnimID02]->setWeight(1);
	
	if(noTransition)
	{
		mAnims02[prevAnimID02]->setEnabled(false);
		mAnims02[prevAnimID02]->setWeight(0);
	}
	else if(prevAnimID02 != AnimID02::NO_ANIM2)	// transition
	{
		mFadingIn02[prevAnimID02] = false;
		mFadingOut02[prevAnimID02] = true;

		mAnims02[baseAnimID02]->setWeight(0);
		mFadingOut02[baseAnimID02] = false;
		mFadingIn02[baseAnimID02] = true;
	}
}


//--------------------------------------------------------------------------------------
void Character::updateAnimations(Ogre::Real deltaTime)
{
	mTimer += deltaTime;

	if(isParachuteOpen)	// parachute opened
	{
		if(baseAnimID02 == AnimID02::OPEN && mTimer >= mAnims02[AnimID02::OPEN]->getLength())
		{
			setParachuteAnimation(AnimID02::FLY_WITH_PARACHUTE, true);
		}
		else if(baseAnimID02 == AnimID02::LAND && mTimer >= mAnims02[AnimID02::LAND]->getLength())
		{
			setParachuteAnimation(AnimID02::AFTER_LANDING_WAIT, true);
		}

		mAnims02[baseAnimID02]->addTime(deltaTime);
	}
	else	// free fall
	{
		mAnims01[baseAnimID01]->addTime(deltaTime);
	}

	fadeAnimations(deltaTime);
}



//--------------------------------------------------------------------------------------
void Character::openParachute()
{
	if(isParachuteOpen) return;

	this->innerNode->detachObject(GIRL);
	this->innerNode->attachObject(this->bodyEntity02);
	this->gravity *= 0.5f;
	isParachuteOpen = true;
	setParachuteAnimation(AnimID02::OPEN);
}

//--------------------------------------------------------------------------------------
void Character::setState(Movement m)
{	
	this->previousState = this->state;
	this->state = m;

	if(isParachuteOpen && baseAnimID02 != AnimID02::OPEN)
	{
		setParachuteAnimation(AnimID02::FLY_WITH_PARACHUTE);
	}
	else
	{
		if(this->state == Movement::NOTHING)
		{
			setAnimation(AnimID01::BACK);
		}
		else if(this->state == Movement::MOVE_BACK)
		{
			setAnimation(AnimID01::BACK);
		}
		else if(this->state == Movement::MOVE_FRONT)
		{
			setAnimation(AnimID01::FRONT);
		}
		else if(this->state == Movement::MOVE_LEFT)
		{
			setAnimation(AnimID01::FRONT);
		}
		else if(this->state == Movement::MOVE_RIGHT)
		{
			setAnimation(AnimID01::FRONT);
		}
		else if(this->state == Movement::ROTATE_LEFT)
		{
			setAnimation(AnimID01::LEFT);
		}
		else if(this->state == Movement::ROTATE_RIGHT)
		{
			setAnimation(AnimID01::RIGHT);
		}
	}
	
}

//--------------------------------------------------------------------------------------
void Character::fallDown(Ogre::Real elapsedTime)
{
	Ogre::Vector3 upVector = this->mMainNode->_getDerivedPosition();
	upVector.normalise();

	mMainNode->translate(-upVector * gravity * elapsedTime);
}

//--------------------------------------------------------------------------------------
void Character::moveCharacter(Ogre::Real elapsedTime)
{
	
	Ogre::Vector3 direction = Ogre::Vector3::ZERO;

	Movement mState = this->state;
	Ogre::Real mSpeed = this->currentSpeed[static_cast<int>(this->state)];

	if(this->state == Movement::NOTHING && 
		previousState != Movement::ROTATE_LEFT &&
		previousState != Movement::ROTATE_RIGHT) 
	{
		mState = this->previousState;
		mSpeed = this->currentSpeed[static_cast<int>(mState)];
	}

	if(mState == Movement::MOVE_BACK)
	{
		direction = Ogre::Vector3::UNIT_Z;
	}
	else if(mState == Movement::MOVE_FRONT)
	{
		direction = Ogre::Vector3::NEGATIVE_UNIT_Z;
	}
	else if(mState == Movement::MOVE_LEFT)
	{
		direction = Ogre::Vector3::NEGATIVE_UNIT_X;
	}
	else if(mState == Movement::MOVE_RIGHT)
	{
		direction = Ogre::Vector3::UNIT_X;
	}
	else if(mState == Movement::ROTATE_LEFT)
	{
		degreeRotation += this->rotationSpeed * elapsedTime;
	}
	else if (mState == Movement::ROTATE_RIGHT)
	{
		degreeRotation -= this->rotationSpeed * elapsedTime;
	}

	if(direction != Ogre::Vector3::ZERO)
	{
		Ogre::Vector3 trans = direction;
		Ogre::Quaternion q = mMainNode->getOrientation();
		trans = trans * mSpeed * elapsedTime;
		trans = q * trans;
			
		this->mMainNode->translate(trans);
	}
}

