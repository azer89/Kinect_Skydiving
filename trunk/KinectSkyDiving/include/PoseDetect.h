#pragma once
#include "Stdafx.h"
#include <vector>

class PPose
{
public:
	bool bFlag;
	Ogre::String poseName;

	bool isTrack[20];
	Ogre::Quaternion oriSkeleton[20];
	Ogre::Vector3 vecSkeleton[20];
};

class PoseDetect
{
private:
	void setFlag(std::string poseName);
	

public:
	PoseDetect(void);
	~PoseDetect(void);

	void clearFlag();

	std::vector<PPose> vecPose;

	// use this function to capture the pose
	// and it will save into text.txt
	void savePose(PPose newPose);

	// load the pose which you have been saved
	void loadPose(Ogre::String poseName);
	bool isPose(std::string name);
	// return the probability for each pose you load is close to the current one most
	std::vector<bool> recognizePose(PPose curPose);
};

