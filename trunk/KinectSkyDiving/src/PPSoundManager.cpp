#include "Stdafx.h"
#include "PPSoundManager.h"

PPSoundManager* mPPSoundManager;

////

void PPSoundManager::update(float dt)
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

void PPSoundManager::playEffect(std::string effectFile)
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

void PPSoundManager::playMusic(std::string musicFile, bool bLoop)
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

