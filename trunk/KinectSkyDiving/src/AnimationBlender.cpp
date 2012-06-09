

#include "Stdafx.h"
#include "AnimationBlender.h"

void AnimationBlender::init(bool l,float fadeSpeed)
{
	FILE* bFile;
	bFile = fopen("names.txt","w");
	mEntity->getSkeleton()->setBlendMode(Ogre::ANIMBLEND_CUMULATIVE);
	Ogre::AnimationStateSet *set = mEntity->getAllAnimationStates();
	Ogre::AnimationStateIterator it = set->getAnimationStateIterator();
	while(it.hasMoreElements())
	{
		Ogre::AnimationState *anim = it.getNext();
		fprintf(bFile,"%s\n",anim->getAnimationName().c_str());
		anim->setEnabled(false);
		anim->setWeight(0);
		anim->setTimePosition(0);
		mAnims.push_back(anim);
		mFadingIn.push_back(false);
		mFadingOut.push_back(false);
		mAnimWeights.push_back(0.0f);

	}
	fclose(bFile);
	//mSource = mEntity->getAnimationState( animation );
	//mSource->setEnabled(true);
	//mSource->setWeight(1);
	mTimeleft = 0;
	mDuration = 1;
	mTarget = 0;
	complete = false;
	loop = l;
	isControllable = true;
	ANIM_FADE_SPEED = fadeSpeed;
} 
void AnimationBlender::blend( const Ogre::String &animation, BlendingTransition transition, Ogre::Real duration, bool l )
{
	loop = l;
	if( transition == AnimationBlender::BlendSwitch )
	{
		if( mSource != 0 )
			mSource->setEnabled(false);
		mSource = mEntity->getAnimationState( animation );
		mSource->setEnabled(true);
		mSource->setWeight(1);
		mSource->setTimePosition(0);
		mTimeleft = 0;
	} 
	else if( transition == AnimationBlender::BlendState )
	{
		Ogre::AnimationState *newTarget = mEntity->getAnimationState( animation );
		newTarget->setWeight(duration);
		mSource->setWeight(1-duration);
		newTarget->setEnabled(true);
		newTarget->setLoop(true);
		mSource->setLoop(true);
		mSource->setEnabled(true);
		mTransition = transition;
		mTarget = newTarget;
		
	}
	else 
	{ 
		Ogre::AnimationState *newTarget = mEntity->getAnimationState( animation );
		if( mTimeleft > 0 )
		{
			// oops, weren't finished yet
			if( newTarget == mTarget )
			{
				// nothing to do! (ignoring duration here)
			}
			else if( newTarget == mSource )
			{
				// going back to the source state, so let's switch
				mSource = mTarget;
				mTarget = newTarget;
				mTimeleft = mDuration - mTimeleft; // i'm ignoring the new duration here
			}
			else
			{
				// ok, newTarget is really new, so either we simply replace the target with this one, or
				// we make the target the new source
				if( mTimeleft < mDuration * 0.5 )
				{
					// simply replace the target with this one
					mTarget->setEnabled(false);
					mTarget->setWeight(0);
				}
				else
				{
					// old target becomes new source
					mSource->setEnabled(false);
					mSource->setWeight(0);
					mSource = mTarget;
				} 
				mTarget = newTarget;
				mTarget->setEnabled(true);
				mTarget->setWeight( 1.0 - mTimeleft / mDuration );
				mTarget->setTimePosition(0);
			}
		}
		else
		{
			// assert( target == 0, "target should be 0 when not blending" )
			// mSource->setEnabled(true);
			// mSource->setWeight(1);
			mTransition = transition;
			mTimeleft = mDuration = duration;
			mTarget = newTarget;
			mTarget->setEnabled(true);
			mTarget->setWeight(0);
			mTarget->setTimePosition(0);
		}
	}
}

