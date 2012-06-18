#pragma once

#include "Stdafx.h"
#include <Ogre.h>
#include "FSM.h"
#include <vector>

#include "PPSoundManager.h"
#include "GameConfig.h"

//
//
class GGBird
{
public:
	GGBird(Ogre::SceneManager* _mSM, Ogre::Vector3 _bornPos)
	{
		mAttack = 0;
		initParameters();
		init(_mSM, _bornPos); 
	}

	GGBird(Ogre::SceneManager* _mSM, Ogre::SceneNode* _node)
	{
		mAttack = 0;
		initParameters();
		init(_mSM, _node); 
	}

	void initParameters()
	{
		ggBirdAttackRadius = GameConfig::getSingletonPtr()->getGGBirdAttackRadius();
		ggBirdDeleteRadius = GameConfig::getSingletonPtr()->getGGBirdDeleteRadius();
		ggBirdTracingRadius = GameConfig::getSingletonPtr()->getGGBirdTracingRadius();
	}

	void init(Ogre::SceneManager* mSM, Ogre::Vector3 bornPos);
	void init(Ogre::SceneManager* mSM, Ogre::SceneNode* node);
	void Update(const float& dt, Ogre::Vector3 posAvatar);

public:
	Ogre::Entity* mEnt;
	Ogre::SceneNode* mNode;
	Ogre::SceneManager* mSceneMgr;

	FSM* mState;				// maintain the action of the GGBird 

	bool bDie;
	bool bAttack;
	int mAttack;
	Ogre::Vector3 mTarget;	 // the target position if the bird find one
	Ogre::Vector3 mDir;		 // the front vector of the GGBird
	Ogre::Vector3 mPos;		 // the position of the GGBird
	Ogre::Vector3 mSeparate; // avoid to intersect with other GGBird
	float mSize;

private:
	Ogre::Real ggBirdAttackRadius;
	Ogre::Real ggBirdDeleteRadius;
	Ogre::Real ggBirdTracingRadius;
};


typedef std::vector<GGBird *> VBirds;

//
//
class GGBirdFactory
{
public:
	GGBirdFactory()
	{
		mBirds.clear();
	}

	void addBird(Ogre::SceneManager* mSM, Ogre::Vector3 bornPos)
	{
		printf("Add a GGBird\n");
		mBirds.push_back(new GGBird(mSM, bornPos));
	}

	void addBird(Ogre::SceneManager* mSM, Ogre::SceneNode* node)
	{
		//printf("Add a GGBird\n");
		mBirds.push_back(new GGBird(mSM, node));
	}

	void killAllBirds()
	{
		for(int a = 0; a < mBirds.size(); a++)
		{
			mBirds[a]->bDie = true;
		}
	}

	void delBird()
	{
		for (int i=mBirds.size()-1; i>=0; i--)
		{
			if (mBirds[i]->bDie)
			{
				printf("Kill a GGBird\n");
				mBirds[i]->mSceneMgr->destroySceneNode(mBirds[i]->mNode);
				mBirds.erase(mBirds.begin()+i);
			}
		}
	}

	int Update(const float& dt, Ogre::Vector3 posAvatar)
	{
		int numAttack = 0;

		for (int i=0; i<mBirds.size(); i++)
		{
			Ogre::Vector3 Fs, r, u;
			Ogre::Real U, A, B, n, m, d;

			Fs = Ogre::Vector3::ZERO;

			// Other Bird
			for (int j=0; j<mBirds.size(); j++)
			{
				r = mBirds[i]->mPos - mBirds[j]->mPos;
				if (r == Ogre::Vector3::ZERO) continue;

				if (i!=j && r.length() < 100)
				{
					u = r;
					u.normalise();

					A = 0.00001f;
					B = 0.0001;
					d = r.length()/(mBirds[i]->mSize * 24);
					U = A / d + B /(d * d);

					Fs += (u * U) ;
				}
			}

			// Char
			r = mBirds[i]->mPos - mBirds[i]->mTarget;
			if (r.length() < 100)
			{
				u = r;
				u.normalise();

				A = 0.00001f;
				B = 0.0001;
				d = r.length()/(mBirds[i]->mSize * 36);
				U = A / d + B /(d * d);

				Fs += (u * U) ;
			}

			mBirds[i]->mSeparate = Fs;
			mBirds[i]->Update(dt, posAvatar);
			//printf("Bird[%d](%.2lf, %.2lf, %.2lf)\n", i, mBirds[i]->mPos.x, mBirds[i]->mPos.y, mBirds[i]->mPos.z);

			if (mBirds[i]->bAttack)
			{
				numAttack++;
				mBirds[i]->bAttack = false;
			}
		}

		delBird();

		return numAttack;
	}

	VBirds mBirds;
	
};


//
//
class GGBirdMoving : public FSMState
{
public:

	GGBird* mGGBird;
	Ogre::AnimationState* mAnim;

	float loop;
	int loopDir;

	GGBirdMoving(GGBird *_GGBird);

	void DoENTER();
	void Update(const float& dt);
	void DoEXIT();
};

class GGBirdTracing : public FSMState
{
public:

	GGBird* mGGBird;
	Ogre::AnimationState* mAnim;

	GGBirdTracing(GGBird *_GGBird);
	void DoENTER();
	void Update(const float& dt);
	void DoEXIT();

private:
	Ogre::Real traceSpeed;
};


class GGBirdAttack : public FSMState
{

private:
	//Ogre::Real timeToDie;
	int attackCounter;

public:
	GGBird* mGGBird;
	Ogre::AnimationState* mAnim;

	GGBirdAttack(GGBird *_GGBird);
	void DoENTER();
	void Update(const float& dt);
	void DoEXIT();
};
