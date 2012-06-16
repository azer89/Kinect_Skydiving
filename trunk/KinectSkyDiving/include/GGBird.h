#pragma once

#include "Stdafx.h"
#include <Ogre.h>
#include "FSM.h"
#include <vector>

#include "PPSoundManager.h"

//
//
class GGBird
{
public:
	GGBird(Ogre::SceneManager* _mSM, Ogre::Vector3 _bornPos)
	{
		init(_mSM, _bornPos); 
	}

	GGBird(Ogre::SceneManager* _mSM, Ogre::SceneNode* _node)
	{
		init(_mSM, _node); 
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
	Ogre::Vector3 mTarget;	 // the target position if the bird find one
	Ogre::Vector3 mDir;		 // the front vector of the GGBird
	Ogre::Vector3 mPos;		 // the position of the GGBird
	Ogre::Vector3 mSeparate; // avoid to intersect with other GGBird
	float mSize;
};


typedef std::vector<GGBird *> VBirds;

//
//
class GGBirdFatory
{
public:
	GGBirdFatory()
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

	void Update(const float& dt, Ogre::Vector3 posAvatar)
	{
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

					A = 0.0001f;
					B = 0.001;
					n = 1;
					m = 2;
					d = r.length()/(mBirds[i]->mSize * 24);
					U = A/pow(d, n) + B/pow(d, m);

					Fs += (u * U) ;
				}
			}

			// Char
			r = mBirds[i]->mPos - mBirds[i]->mTarget;
			if (r.length() < 100)
			{
				u = r;
				u.normalise();

				A = 0.0001f;
				B = 0.001;
				n = 1;
				m = 2;
				d = r.length()/(mBirds[i]->mSize * 36);
				U = A/pow(d, n) + B/pow(d, m);

				Fs += (u * U) ;
			}

			mBirds[i]->mSeparate = Fs;
			mBirds[i]->Update(dt, posAvatar);
			//printf("Bird[%d](%.2lf, %.2lf, %.2lf)\n", i, mBirds[i]->mPos.x, mBirds[i]->mPos.y, mBirds[i]->mPos.z);
		}

		delBird();
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
};


class GGBirdAttack : public FSMState
{
public:
	GGBird* mGGBird;
	Ogre::AnimationState* mAnim;

	GGBirdAttack(GGBird *_GGBird);
	void DoENTER();
	void Update(const float& dt);
	void DoEXIT();
};
