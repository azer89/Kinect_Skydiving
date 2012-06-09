/*
-----------------------------------------------------------------------------
Filename:    OgreKinect.cpp
-----------------------------------------------------------------------------

  PP, genialpp@gmail.com

-----------------------------------------------------------------------------
*/

#include "Stdafx.h"
#include "OgreKinect.h"
#include <bitset>

OgreKinect::OgreKinect( Ogre::SceneManager* mSceneMgr )
{
	bInitKinect = false;

	init(mSceneMgr);
}

//-------------------------------------------------------------------------------------
OgreKinect::~OgreKinect(void)
{
	KinectSDK::NuiShutdown();
}

std::vector<bool> OgreKinect::update(const float& dt)
{
	std::vector<bool> result;
	if (bInitKinect)
	{
		Nui_GotDepthAlert(dt);
		//Nui_GotColorAlert(dt);
		return UpdateKinectSkeleton();
	}

	//update the interface
	//menu->hikariMgr->update();
	return result;
}

//-------------------------------------------------------------------------------------
void OgreKinect::init(Ogre::SceneManager* mSceneMgr)
{
	// init Kinect
	initKinect();

	// Show Color map or Depth map
	/*texRenderTarget = Ogre::TextureManager::getSingleton().createManual("texRenderTarget", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		Ogre::TEX_TYPE_2D, 640, 480, 0, Ogre::PF_B8G8R8A8, Ogre::TU_DEFAULT);*/
	texRenderTarget = Ogre::TextureManager::getSingleton().createManual("texRenderTarget", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		Ogre::TEX_TYPE_2D, 320, 240, 0, Ogre::PF_B8G8R8A8, Ogre::TU_DEFAULT);

	Ogre::Rectangle2D *mMiniScreen = new Ogre::Rectangle2D(true);
	mMiniScreen->setCorners(0.5f, -0.5f, 1.0f, -1.0f);
	mMiniScreen->setBoundingBox(Ogre::AxisAlignedBox(-100000.0f * Ogre::Vector3::UNIT_SCALE, 100000.0f * Ogre::Vector3::UNIT_SCALE));

	Ogre::SceneNode* miniScreenNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("MiniScreenNode");
	miniScreenNode->attachObject(mMiniScreen);	

	Ogre::MaterialPtr renderMaterial = Ogre::MaterialManager::getSingleton().create("matColormap", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	Ogre::Technique* matTechnique = renderMaterial->createTechnique();
	matTechnique->createPass();
	renderMaterial->getTechnique(0)->getPass(0)->setLightingEnabled(false);
	renderMaterial->getTechnique(0)->getPass(0)->createTextureUnitState("texRenderTarget");
	renderMaterial->getTechnique(0)->getPass(0)->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
	mMiniScreen->setMaterial("matColormap");
	
	//
	//jointCalc = new JointOrientationCalculator();
	mPoseDetect = new PoseDetect();

	// UI
// 	hViewPort = mCamera->getViewport();
// 	//initialize the interface
// 	menu = new Interface(this);
// 	menu->setupHikari();
// 	kinectUI = new KinectUiControl(this);


	// 
	std::fstream fp;
	std::string fr("..//..//Media//Assets//");
	fp.open("..//..//Media//Assets//Poses.txt", std::ios::in);
	
	while(!fp.eof())
	{
		std::string buff;
		fp >> buff;
		mPoseDetect->loadPose(fr + buff +std::string(".txt"));
	}
}

HRESULT OgreKinect::initKinect()
{
	HRESULT  hr;
	RECT     rc;
	bool     result;
	rc.bottom=0;
	rc.left=0;
	rc.right=0;
	rc.top=0;

	ZeroMemory(m_SkeletonIds,sizeof(m_SkeletonIds));

    hr = KinectSDK::NuiCreateSensorByIndex(0, &m_pNuiSensor);

	// Disable Kinect 
	if(rc.bottom==0 &&rc.left==0&&rc.right==0&&rc.top==0)
	  return -1;

	m_hNextDepthFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
	m_hNextColorFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
	m_hNextSkeletonEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

	DWORD nuiFlags = NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX | NUI_INITIALIZE_FLAG_USES_SKELETON |  NUI_INITIALIZE_FLAG_USES_COLOR;
	hr = m_pNuiSensor->NuiInitialize( nuiFlags );
	if ( E_NUI_SKELETAL_ENGINE_BUSY == hr )
	{
		hr = m_pNuiSensor->NuiInitialize( nuiFlags) ;
	}

	if ( FAILED( hr ) )
	{
		if ( E_NUI_DEVICE_IN_USE == hr )
		{
			//MessageBoxResource( IDS_ERROR_IN_USE, MB_OK | MB_ICONHAND );
		}
		else
		{
			//MessageBoxResource( IDS_ERROR_NUIINIT, MB_OK | MB_ICONHAND );
		}
		return hr;
	}

	if ( KinectSDK::HasSkeletalEngine( m_pNuiSensor ) )
	{
		hr = m_pNuiSensor->NuiSkeletonTrackingEnable( m_hNextSkeletonEvent, 0 );
		if( FAILED( hr ) )
		{
			//MessageBoxResource( IDS_ERROR_SKELETONTRACKING, MB_OK | MB_ICONHAND );
			return hr;
		}
	}

	hr = m_pNuiSensor->NuiImageStreamOpen(
		KinectSDK::NUI_IMAGE_TYPE_COLOR,
		KinectSDK::NUI_IMAGE_RESOLUTION_640x480,
		0,
		2,
		m_hNextColorFrameEvent,
		&m_pVideoStreamHandle );

	if ( FAILED( hr ) )
	{
		//MessageBoxResource( IDS_ERROR_VIDEOSTREAM, MB_OK | MB_ICONHAND );
		return hr;
	}

	hr = m_pNuiSensor->NuiImageStreamOpen(
		HasSkeletalEngine(m_pNuiSensor) ? KinectSDK::NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX : KinectSDK::NUI_IMAGE_TYPE_DEPTH,
		KinectSDK::NUI_IMAGE_RESOLUTION_320x240,
		0,
		2,
		m_hNextDepthFrameEvent,
		&m_pDepthStreamHandle );

	if ( FAILED( hr ) )
	{
		//MessageBoxResource(IDS_ERROR_DEPTHSTREAM, MB_OK | MB_ICONHAND);
		return hr;
	}

	bInitKinect = true;
	return hr;
}

//lookups for color tinting based on player index
static const int g_IntensityShiftByPlayerR[] = { 1, 2, 0, 2, 0, 0, 2, 0 };
static const int g_IntensityShiftByPlayerG[] = { 1, 2, 2, 0, 2, 0, 0, 1 };
static const int g_IntensityShiftByPlayerB[] = { 1, 0, 2, 2, 0, 2, 0, 2 };

RGBQUAD OgreKinect::Nui_ShortToQuad_Depth( USHORT s )
{
	USHORT RealDepth = KinectSDK::NuiDepthPixelToDepth(s);
	USHORT Player    = KinectSDK::NuiDepthPixelToPlayerIndex(s);

	// transform 13-bit depth information into an 8-bit intensity appropriate
	// for display (we disregard information in most significant bit)
	BYTE intensity = (BYTE)~(RealDepth >> 4);

	// tint the intensity by dividing by per-player values
	RGBQUAD color;
	color.rgbRed   = intensity >> g_IntensityShiftByPlayerR[Player];
	color.rgbGreen = intensity >> g_IntensityShiftByPlayerG[Player];
	color.rgbBlue  = intensity >> g_IntensityShiftByPlayerB[Player];

	return color;
}

void OgreKinect::Nui_GotColorAlert(float waitTime)
{
	KinectSDK::NUI_IMAGE_FRAME imageFrame;

	HRESULT hr = m_pNuiSensor->NuiImageStreamGetNextFrame( m_pVideoStreamHandle, waitTime, &imageFrame );

	if ( FAILED( hr ) )
	{
		return;
	}

	KinectSDK::INuiFrameTexture * pTexture = imageFrame.pFrameTexture;
	KinectSDK::NUI_LOCKED_RECT LockedRect;
	pTexture->LockRect( 0, &LockedRect, NULL, 0 );
	if ( LockedRect.Pitch != 0 )
	{
		//m_pDrawColor->Draw( static_cast<BYTE *>(LockedRect.pBits), LockedRect.size );

		// Get the pixel buffer
		Ogre::HardwarePixelBufferSharedPtr pixelBuffer = texRenderTarget->getBuffer();

		// Lock the pixel buffer and get a pixel box
		pixelBuffer->lock(Ogre::HardwareBuffer::HBL_NORMAL); // for best performance use HBL_DISCARD!
		const Ogre::PixelBox& pixelBox = pixelBuffer->getCurrentLock();

		Ogre::uint8* pDest = static_cast<Ogre::uint8*>(pixelBox.data);
		Ogre::uint8* pSorc = static_cast<Ogre::uint8*>(LockedRect.pBits);

		// Fill in some pixel data. This will give a semi-transparent blue,
		// but this is of course dependent on the chosen pixel format.
		for(size_t i = 0; i < 640; i++)
		{
			for (size_t j = 0; j < 480; j++)
			{
				*pDest++ = pSorc[0]; // B
				*pDest++ = pSorc[1]; // G
				*pDest++ = pSorc[2]; // R
				*pDest++ = 254; // A
				
				pSorc+=4;
			}
		}

		// Unlock the pixel buffer
		pixelBuffer->unlock();
	}
	else
	{
		//OutputDebugString( L"Buffer length of received texture is bogus\r\n" );
	}

	pTexture->UnlockRect( 0 );

	hr = m_pNuiSensor->NuiImageStreamReleaseFrame( m_pVideoStreamHandle, &imageFrame );
}

void OgreKinect::Nui_GotDepthAlert(float waitTime)
{
    KinectSDK::NUI_IMAGE_FRAME imageFrame;

    HRESULT hr = m_pNuiSensor->NuiImageStreamGetNextFrame( m_pDepthStreamHandle, waitTime, &imageFrame );
	if ( FAILED( hr ) )
	{
		return;
	}

    KinectSDK::INuiFrameTexture * pTexture = imageFrame.pFrameTexture;
    KinectSDK::NUI_LOCKED_RECT LockedRect;
    pTexture->LockRect( 0, &LockedRect, NULL, 0 );
    if( LockedRect.Pitch != 0 )
    {
		// Get the pixel buffer
		Ogre::HardwarePixelBufferSharedPtr pixelBuffer = texRenderTarget->getBuffer();

		// Lock the pixel buffer and get a pixel box
		pixelBuffer->lock(Ogre::HardwareBuffer::HBL_NORMAL); // for best performance use HBL_DISCARD!
		const Ogre::PixelBox& pixelBox = pixelBuffer->getCurrentLock();

		Ogre::uint8* pDest = static_cast<Ogre::uint8*>(pixelBox.data);
		USHORT * pSorc = (USHORT *)(LockedRect.pBits);

		// Fill in some pixel data. This will give a semi-transparent blue,
		// but this is of course dependent on the chosen pixel format.
		for(size_t i = 0; i < 320; i++)
		{
			for (size_t j = 0; j < 240; j++)
			{
				USHORT RealDepth = KinectSDK::NuiDepthPixelToDepth(*pSorc);
				USHORT Player    = KinectSDK::NuiDepthPixelToPlayerIndex(*pSorc);

				// transform 13-bit depth information into an 8-bit intensity appropriate
				// for display (we disregard information in most significant bit)
				BYTE intensity = (BYTE)~(RealDepth >> 4);

				// tint the intensity by dividing by per-player values
				RGBQUAD color;
				color.rgbRed   = intensity >> g_IntensityShiftByPlayerR[Player];
				color.rgbGreen = intensity >> g_IntensityShiftByPlayerG[Player];
				color.rgbBlue  = intensity >> g_IntensityShiftByPlayerB[Player];

				*pDest++ = color.rgbBlue; // B
				*pDest++ = color.rgbGreen; // G
				*pDest++ = color.rgbRed; // R
//				*pDest++ = (color.rgbBlue == color.rgbGreen && color.rgbBlue == color.rgbRed)? 0: 255; // A
				*pDest++ = 255; // A

				pSorc++;
			}
		}

		// Unlock the pixel buffer
		pixelBuffer->unlock();
    }

	pTexture->UnlockRect( 0 );

    m_pNuiSensor->NuiImageStreamReleaseFrame( m_pDepthStreamHandle, &imageFrame );
}


std::vector<bool> OgreKinect::UpdateKinectSkeleton()
{
	isTracking = false;
	std::vector<bool> result;
	KinectSDK::NUI_SKELETON_FRAME SkeletonFrame = {0};

	bool bFoundSkeleton = false;

	if (SUCCEEDED(m_pNuiSensor->NuiSkeletonGetNextFrame( 0, &SkeletonFrame )) )
	{
		for ( int i = 0 ; i < NUI_SKELETON_COUNT ; i++ )
		{
			if( SkeletonFrame.SkeletonData[i].eTrackingState == KinectSDK::NUI_SKELETON_TRACKED ||
				SkeletonFrame.SkeletonData[i].eTrackingState == KinectSDK::NUI_SKELETON_POSITION_ONLY)
			{
				bFoundSkeleton = true;
			}
		}
	}

	// no skeletons!
	if( !bFoundSkeleton )
	{
		return result;
	}

	// smooth out the skeleton data
	HRESULT hr = m_pNuiSensor->NuiTransformSmooth(&SkeletonFrame,NULL);
	if ( FAILED(hr) )
	{
		return result;
	}

	bool bSkeletonIdsChanged = false;
	for ( int i = 0 ; i < NUI_SKELETON_COUNT; i++ )
	{
		if ( m_SkeletonIds[i] != SkeletonFrame.SkeletonData[i].dwTrackingID )
		{
			m_SkeletonIds[i] = SkeletonFrame.SkeletonData[i].dwTrackingID;
			bSkeletonIdsChanged = true;
		}

		// Show skeleton only if it is tracked, and the center-shoulder joint is at least inferred.
		if ( SkeletonFrame.SkeletonData[i].eTrackingState == KinectSDK::NUI_SKELETON_TRACKED &&
			 SkeletonFrame.SkeletonData[i].eSkeletonPositionTrackingState[KinectSDK::NUI_SKELETON_POSITION_SHOULDER_CENTER] != KinectSDK::NUI_SKELETON_POSITION_NOT_TRACKED)
		{
			isTracking = true;

			// pose detection
			KinectSDK::NUI_SKELETON_BONE_ORIENTATION mBoneOrient[KinectSDK::NUI_SKELETON_POSITION_COUNT];
			if (KinectSDK::NuiSkeletonCalculateBoneOrientations(&SkeletonFrame.SkeletonData[i], mBoneOrient) == S_OK)
			{
				PPose newPose;
				for (int ii=0; ii<20; ii++)
				{
					newPose.isTrack[ii] = true;
					KinectSDK::Vector4 ori = mBoneOrient[ii].absoluteRotation.rotationQuaternion;
					newPose.oriSkeleton[ii] = Ogre::Quaternion(ori.w, ori.x, ori.y, ori.z);

					KinectSDK::Vector4 vec = SkeletonFrame.SkeletonData[i].SkeletonPositions[ii];
					newPose.vecSkeleton[ii] = Ogre::Vector3(vec.x , vec.y, vec.z);
				}

				result = mPoseDetect->recognizePose(newPose);
				return result;
			}
		}
	}

	return result;
}

KinectSDK::Vector4 ogreVector3toVector4(Ogre::Vector3 vec)
{
	KinectSDK::Vector4 v4;
	v4.w=1;
	v4.x=vec.x;
	v4.y=vec.y;
	v4.z=vec.z;
	return v4;
}
