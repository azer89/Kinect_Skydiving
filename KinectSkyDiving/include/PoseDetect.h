

#pragma once
#include "Stdafx.h"
#include <vector>

enum JointStatus
{
	TRACKED = 0,
	INFERRED = 1,
	LOST = 2
};

class PPose
{
public:
	bool bFlag;
	Ogre::String poseName;

	bool isTrack[20];
	//Ogre::Quaternion oriSkeleton[20];
	//Ogre::Vector3 vecSkeleton[20];
	//JointStatus skeletonStatus[20];
};

class PoseDetect
{
private:
	Ogre::Vector3		jointPosition[20];
	JointStatus			jointStatus[20];
	Ogre::Quaternion	jointOrientation[20];

private:
	void setFlag(std::string poseName);
	Ogre::Real checkCollinearity(Ogre::Vector3 v1, Ogre::Vector3 v2);

public:
	std::vector<PPose> vecPose;
	std::vector<bool> recognizePose(PPose curPose);		// return the probability for each pose you load is close to the current one most

public:
	PoseDetect(void);
	~PoseDetect(void);

	void clearFlag();

	void setVecSkeleton(int index, Ogre::Vector3 pos) { this->jointPosition[index] = pos; }
	void setJointStatus(int index, JointStatus status) { this->jointStatus[index] = status; }
	void setJointOrientation(int index, Ogre::Quaternion q) { this->jointOrientation[index] = q; }
	
	void savePose(PPose newPose);			// use this function to capture the pose and it will save into text.txt
	void loadPose(Ogre::String poseName);	// load the pose which you have been saved
	bool isPose(std::string name);
	
	
};

