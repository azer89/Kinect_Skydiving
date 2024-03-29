
#include "Stdafx.h"
#include "GGBird.h"

//
//  GGBird
//
void GGBird::Update( const float& dt, Ogre::Vector3 posAvatar )
{
	mPos = mNode->getPosition();
	mTarget = posAvatar;
	float disWithAvatar = (posAvatar - mPos).length();

	if(disWithAvatar < ggBirdAttackRadius)
	{
		mAttack++;
		mState->transitionTo("attack");
	}
	else if(disWithAvatar > ggBirdDeleteRadius && mAttack >= 2)
	{
		bDie = true;
	}
	else if(disWithAvatar < ggBirdTracingRadius)
	{
		mState->transitionTo("tracing");
	}
	else
	{
		mState->transitionTo("fly");
	}
	
	if (mState->stateBank.size() > 0) mState->currentState->Update(dt);
}

void GGBird::init(Ogre::SceneManager* mSM, Ogre::SceneNode* node)
{
	mSceneMgr = mSM;
	mNode = node;
	mEnt = static_cast<Ogre::Entity*>(node->getAttachedObject(0));

	bDie = false;
	bAttack = false;
	mTarget = Ogre::Vector3::ZERO;
	mDir = Ogre::Vector3(0,0,1);
	mPos = node->getPosition();
	mSeparate = Ogre::Vector3::ZERO;

	mSize = 0.3f;
	mNode->setScale(mSize, mSize, mSize);

	mState = new FSM();
	GGBirdMoving* state1 = new GGBirdMoving(this);
	mState->AddState(state1, true);
	GGBirdAttack* state2 = new GGBirdAttack(this);
	mState->AddState(state2, false);
	GGBirdTracing* state3 = new GGBirdTracing(this);
	mState->AddState(state3, false);
}

void GGBird::init(Ogre::SceneManager* mSM, Ogre::Vector3 bornPos)
{
	mSceneMgr = mSM;
	mEnt = mSceneMgr->createEntity("GGBird.mesh");
	mNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	mNode->setPosition(bornPos);
	mNode->attachObject(mEnt);

	mSize = 0.3f;
	mNode->setScale(mSize, mSize, mSize);

	bDie = false;
	bAttack = false;
	mTarget = Ogre::Vector3::ZERO;
	mDir = Ogre::Vector3(0,0,1);
	mPos = bornPos;

	mState = new FSM();
	GGBirdMoving* state1 = new GGBirdMoving(this);
	mState->AddState(state1, true);
	GGBirdAttack* state2 = new GGBirdAttack(this);
	mState->AddState(state2, false);
	GGBirdTracing* state3 = new GGBirdTracing(this);
	mState->AddState(state3, false);
}



//
//  State: Moving
//
GGBirdMoving::GGBirdMoving(GGBird *_GGBird)
{
	mGGBird = _GGBird;
	stateName = "fly";
	mAnim = mGGBird->mEnt->getAnimationState(stateName);

	loop = 0.0f;
	loopDir = -2;
}

void GGBirdMoving::DoENTER()
{
	mAnim->setTimePosition(0.0f);
	mAnim->setLoop(true);
	mAnim->setEnabled(true);
}

void GGBirdMoving::Update(const float& dt)
{
	mAnim->addTime(dt);

	float rr = (rand() % 100) / 100.0f; // 0~1
	mGGBird->mNode->yaw(Ogre::Radian(rr * dt));

	/*
	Ogre::Real mSpeed = 0.1f;

	if (loopDir == -2)
	{
		loopDir = (rand() % 2 == 0)? 1: -1; 
	}

	loop += 0.001 * mSpeed;
	if (loop > Ogre::Math::TWO_PI) loop = 0;

	Ogre::Vector3 vPos = mGGBird->mNode->getPosition();
	Ogre::Vector3 newV ( vPos.x + loopDir * Ogre::Math::Cos(loop) * 1.0f , 
						 vPos.y + loopDir * Ogre::Math::Sin(loop) * 0.01f, 
						 vPos.z - loopDir * Ogre::Math::Sin(loop) * 1.0f );

	newV += mGGBird->mSeparate* mSpeed;

	Ogre::Vector3 front = newV - vPos;
	Ogre::Vector3 up = Ogre::Vector3(0, 1, 0);
	Ogre::Vector3 left = up.crossProduct(front);
	front.normalise();
	up.normalise();
	left.normalise();
	Ogre::Quaternion quat(left, up, front);
	mGGBird->mNode->setOrientation(quat);
	mGGBird->mNode->setPosition(newV);*/

}

void GGBirdMoving::DoEXIT()
{
	mAnim->setEnabled(false);
}


//
// State : Tracing
//

GGBirdTracing::GGBirdTracing(GGBird *_GGBird) : traceSpeed(GameConfig::getSingletonPtr()->getGGBirdTraceSpeed())
{
	mGGBird = _GGBird;
	stateName = "tracing";
	mAnim = mGGBird->mEnt->getAnimationState("fly");
}

void GGBirdTracing::DoENTER()
{
	mAnim->setTimePosition(0.0f);
	mAnim->setLoop(true);
	mAnim->setEnabled(true);

	//mPPSoundManager->playEffect("1.wav"); ///
}

void GGBirdTracing::Update(const float& dt)
{
	mAnim->addTime(dt);

	Ogre::Real mSpeed = traceSpeed * dt; //5.0f * dt;

	Ogre::Vector3 vPos = mGGBird->mNode->getPosition();
	Ogre::Vector3 vDir = (mGGBird->mTarget - vPos).normalisedCopy();
	Ogre::Vector3 newV = vPos + vDir * mSpeed + mGGBird->mSeparate * mSpeed * 2;

	Ogre::Vector3 front = newV - vPos;
	Ogre::Vector3 up = Ogre::Vector3(0, 1, 0);
	Ogre::Vector3 left = up.crossProduct(front);
	front.normalise();
	up.normalise();
	left.normalise();
	Ogre::Quaternion quat(left, up, front);
	mGGBird->mNode->setOrientation(quat);
	mGGBird->mNode->setPosition(newV);
}

void GGBirdTracing::DoEXIT()
{
	mAnim->setEnabled(false);
}

//
// State - Attack 
//
GGBirdAttack::GGBirdAttack(GGBird *_GGBird)
{
	//timeToDie = 0.0f;
	attackCounter = 0;
	mGGBird = _GGBird;
	stateName = "attack";
	mAnim = mGGBird->mEnt->getAnimationState(stateName);
}

void GGBirdAttack::DoENTER()
{
	mAnim->setTimePosition(0.0f);
	mAnim->setLoop(false);
	mAnim->setEnabled(true);

	mGGBird->bAttack = true;

	//mPPSoundManager->playEffect("2.wav"); ///
}

void GGBirdAttack::Update(const float& dt)
{
	Ogre::Real addTime = Ogre::Math::Clamp<Ogre::Real>(mAnim->getTimePosition() + dt, 0, mAnim->getLength());

	mAnim->addTime(addTime);

	//if (mAnim->getLength() == mAnim->getTimePosition())
	//{
		//attackCounter++;
		//mGGBird->bDie = true;
	//}
}

void GGBirdAttack::DoEXIT()
{
	mAnim->setEnabled(false);
}
