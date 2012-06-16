#ifndef _UNIVERSE_H__
#define _UNIVERSE_H__

#include "DVector3.h"
#include "ConfigScript.h"

#include "SpaceBackdrop.h"
#include "Planet.h"
#include "SimplePlanet.h"
#include "SpritePlanet.h"
#include "Star.h"
#include "Atmosphere.h"

#include <OgreSceneNode.h>
#include <OgreEntity.h>
#include <OgreVector3.h>
#include <OgreTimer.h>
#include <OgreCamera.h>
#include <OgreLight.h>

#include <vector>

#define RENDER_QUEUE_OpaquePlanet 10
#define RENDER_QUEUE_Starfield 11
#define RENDER_QUEUE_Star 12
#define RENDER_QUEUE_TransparentPlanet 12
#define RENDER_QUEUE_Atmosphere 13

namespace GalaxyEngine
{
	class StarProxy;
	class PlanetProxy;

	struct FloatingOrigin
	{
	public:
		FloatingOrigin() 
		{
			target = NULL;
			position = DVector3(0, 0, 0);
			rotation = Ogre::Quaternion::IDENTITY;
			inverseRotation = rotation.Inverse();
		}

		DVector3 translateLocalToGlobal(const Ogre::Vector3 &pos);
		Ogre::Quaternion translateLocalToGlobal(const Ogre::Quaternion &rot);
		Ogre::Vector3 translateGlobalToLocal(const DVector3 &pos);
		Ogre::Quaternion translateGlobalToLocal(const Ogre::Quaternion &rot);

		inline void *getTarget() { return target; }	
		inline const DVector3 &getPosition() { return position; }	
		inline const Ogre::Quaternion &getRotation() { return rotation; }	
		inline const Ogre::Quaternion &getInverseRotation() { return inverseRotation; }

		inline void setTarget(void *t) { target = t; }
		inline void setPosition(const DVector3 &pos) { position = pos; }
		inline void setRotation(const Ogre::Quaternion &rot) {
			rotation = rot;
			inverseRotation	= rotation.Inverse();
		}

	private:
		void *target;
		DVector3 position;
		Ogre::Quaternion rotation, inverseRotation;
	};  


	class Universe
	{
	public:
		Universe(const Ogre::String &universeName);
		~Universe();

		inline double getSimGravityFactor() { return gravityFactor; }
		inline double getSimStepSize() { return stepSize; }
		inline unsigned int getSimStepDelay() { return stepDelay; }

		inline const Ogre::String &getMediaPath() { return mediaPath; }

		PlanetProxy *addPlanet(const Ogre::String &planetName);
		StarProxy *addStar(float radius, const Ogre::String &starColorMap);
		void update();

		StarProxy *getNearStar() { return maxStar; }
		PlanetProxy *getNearPlanet() { return maxPlanet; }
		PlanetProxy * getFirstPlanet()
		{
			PlanetProxy* firstPlanet = &planets[0];
			return firstPlanet;
		}

		inline FloatingOrigin &getCurrentOrigin() { return origin; }

		void registerOriginChangeHandler(void(*handlerFunction)(FloatingOrigin &lastOrigin, FloatingOrigin &currentOrigin)) 
		{
			originChangeHandlers.push_back(handlerFunction);
		}

		Ogre::Light *getLightSource() { return sunLight01; }

		//PlanetProxy getPlanetProxy(int index)
		//{
		//	return planets[index];
		//}


	private:
		Ogre::String mediaPath;

		//SpaceBackdrop *backdrop;

		//Ogre::Camera *camera;
		float perspectiveScalingFactor;
		PlanetProxy *fullyLoadedPlanet, *maxPlanet;
		StarProxy *maxStar;

		std::vector<PlanetProxy> planets;
		std::vector<StarProxy> stars;
		
		FloatingOrigin origin;
		std::vector<void(*)(FloatingOrigin&, FloatingOrigin&)> originChangeHandlers;

		Ogre::Light *sunLight01;
		Ogre::SceneNode *sunLightNode01;

		Ogre::Light *sunLight02;
		Ogre::SceneNode *sunLightNode02;

		//Ogre::Light *sunLight03;
		//Ogre::SceneNode *sunLightNode03;

		double gravityFactor;

		static Utility::Timer timer;
		unsigned long lastTime;

		double stepSize;			//Global speed scale value
		unsigned int stepDelay;		//The exact amount of time between each simulation step
		unsigned long stepTimer;	//The time (milliseconds) since the last simulation step
	};


	class PlanetProxy
	{
	public:
		enum PlanetLOD 
		{
			LOD_Complex = 0,
			LOD_Simple,
			LOD_Sprite,
			LOD_Invisible,
		};

		double mass;

	public:

		PlanetProxy(Universe *u) 
		{ 
			universe = u; 
		}

		void initialize(const Ogre::String &name);
		void destroy();

		void update();

		void loadLOD(PlanetLOD lod);
		void unloadLOD(PlanetLOD lod);
		bool isloadedLOD(PlanetLOD lod);
		void setLOD(PlanetLOD lod);

		inline PlanetLOD getLOD() { return currentLOD; }
		inline Planet* getPlanet() 
		{ 
			return planet; 
		}

		void setNextPosition(const DVector3 pos);
		inline const DVector3 &getNextPosition() { return nextPosition; }
		inline const DVector3 &getPosition() { return position; }

		inline void setAxialRotation(const Ogre::Vector3 &axisOfRotation, Ogre::Radian currentAngle, Ogre::Radian angularMomentum) {
			this->axisOfRotation = axisOfRotation;
			this->currentAngle = currentAngle;
			this->angularMomentum = angularMomentum;
		}

		inline const Ogre::Quaternion &getRotation() { return currentRotation; }

		void setVelocity(const DVector3 velocity);
		DVector3 getVelocity();

	private:
		friend class Universe;
		Universe *universe;

		DVector3 nextPosition;
		DVector3 lastPosition;
		DVector3 position;

		Ogre::Vector3 axisOfRotation;
		Ogre::Radian angularMomentum;
		Ogre::Radian currentAngle;
		Ogre::Quaternion currentRotation;

		ConfigNode *cfgScript;
		Ogre::SceneNode *node, *atmosphereSubNode;

		PlanetLOD currentLOD;
		Planet *planet;
		SimplePlanet *simplePlanet;
		SpritePlanet *spritePlanet;
        Atmosphere *atmosphere;

		//bool isLoaded;
		float radius;
	};


	class StarProxy
	{
	public:
		enum StarLOD {
			LOD_Complex = 0,
			LOD_Sprite,
			LOD_Invisible,
		};

		StarProxy(Universe *u) { universe = u; }

		void initialize(float radius, const Ogre::String &colorMap);
		void destroy();

		void update();

		void loadLOD(StarLOD lod);
		void unloadLOD(StarLOD lod);
		bool isloadedLOD(StarLOD lod);
		void setLOD(StarLOD lod);
		inline StarLOD getLOD() { return currentLOD; }

		void setNextPosition(const DVector3 pos);
		inline const DVector3 &getNextPosition() { return nextPosition; }
		inline const DVector3 &getPosition() { return position; }

		void setVelocity(const DVector3 velocity);
		DVector3 getVelocity();

		double mass;

	private:
		friend class Universe;
		Universe *universe;

		DVector3 nextPosition;
		DVector3 lastPosition;
		DVector3 position;

		Ogre::SceneNode *node;

		void createSceneNode();
		void destroySceneNode();

		StarLOD currentLOD;
		Star *star;

		float radius;
		Ogre::String starColorMap;
	};


}

#endif