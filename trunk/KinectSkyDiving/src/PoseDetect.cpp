#include "Stdafx.h"
#include "PoseDetect.h"

//-------------------------------------------------------------------------------------
PoseDetect::PoseDetect(void)
{
	vecPose.clear();
}

//-------------------------------------------------------------------------------------
PoseDetect::~PoseDetect(void)
{
	vecPose.clear();
}

//-------------------------------------------------------------------------------------
void PoseDetect::loadPose( Ogre::String poseName )
{
	PPose newPose;
	newPose.poseName = poseName.substr(23, poseName.length() - 27);
	std::fstream fp;
	fp.open(poseName.c_str(), std::ios::in);
	for (int i=0; i<20; i++) fp >> newPose.isTrack[i];
	for (int i=0; i<20; i++) fp >> jointOrientation[i].x >> jointOrientation[i].y >> jointOrientation[i].z >> jointOrientation[i].w;	
	fp.close();
	vecPose.push_back(newPose);
}

//-------------------------------------------------------------------------------------
Ogre::Real PoseDetect::checkCollinearity(Ogre::Vector3 v1, Ogre::Vector3 v2)
{
	v1.normalise();
	v2.normalise();

	return v1.dotProduct(v2); // returned value is between 0..1
}

//-------------------------------------------------------------------------------------
std::vector<bool> PoseDetect::recognizePose( PPose curPose )
{
	std::vector<bool> flagPose;

	JointStatus rWrisStatus = jointStatus[KinectSDK::NUI_SKELETON_POSITION_WRIST_RIGHT];
	JointStatus lWristStatus = jointStatus[KinectSDK::NUI_SKELETON_POSITION_WRIST_LEFT];

	Ogre::Vector3 rWristPos =  jointPosition[KinectSDK::NUI_SKELETON_POSITION_WRIST_RIGHT];
	Ogre::Vector3 lWristPos =  jointPosition[KinectSDK::NUI_SKELETON_POSITION_WRIST_LEFT];
	Ogre::Vector3 rShoulderPos = jointPosition[KinectSDK::NUI_SKELETON_POSITION_SHOULDER_RIGHT];
	Ogre::Vector3 lShoulderPos = jointPosition[KinectSDK::NUI_SKELETON_POSITION_SHOULDER_LEFT];
	Ogre::Vector3 headPos = jointPosition[KinectSDK::NUI_SKELETON_POSITION_HEAD];
	Ogre::Vector3 cHipPos = jointPosition[KinectSDK::NUI_SKELETON_POSITION_HIP_CENTER];

	Ogre::Real unitY_RWrist_Col = checkCollinearity(Ogre::Vector3::UNIT_Y, rWristPos - rShoulderPos);
	Ogre::Real unitY_LWrist_Col = checkCollinearity(Ogre::Vector3::UNIT_Y, lWristPos - lShoulderPos);
	Ogre::Real unitZ_RWrist_Col = checkCollinearity(Ogre::Vector3::UNIT_Z, rWristPos - rShoulderPos);
	Ogre::Real unitZ_LWrist_Col = checkCollinearity(Ogre::Vector3::UNIT_Z, lWristPos - lShoulderPos);
	Ogre::Real negUnitZ_LShoulder_Col = checkCollinearity(Ogre::Vector3::NEGATIVE_UNIT_Z, lShoulderPos - rShoulderPos);
	Ogre::Real negUnitZ_RShoulder_Col = checkCollinearity(Ogre::Vector3::NEGATIVE_UNIT_Z, rShoulderPos - lShoulderPos);
	Ogre::Real unitX_head_Col = checkCollinearity(Ogre::Vector3::UNIT_X, headPos - cHipPos);
	Ogre::Real negUnitX_head_Col = checkCollinearity(Ogre::Vector3::NEGATIVE_UNIT_X, headPos - cHipPos);

	bool isRWrist_H_RShoulder = (rWristPos.y > rShoulderPos.y);
	bool isLWrist_H_LShoulder = (lWristPos.y > lShoulderPos.y);

	bool isRWrist_F_RShoulder = (rWristPos.z < rShoulderPos.z);
	bool isLWrist_F_LShoulder = (lWristPos.z < lShoulderPos.z);
	
	bool isLShoulder_F_RShoulder  = (lShoulderPos.z < rShoulderPos.z);
	bool isRShoulder_F_LShoulder = (rShoulderPos.z < lShoulderPos.z);;

	clearFlag();

	if(unitY_LWrist_Col > 0.4 && unitY_RWrist_Col > 0.4 && isRWrist_H_RShoulder && isLWrist_H_LShoulder)
	{
		setFlag("open");
	}
	else if ((unitZ_RWrist_Col > 0.6 || !isRWrist_F_RShoulder) && (unitZ_LWrist_Col > 0.6 || !isLWrist_F_LShoulder))
	{
		setFlag("front");
	}		
	else if (negUnitX_head_Col > 0.2 && !isLWrist_H_LShoulder)
	{
		setFlag("left");
	}
	else if (unitX_head_Col > 0.2 && isLWrist_H_LShoulder)		
	{
		setFlag("right");
	}
	else if (!isRWrist_H_RShoulder && isRWrist_F_RShoulder && negUnitZ_RShoulder_Col > 0.4)
	{
		setFlag("rotate_left");
	}		
	else if (!isLWrist_H_LShoulder && isLWrist_F_LShoulder && negUnitZ_LShoulder_Col > 0.4)
	{
		setFlag("rotate_right");
	}
	else
	{
		setFlag("none");
	}

	for (int idxPose = 0; idxPose<vecPose.size(); idxPose++)
	{
		flagPose.push_back(	vecPose[idxPose].bFlag );
	}
	
	return flagPose;
}

//-------------------------------------------------------------------------------------
void PoseDetect::savePose( PPose newPose )
{
	std::fstream fp;
	fp.open("newPose.txt", std::ios::out);
	for (int i = 0; i < 20; i++) fp << newPose.isTrack[i] << " ";	
	fp << std::endl;
	for (int i = 0; i < 20; i++) fp << jointOrientation[i].x << " " << jointOrientation[i].y << " " << jointOrientation[i].z << " " << jointOrientation[i].w << std::endl;	
	fp.close();
}

//-------------------------------------------------------------------------------------
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

//-------------------------------------------------------------------------------------
void PoseDetect::clearFlag()
{
	for(int idx=0; idx<vecPose.size(); idx++) vecPose[idx].bFlag = false;
}

//-------------------------------------------------------------------------------------
bool PoseDetect::isPose(std::string name)
{
	for(int i = 0;i<vecPose.size();i++)
	{
		if(name == vecPose[i].poseName)
		{
			return vecPose[i].bFlag;
		}
	}
	return false;
}
