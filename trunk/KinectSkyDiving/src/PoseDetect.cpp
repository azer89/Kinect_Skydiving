#include "Stdafx.h"
#include "PoseDetect.h"


PoseDetect::PoseDetect(void)
{
	vecPose.clear();
}


PoseDetect::~PoseDetect(void)
{
	vecPose.clear();
}

void PoseDetect::loadPose( Ogre::String poseName )
{
	PPose newPose;
	newPose.poseName = poseName.substr(23, poseName.length() - 27);
	std::fstream fp;
	fp.open(poseName.c_str(), std::ios::in);
	for (int i=0; i<20; i++)
		fp >> newPose.isTrack[i];
	for (int i=0; i<20; i++)
		fp >> newPose.oriSkeleton[i].x >> newPose.oriSkeleton[i].y >> newPose.oriSkeleton[i].z >> newPose.oriSkeleton[i].w;	
	fp.close();
	vecPose.push_back(newPose);
}

std::vector<bool> PoseDetect::recognizePose( PPose curPose )
{
	float threadhold = 0.5f;
	std::vector<bool> flagPose;
	
	bool isRWrist_H_RShoulder = (curPose.vecSkeleton[KinectSDK::NUI_SKELETON_POSITION_WRIST_RIGHT].y > curPose.vecSkeleton[KinectSDK::NUI_SKELETON_POSITION_SHOULDER_RIGHT].y);
	bool isLWrist_H_LShoulder = (curPose.vecSkeleton[KinectSDK::NUI_SKELETON_POSITION_WRIST_LEFT].y > curPose.vecSkeleton[KinectSDK::NUI_SKELETON_POSITION_SHOULDER_LEFT].y);
	
	bool isRWrist_F_RShoulder = (curPose.vecSkeleton[KinectSDK::NUI_SKELETON_POSITION_WRIST_RIGHT].z < curPose.vecSkeleton[KinectSDK::NUI_SKELETON_POSITION_SHOULDER_RIGHT].z);
	bool isLWrist_F_LShoulder = (curPose.vecSkeleton[KinectSDK::NUI_SKELETON_POSITION_WRIST_LEFT].z < curPose.vecSkeleton[KinectSDK::NUI_SKELETON_POSITION_SHOULDER_LEFT].z);

	clearFlag();

	if (!isRWrist_F_RShoulder && !isLWrist_F_LShoulder)
	{
		setFlag("front");
	}

	if (isRWrist_H_RShoulder && !isLWrist_H_LShoulder)
	{
		setFlag("left");
	}

	if (!isRWrist_H_RShoulder && isLWrist_H_LShoulder)
	{
		setFlag("right");
	}

	if (isRWrist_F_RShoulder && !isLWrist_F_LShoulder)
	{
		setFlag("rotate_left");
	}

	if (!isRWrist_F_RShoulder && isLWrist_F_LShoulder)
	{
		setFlag("rotate_right");
	}

	for (int idxPose=0; idxPose<vecPose.size(); idxPose++)
	{
		flagPose.push_back(	vecPose[idxPose].bFlag );
	}
	
	return flagPose;
}

void PoseDetect::savePose( PPose newPose )
{
	std::fstream fp;
	fp.open("newPose.txt", std::ios::out);

	for (int i=0; i<20; i++)
		fp << newPose.isTrack[i] << " ";
	fp << std::endl;
	for (int i=0; i<20; i++)
		fp << newPose.oriSkeleton[i].x << " " << newPose.oriSkeleton[i].y << " " << newPose.oriSkeleton[i].z << " " << newPose.oriSkeleton[i].w << std::endl;	

	fp.close();
}

void PoseDetect::setFlag(std::string poseName)
{
	for (int i=0; i<vecPose.size(); i++)
	{
		if (vecPose[i].poseName == poseName)
		{
			vecPose[i].bFlag = true;
			return ;
		}
	}
}

void PoseDetect::clearFlag()
{
	for(int idx=0; idx<vecPose.size(); idx++)
		vecPose[idx].bFlag = false;
}

bool PoseDetect::isPose(std::string name)
{
	for(int i=0;i<vecPose.size();i++)
	{
		if(name == vecPose[i].poseName)
		{
			return vecPose[i].bFlag;
		}
	}
	return false;
}