void AnimationBlender::blend( std::vector<std::pair<std::string,float>> _newAnimations,float duration, bool _loop/*=true*/,BlendingTransition transition )
{
	loop = _loop;
	mTransition = transition;
	mTimeleft = mDuration = duration;
	if(newAnimations.size()==0)
	{
		oldAnimations.clear();
		newAnimations.clear();
		//set the old and new animations to be the same (initial state, no blending)
		for(int i=0; i< _newAnimations.size(); i++)
		{
			/*oldAnimations.push_back(std::pair<Ogre::AnimationState*,float>(mEntity->getAnimationState(""),0));
			oldAnimations[oldAnimations.size()-1].first->setWeight(0);
			oldAnimations[oldAnimations.size()-1].first->setLoop(loop);
			oldAnimations[oldAnimations.size()-1].first->setEnabled(true);*/

			newAnimations.push_back(std::pair<Ogre::AnimationState*,float>(mEntity->getAnimationState(_newAnimations[i].first),_newAnimations[i].second));
			newAnimations[newAnimations.size()-1].first->setWeight(_newAnimations[i].second);
			newAnimations[newAnimations.size()-1].first->setLoop(loop);
			newAnimations[newAnimations.size()-1].first->setEnabled(true);
		}
	}
	else
	{
		//swap the old and new animation and set the new
		oldAnimations.clear();
		for(int i=0; i<newAnimations.size();i++)
		{
			oldAnimations.push_back(newAnimations[i]);
		}
		newAnimations.clear();
		for(int i=0; i< _newAnimations.size(); i++)
		{
		
			newAnimations.push_back(std::pair<Ogre::AnimationState*,float>(mEntity->getAnimationState(_newAnimations[i].first),_newAnimations[i].second));
			//newAnimations[newAnimations.size()-1].first->setWeight(_newAnimations[i].second);
			newAnimations[newAnimations.size()-1].first->setLoop(loop);
			newAnimations[newAnimations.size()-1].first->setEnabled(true);
		}
	}
}
void AnimationBlender::blend(std::vector<std::pair<std::string,float>> _newAnimations,bool print)
{
	/*for(int i=0; i<mAnims.size(); i++)
	{
		bool found =false;
		for(int j=0; j<_newAnimations.size(); j++)
		{
			if(mAnims[i]->getAnimationName() == _newAnimations[j].first)
			{
				if(!mAnims[i]->getEnabled())
				{
					found=true;
					mAnimWeights[i] = _newAnimations[j].second;
					mAnims[i]->setEnabled(true);
					mFadingIn[i]=true;
				}
				else
				{
					found=true;
					mAnimWeights[i] = _newAnimations[j].second;
				}
				
			}
		}
		if(!found)
		{
			mFadingIn[i] = false;
			mFadingOut[i] = true;
		}
		
	}*/

	for(int i=0; i<_newAnimations.size(); i++)
	{
		if(_newAnimations[i].first.find("Jump")!=-1 ||
			_newAnimations[i].first.find("Stop")!=-1||
			_newAnimations[i].first.find("Fall")!=-1||
			_newAnimations[i].first.find("Throw")!=-1||
			_newAnimations[i].first.find("Attack")!=-1)
		{
			bool hasJump=false;
			for(int j=0; j<mAnims.size(); j++)
			{
				if(mAnims[j]->getLoop()==false && mAnims[j]->getEnabled()==true)
				{
					hasJump =true;
				}
			}
			//Make tuning animation priority 
			if(_newAnimations[i].first.find("Turn")!=-1)
			{
				hasJump=false;
				for(int j=0; j<mAnims.size(); j++)
				{
					mAnims[j]->setEnabled(false);
					mAnims[j]->setLoop(true);
				}
			}
			//

			if(!hasJump)
				for(int j=0; j<mAnims.size(); j++)
				{
					if(_newAnimations[i].first == mAnims[j]->getAnimationName())
					{
						mAnims[j]->setEnabled(true);
						mAnims[j]->setLoop(false);
						mAnims[j]->setWeight(1.0f);
						mAnims[j]->setTimePosition(0);
						mAnimWeights[i] =1;
						if(print)
							printf("Pose:: %s\n",_newAnimations[i].first.c_str());
					
					}
					else
					{
						mAnims[j]->setEnabled(false);
					}
			}

		}
		else
		{
			bool hasJump=false;
			for(int j=0; j<mAnims.size(); j++)
			{
				if(mAnims[j]->getLoop()==false && mAnims[j]->getEnabled()==true)
				{
					isControllable = false;
					hasJump =true;
				}
			}
			if(!hasJump)
			{
				isControllable = true;
				for(int j=0; j<mAnims.size(); j++)
				{
					if(_newAnimations[i].first == mAnims[j]->getAnimationName())
					{
						mAnimWeights[i] = _newAnimations[i].second;
						mAnims[j]->setEnabled(true);
						mAnims[j]->setWeight(_newAnimations[i].second);
						if(print)
							printf("Pose:: %s\n",_newAnimations[i].first.c_str());
						//mFadingIn[i]=true;
					}
					else
					{
						mAnims[j]->setEnabled(false);
						mAnims[j]->setWeight(0);
					}
				}
				
			}

		}
	}

	bool hasJump=false;
	for(int j=0; j<mAnims.size(); j++)
	{
		if(mAnims[j]->getLoop()==false && mAnims[j]->getEnabled()==true)
		{
			isControllable = false;
			hasJump =true;
		}
	}
	if(!hasJump)
	{
		isControllable = true;
	}

}
void AnimationBlender::addTime( Ogre::Real time )
{
	//if( mSource != 0 )
	//{
	//	if( mTimeleft > 0 && mTransition != AnimationBlender::BlendState)
	//	{
	//		mTimeleft -= time;
	//		if( mTimeleft < 0 )
	//		{
	//			// finish blending
	//			mSource->setEnabled(false);
	//			mSource->setWeight(0);
	//			mSource = mTarget;
	//			mSource->setEnabled(true);
	//			mSource->setWeight(1);
	//			mTarget = 0;
	//		}
	//		else
	//		{
	//			// still blending, advance weights
	//			mSource->setWeight(mTimeleft / mDuration);
	//			mTarget->setWeight(1.0 - mTimeleft / mDuration);
	//			if(mTransition == AnimationBlender::BlendWhileAnimating)
	//				mTarget->addTime(time);
	//		}
	//	}
	//	else if(mTransition==AnimationBlender::BlendState)
	//	{
	//		mTarget->addTime(time);
	//	}
	//	if (mSource->getTimePosition() >= mSource->getLength())
	//	{
	//		complete = true;
	//	}
	//	else
	//	{
	//		complete = false;
	//	}
	//	mSource->addTime(time);
	//	mSource->setLoop(loop);
	//}
	if(newAnimations.size()>0)
	{
		if( mTimeleft > 0)
		{
			mTimeleft -= time;
			if( mTimeleft < 0 )
			{
				// finish blending
				/*mSource->setEnabled(false);
				mSource->setWeight(0);
				mSource = mTarget;
				mSource->setEnabled(true);
				mSource->setWeight(1);
				mTarget = 0;*/
				for (int i=0; i<oldAnimations.size();i++)
				{
					//oldAnimations[i].first->setWeight(0);
				}
			}
			else
			{
				// still blending, advance weights
				float blendingWeight = mTimeleft / mDuration;
				for (int i=0; i<newAnimations.size();i++)
				{
					
					float oldWeight=0;
					for (int j=0; j<oldAnimations.size();j++)
					{
						if(oldAnimations[j].first == newAnimations[i].first)
						{
							oldWeight = oldAnimations[j].first->getWeight();
						}
					}
					if(oldWeight<(1 - newAnimations[i].second*blendingWeight)){
						//printf("Set New Weight: %f\n",1 - newAnimations[i].second*blendingWeight);
						newAnimations[i].first->setWeight(1 - newAnimations[i].second*blendingWeight);
					}
					//newAnimations[i].first->addTime(time);
					//newAnimations[i].first->setLoop(loop);
				}
				for (int i=0; i<oldAnimations.size();i++)
				{
					float newWeight=0;
					for (int j=0; j<newAnimations.size();j++)
					{
						if(newAnimations[j].first == oldAnimations[i].first)
						{
							newWeight = newAnimations[j].first->getWeight();
						}
					}
					if(newWeight==0)
					{
						//printf("Set Old Weight: %f\n",oldAnimations[i].second*blendingWeight);
						oldAnimations[i].first->setWeight(oldAnimations[i].second*blendingWeight);
					}
					if(mTransition == AnimationBlender::BlendWhileAnimating)
						oldAnimations[i].first->addTime(time);
				}

				
			}
		}
		for (int i=0; i<newAnimations.size();i++)
		{
			newAnimations[i].first->addTime(time);
		}
	}
}
void AnimationBlender::fadeAnimations(Ogre::Real deltaTime)
{

	for (int i = 0; i < mAnims.size(); i++)
	{
		mAnims[i]->addTime(deltaTime);

		if(mAnims[i]->getLoop()==false && mAnims[i]->getTimePosition()>=mAnims[i]->getLength())
		{
			mAnims[i]->setEnabled(false);
			mAnims[i]->setLoop(true);
			mAnims[i]->setWeight(0);
			mFadingOut[i]=false;
			mFadingIn[i] = false;
		}

		if (mFadingIn[i])
		{
			// slowly fade this animation in until it has full wieght
			Ogre::Real newWeight = mAnims[i]->getWeight() + deltaTime * ANIM_FADE_SPEED;
			mAnims[i]->setWeight(Ogre::Math::Clamp<Ogre::Real>(newWeight, 0, mAnimWeights[i]));

			if (newWeight >= mAnimWeights[i])
			{
				mFadingIn[i] = false;
				if(mAnims[i]->getLoop()==false)
				{
					mAnims[i]->setEnabled(false);
					mFadingOut[i]=false;
				}
			}
		}
		if (mFadingOut[i])
		{
			// slowly fade this animation out unit it has no weight, add the disable it
			Ogre::Real newWeight = mAnims[i]->getWeight() - deltaTime * ANIM_FADE_SPEED;
			if(mAnimWeights[i]>0)
				mAnims[i]->setWeight(Ogre::Math::Clamp<Ogre::Real>(newWeight, 0, mAnimWeights[i]));

			if (newWeight <= 0)
			{
				mAnims[i]->setEnabled(false);
				mFadingOut[i] = false;
				mAnimWeights[i] = 0;
			}
		}
	}
}
AnimationBlender::AnimationBlender( Ogre::Entity *entity ) : mEntity(entity) 
{
}