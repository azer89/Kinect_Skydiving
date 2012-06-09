
#ifndef __OgreKinect_h_
#define __OgreKinect_h_

#include "Stdafx.h"
#include "PoseDetect.h"

#pragma comment(lib, "Kinect10.lib")

class OgreKinect
{
public:
	OgreKinect(Ogre::SceneManager* mSceneMgr);
    virtual ~OgreKinect(void);

	void init(Ogre::SceneManager* mSceneMgr);
	std::vector<bool> update(const float& dt);

	// Current kinect
	KinectSDK::INuiSensor *            m_pNuiSensor;
	BSTR                    m_instanceId;

	// thread handling
	HANDLE        m_hNextDepthFrameEvent;
	HANDLE        m_hNextColorFrameEvent;
	HANDLE        m_hNextSkeletonEvent;
	HANDLE        m_pDepthStreamHandle;
	HANDLE        m_pVideoStreamHandle;

	DWORD         m_SkeletonIds[NUI_SKELETON_COUNT];
	RGBQUAD       m_rgbWk[640*480];

	// Show Depth map or Color map
	Ogre::TexturePtr texRenderTarget;
	PoseDetect *mPoseDetect;

	bool isTracking;

protected:
	bool bSnapshot;
	

	bool bStart;
	bool bInitKinect;
	HRESULT initKinect();
	std::vector<bool> UpdateKinectSkeleton();
	void Nui_GotDepthAlert(float waitTime);
	void Nui_GotColorAlert(float waitTime);
	RGBQUAD Nui_ShortToQuad_Depth( USHORT s );
};
KinectSDK::Vector4 ogreVector3toVector4(Ogre::Vector3 vec);
#endif // #ifndef __OgreKinect_h_
