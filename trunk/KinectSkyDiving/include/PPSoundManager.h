//////////////////////////////////////////////////////////////////////////
//
//    Goal: a simple packet for irrKlang library
//
//    Features:  1. auto memory control                    
//               2. play effect
//	             3. play music   (fade in between)
//
//    How to:  
//		       Step 1: change the folder path to your own folder which include all the file of sound
//		       Step 2: #include "PPSoundManager.h" in which cpp that you need sound
//		       Step 3: new mPPSoundManager only once and mPPSoundManager->update in your frame update
//			   Step 4: mPPSoundManager->playEffect("1.wav"), mPPSoundManager->playMusic("1.mp3") whenever you want
//
//////////////////////////////////////////////////////////////////////////
//
//    Author: PP, genialpp@gmail.com
//  
//////////////////////////////////////////////////////////////////////////

#ifndef __PPSoundManager__
#define __PPSoundManager__

#include "irrKlang-1.3.0/irrKlang.h"
#include <vector>

#pragma comment(lib, "irrKlang.lib")

//////////////////////////////////////////////////////////////////////////
const float fadeSpeed = 0.5f;

//////////////////////////////////////////////////////////////////////////
const std::string filePath = "../../media/sound/";


//
//
class PPSound : public irrklang::ISoundStopEventReceiver
{
public:
	PPSound(irrklang::ISound* _ptrSound, std::string _fileName, bool needFadeIn = false)
	{
		bKillMePLZ = false;
		bFadeIn = needFadeIn;
		
		fileName = _fileName;
		fVolume = (needFadeIn)? 0.0f: 1.0f;
		
		ptrSound = _ptrSound;
		ptrSound->setVolume(fVolume);
		ptrSound->setSoundStopEventReceiver(this);
	}

	void increaseVolume(float v)
	{
		fVolume += v; 
		if (fVolume > 1) fVolume = 1.0f;
		ptrSound->setVolume(fVolume);
	};

	void decreaseVolume(float v)
	{
		fVolume -= v; 
		if (fVolume < 0) fVolume = 0.0f;
		ptrSound->setVolume(fVolume);
	};

	std::string getName(){ return fileName; }
	float getVolume(){ return fVolume; }
	bool isKillMePLZ(){ return bKillMePLZ; }
	bool isFadeIn(){ return bFadeIn; }

protected:
	bool bKillMePLZ;
	bool bFadeIn;
	float fVolume;
	std::string fileName;
	irrklang::ISound* ptrSound;

	virtual void OnSoundStopped (irrklang::ISound* sound, irrklang::E_STOP_EVENT_CAUSE reason, void* userData)
	{
		bKillMePLZ = true;
	}
};


//
//
class PPSoundManager
{
protected:
	irrklang::ISoundEngine* engine;
	std::vector<PPSound *> vecPPSound;

	bool bSwitchMusic;
	PPSound* mMusic;
	PPSound* mMusicNext;

public:
	PPSoundManager()
	{
		engine = irrklang::createIrrKlangDevice();
		vecPPSound.clear();

		bSwitchMusic = false;
		mMusic = NULL;
		mMusicNext = NULL;
	}

	~PPSoundManager(void)
	{
		engine->drop();
	}

	void update(float dt)
	{
		for (int i=vecPPSound.size()-1; i>=0; i--)
		{
			if (vecPPSound[i]->isKillMePLZ())
			{
				vecPPSound.erase(vecPPSound.begin() + i);
			}
		}

		if (bSwitchMusic)
		{
			mMusic->decreaseVolume(dt * fadeSpeed * 2);
			if (mMusic->getVolume() == 0.0f) // mute the current one first then increase the next one
				mMusicNext->increaseVolume(dt * fadeSpeed / 2);

			if (mMusic->getVolume() == 0.0f && mMusicNext->getVolume() == 1.0f)
			{
				delete mMusic;
				mMusic = mMusicNext;
				mMusicNext = NULL;
				bSwitchMusic = false;
			}
		}
		else if (mMusic)
		{
			if (mMusic->isFadeIn() && mMusic->getVolume() < 1.0f)
				mMusic->increaseVolume(dt * fadeSpeed);
		}
	}

	void playEffect(std::string effectFile)
	{
		std::string str(filePath);
		str.append(effectFile);
		printf("playEffect %s\n", effectFile.c_str());
		irrklang::ISound* snd = engine->play2D(str.c_str(), false, false, true);
		if (snd)
		{
			vecPPSound.push_back(new PPSound(snd, str));
		}
	}

	void playMusic(std::string musicFile, bool bLoop = true)
	{
		std::string str(filePath);
		str.append(musicFile);
		printf("playMusic %s\n", musicFile.c_str());

		irrklang::ISound* snd = engine->play2D(str.c_str(), bLoop, false, true);
		if (mMusic)
		{
			bSwitchMusic = true;
			mMusicNext = new PPSound(snd, str, true);
		}
		else
		{
			mMusic = new PPSound(snd, str, true);
		}
	}
}; 

//
//
extern PPSoundManager* mPPSoundManager;

#endif
