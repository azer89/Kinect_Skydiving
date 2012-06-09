
#ifndef __AnimationBlender_h_
#define __AnimationBlender_h_

#include "Ogre.h"
#include <stdio.h>
class AnimationBlender
{
public:
	enum BlendingTransition
	{
		BlendSwitch,         // stop source and start dest
		BlendWhileAnimating,   // cross fade, blend source animation out while blending destination animation in
		BlendThenAnimate,      // blend source to first frame of dest, when done, start dest anim
		BlendState			//blend two states and play the blended animation (no transition)
	};

private:
	Ogre::Entity *mEntity;
	Ogre::AnimationState *mSource;
	Ogre::AnimationState *mTarget;

	BlendingTransition mTransition;
	bool loop;
	std::vector<std::pair<Ogre::AnimationState*,float>> oldAnimations;
	std::vector<std::pair<Ogre::AnimationState*,float>> newAnimations;
	std::vector<Ogre::AnimationState *> mAnims;
	std::vector<float> mAnimWeights;
	std::vector<bool> mFadingIn;
	std::vector<bool> mFadingOut;
	float ANIM_FADE_SPEED;

	~AnimationBlender() {}

public: 
	Ogre::Real mTimeleft, mDuration;

	bool complete;
	void blend(std::vector<std::pair<std::string,float>> newAnimations,float duration, bool _loop=true,BlendingTransition transition = BlendingTransition::BlendWhileAnimating);
	void blend( const Ogre::String &animation, BlendingTransition transition, Ogre::Real duration, bool l=true );
	void blend(std::vector<std::pair<std::string,float>> _newAnimations,bool print = false);
	void addTime( Ogre::Real );
	Ogre::Real getProgress() { return mTimeleft/ mDuration; }
	Ogre::AnimationState *getSource() { return mSource; }
	Ogre::AnimationState *getTarget() { return mTarget; }
	AnimationBlender(Ogre:: Entity *);
	void init( bool l=true, float fadeSpeed=1);
	void fadeAnimations(Ogre::Real deltaTime);
	std::string getAnimationName(int index) {return mAnims[index]->getAnimationName();}
	bool isControllable;
};

#endif
