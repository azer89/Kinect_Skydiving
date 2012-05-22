
#include "Stdafx.h"

#include "Universe.h"
#include "Core.h"
#include "Exception.h"
#include "ConfigScript.h"
#include "DVector3.h"
#include "Utility.h"

#include "SpaceBackdrop.h"
#include "Planet.h"
#include "SimplePlanet.h"
#include "SpritePlanet.h"
#include "PlanetLoader_Hybrid.h"
#include "Star.h"
#include "Atmosphere.h"

using namespace Ogre;
using namespace std;

#include <vector>

namespace GalaxyEngine
{
	Utility::Timer Universe::timer;
	
	Universe::Universe(Camera *cam, const Ogre::String &universeName)
	{
		//LOD vars
		camera = cam;
		fullyLoadedPlanet = NULL;

		//Load universe script
		ConfigNode *root = ConfigScriptLoader::getSingleton().getConfigScript("universe", universeName);
		if (!root) { EXCEPTION("Universe script not found.", "Universe::Universe()"); }

		//Initialize the resource group for the universe
		ConfigNode *node = root->findChild("mediaPath"); assert(node);
		Universe::mediaPath = Core::getSingleton().getMediaPath() + node->getValue();
		ResourceGroupManager::getSingleton().addResourceLocation(Universe::mediaPath, "FileSystem", "Universe");
		ResourceGroupManager::getSingleton().initialiseResourceGroup("Universe");

		//Init. variables
		stepSize = root->findChild("timeStep")->getValueF(0);
		stepDelay = 1000.0f / root->findChild("timeStep")->getValueF(1);
		gravityFactor = root->findChild("gravityFactor")->getValueF();
		lastTime = timer.getMilliseconds();
		stepTimer = 0;

		//Calculate the "perspective scaling factor" (used in LOD calculations)
		float viewportHeight = cam->getViewport()->getActualHeight();
		perspectiveScalingFactor = viewportHeight / (2 * Math::Tan(cam->getFOVy()/2));

		//Load backdrop
		//backdrop = new SpaceBackdrop(cam->getFarClipDistance() - 10.0f, 8);
		//backdrop->setBackdropImage(root->findChild("backdrop")->getValue());
        //backdrop->setRenderQueueGroup(RENDER_QUEUE_Starfield);

		//Load planets and stars
		uint32 nodes = (uint32)root->getChildren().size();
		for (uint32 i = 0; i < nodes; ++i) 
		{
			ConfigNode *node = root->getChild(i);
			ConfigNode *subNode;

			//Add star
			if (node->getName() == "star") 
			{
				subNode = node->findChild("radius"); assert(subNode);
				float radius = subNode->getValueF();
				subNode = node->findChild("colorMap"); assert(subNode);
				String starColorMap = subNode->getValue();

				StarProxy *star = addStar(radius, starColorMap);

				subNode = node->findChild("position"); assert(subNode);
				DVector3 position(subNode->getValueD(0), subNode->getValueD(1), subNode->getValueD(2));
				star->setNextPosition(position);

				subNode = node->findChild("velocity"); assert(subNode);
				DVector3 velocity(subNode->getValueD(0), subNode->getValueD(1), subNode->getValueD(2));
				star->setVelocity(velocity);

				subNode = node->findChild("mass"); assert(subNode);
				star->mass = subNode->getValueD();
			}

			//Add planet
			if (node->getName() == "planet") 
			{
				PlanetProxy *planet = addPlanet(node->getValue());

				subNode = node->findChild("position"); assert(subNode);
				DVector3 position(subNode->getValueD(0), subNode->getValueD(1), subNode->getValueD(2));
				planet->setNextPosition(position);
				
				subNode = node->findChild("velocity"); assert(subNode);
				DVector3 velocity(subNode->getValueD(0), subNode->getValueD(1), subNode->getValueD(2));
				planet->setVelocity(velocity);

				subNode = node->findChild("axialRotation"); assert(subNode);
				Vector3 axisOfRotation(subNode->getValueF(0), subNode->getValueF(1), subNode->getValueF(2));
				axisOfRotation.normalise();
				planet->setAxialRotation(axisOfRotation, Degree(subNode->getValueF(3)), Degree(subNode->getValueF(4)));

				subNode = node->findChild("mass"); assert(subNode);
				planet->mass = subNode->getValueD();
			}
		}

		//Setup sun light
		sunLightNode = Core::getSingleton().getSceneManager()->getRootSceneNode()->createChildSceneNode();
		sunLight = Core::getSingleton().getSceneManager()->createLight("UniverseSunLight");
		sunLightNode->attachObject(sunLight);
		sunLightNode->setPosition(0, 250, 0);

		/*
		Ogre::SceneNode* sunLightNode01 = Core::getSingleton().getSceneManager()->getRootSceneNode()->createChildSceneNode();
		Light* sunLight01 = Core::getSingleton().getSceneManager()->createLight("UniverseSunLight01");
		sunLightNode01->attachObject(sunLight01);
		sunLightNode01->setPosition(4, 0, 100);*/
	}

	Universe::~Universe()
	{
		//Delete planets		
		std::vector<PlanetProxy>::iterator i;
		for (i = planets.begin(); i != planets.end(); ++i) 
		{
			PlanetProxy &body = *i;
			body.destroy();
		}
		
		SimplePlanet::freeCachedMeshes();

		//Delete stars
		std::vector<StarProxy>::iterator o;
		for (o = stars.begin(); o != stars.end(); ++o) {
			StarProxy &body = *o;
			body.destroy();
		}

		//Delete backdrop
		//delete backdrop;

		//Delete sun light
		Core::getSingleton().getSceneManager()->destroyLight(sunLight);
		Core::getSingleton().getSceneManager()->destroySceneNode(sunLightNode->getName());
	}

	PlanetProxy *Universe::addPlanet(const String &planetName)
	{
		planets.push_back(PlanetProxy(this));
		PlanetProxy *body = &planets.back();

		body->initialize(planetName);

		return body;
	}

	StarProxy *Universe::addStar(float radius, const String &starColorMap)
	{
		stars.push_back(StarProxy(this));
		StarProxy *body = &stars.back();

		body->initialize(radius, starColorMap);

		return body;
	}

	void Universe::update()
	{
		//Get the time elapsed since the last frame
		unsigned long currentTime = timer.getMilliseconds();
		unsigned long deltaTime_ms = currentTime - lastTime;
		lastTime = currentTime;
		double deltaTime = deltaTime_ms / 1000.0;
		double stepSizeSquared = stepSize * stepSize;
	
		//Get misc. values
		const uint32 numPlanets = (uint32)planets.size();
		const uint32 numStars = (uint32)stars.size();

		//Convert the origin-relative camera position to an absolute one
		DVector3 camPos = origin.translateLocalToGlobal(camera->getDerivedPosition());

		//Check if it's time to perform another step of the gravity simulation
		stepTimer += deltaTime_ms;
		double interpolationFactor;
		if (stepTimer < stepDelay) 
		{
			//It's not time for another frame yet, so interpolate
			interpolationFactor = (double)stepTimer / (double)stepDelay;
		} 
		else 
		{
			//Reset interpolation and calculate a new frame
			stepTimer = 0;
			interpolationFactor = 0.0f;

			//Perform gravity simulation on all stars
			for (uint32 i = 0; i < numStars; ++i) 
			{
				StarProxy &body = stars[i];

				//Calculate the gravitational force to apply to the star. Planets are not considered
				//because their effect is too insignificant in the game and would only waste CPU time.
				Vector3 forceVec = Vector3::ZERO;
				for (uint32 o = 0; o < numStars; ++o) {
					if (o != i) {
						StarProxy &body2 = stars[o];

						//Gravitational force calculation
						DVector3 vec;
						vec.x = body2.nextPosition.x - body.nextPosition.x;					//Calculate the vector from this body to the other
						vec.y = body2.nextPosition.y - body.nextPosition.y;					//..
						vec.z = body2.nextPosition.z - body.nextPosition.z;					//..
						double distSquared = vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;	//Calculate squared distance
						double force = gravityFactor * (body2.mass) / distSquared;			//Calculate gravitational force
						double invDist = 1.0 / std::sqrt(distSquared);						//Precalculate inverse non-squared distance
						forceVec.x += (force * vec.x * invDist);					//Scale the direction vector by the force scalar
						forceVec.y += (force * vec.y * invDist);					//..
						forceVec.z += (force * vec.z * invDist);					//..
					}
				}

				//Apply the force to the star
				DVector3 tmp = body.nextPosition;
				body.nextPosition.x += (body.nextPosition.x - body.lastPosition.x) + forceVec.x * stepSizeSquared;
				body.nextPosition.y += (body.nextPosition.y - body.lastPosition.y) + forceVec.y * stepSizeSquared;
				body.nextPosition.z += (body.nextPosition.z - body.lastPosition.z) + forceVec.z * stepSizeSquared;
				body.lastPosition = tmp;
			}

			/*
			//Perform gravity simulation on all planets
			for (uint32 i = 0; i < numPlanets; ++i) 
			{
				PlanetProxy &body = planets[i];

				//Calculate the gravitational force to apply to the planet.
				Vector3 forceVec = Vector3::ZERO;
				for (uint32 o = 0; o < numStars; ++o) {
					StarProxy &body2 = stars[o];

					if (body2.mass > 0) {
						//Gravitational force calculation
						DVector3 vec;
						vec.x = body2.nextPosition.x - body.nextPosition.x;					//Calculate the vector from this body to the other
						vec.y = body2.nextPosition.y - body.nextPosition.y;					//..
						vec.z = body2.nextPosition.z - body.nextPosition.z;					//..
						double distSquared = vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;	//Calculate squared distance
						double force = gravityFactor * (body2.mass) / distSquared;			//Calculate gravitational force
						double invDist = 1.0 / std::sqrt(distSquared);						//Precalculate inverse non-squared distance
						forceVec.x += (force * vec.x * invDist);					//Scale the direction vector by the force scalar
						forceVec.y += (force * vec.y * invDist);					//..
						forceVec.z += (force * vec.z * invDist);					//..
					}
				}

				for (uint32 o = 0; o < numPlanets; ++o) 
				{
					if (o != i) {
						PlanetProxy &body2 = planets[o];

						//Gravitational force calculation
						DVector3 vec;
						vec.x = body2.nextPosition.x - body.nextPosition.x;					//Calculate the vector from this body to the other
						vec.y = body2.nextPosition.y - body.nextPosition.y;					//..
						vec.z = body2.nextPosition.z - body.nextPosition.z;					//..
						double distSquared = vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;	//Calculate squared distance
						double force = gravityFactor * (body2.mass) / distSquared;			//Calculate gravitational force
						double invDist = 1.0 / std::sqrt(distSquared);						//Precalculate inverse non-squared distance
						forceVec.x += (force * vec.x * invDist);					//Scale the direction vector by the force scalar
						forceVec.y += (force * vec.y * invDist);					//..
						forceVec.z += (force * vec.z * invDist);					//..
					}
				}

				//Apply the force to the planet
				DVector3 tmp = body.nextPosition;
				body.nextPosition.x += (body.nextPosition.x - body.lastPosition.x) + forceVec.x * stepSizeSquared;
				body.nextPosition.y += (body.nextPosition.y - body.lastPosition.y) + forceVec.y * stepSizeSquared;
				body.nextPosition.z += (body.nextPosition.z - body.lastPosition.z) + forceVec.z * stepSizeSquared;
				body.lastPosition = tmp;
			}
			*/
		}


		//Select the LODs and update interpolation of stars
		double maxStarPixelSize = 0.0;
		maxStar = NULL;
		for (uint32 i = 0; i < numStars; ++i) {
			StarProxy &star = stars[i];

			//Interpolate between the last and next position to get the current position. This way when the
			//gravity simulation runs at a very low frame rate (like 2 FPS), star orbits will still move smoothly.
			star.position.x = (star.nextPosition.x - star.lastPosition.x) * interpolationFactor + star.lastPosition.x;
			star.position.y = (star.nextPosition.y - star.lastPosition.y) * interpolationFactor + star.lastPosition.y;
			star.position.z = (star.nextPosition.z - star.lastPosition.z) * interpolationFactor + star.lastPosition.z;

			//Calculate distance / screen space pixel size
			double dist = std::max(0.0, camPos.distance(star.position) - star.radius);
			double pixelSize = perspectiveScalingFactor * (2 * star.radius) / dist;

			//Remember which star is largest on-screen
			if (pixelSize > maxStarPixelSize) {
				maxStarPixelSize = pixelSize;
				maxStar = &star;
			}

			//Set this star's appropriate LOD level
			StarProxy::StarLOD lodLevel;
			if (pixelSize <= 1) {
				//LOD_Invisible is used when the star is extremely far away, and would normally be smaller than one pixel
				star.unloadLOD(StarProxy::LOD_Sprite);
				star.unloadLOD(StarProxy::LOD_Complex);
				lodLevel = StarProxy::LOD_Invisible;
				star.loadLOD(StarProxy::LOD_Invisible);
			}
			if (pixelSize <= 10) {
				//LOD_Sprite is for long distances, where only a few pixels are rendered for the planet
				//At this LOD, all other LOD levels should be unloaded because the most likely won't be needed soon.
				star.unloadLOD(StarProxy::LOD_Complex);
				lodLevel = StarProxy::LOD_Sprite;
				star.loadLOD(StarProxy::LOD_Sprite);
			} else {
				//LOD_Complex is the highest level of detail, which loads everything on the planet for surface view.
				//If this LOD has not been loaded yet, the planet should use LOD_Medium until it is.
				lodLevel = StarProxy::LOD_Complex;
				star.loadLOD(StarProxy::LOD_Complex);
			}

			//If this LOD isn't loaded yet, use a lower detail one
			while (lodLevel < StarProxy::LOD_Invisible && !star.isloadedLOD(lodLevel))
				lodLevel = (StarProxy::StarLOD)(((uint32)lodLevel) + 1);

			//Set the chosen LOD
			star.setLOD(lodLevel);
		}


		//Select the LODs and update interpolation of planets
		double maxPlanetPixelSize = 0.0;
		maxPlanet = NULL;
		for (uint32 i = 0; i < numPlanets; ++i) {
			PlanetProxy &planet = planets[i];

			//Rotate the planet slowly around it's axis. This is done here in the LOD loop which is executed
			//every frame, and the angle is updated using delta time, rather than a fixed frame rate with
			//interpolation as the gravity calculations are. This is possible because linear rotation is
			//fairly deterministic, even with deltaTime, and planet rotation correctness isn't as important
			//as orbit correctness.
			planet.currentAngle += planet.angularMomentum * deltaTime;
			//Note: The actual quaternion is calculated per frame when the planet is being rendered,
			//otherwise the relatively expensive quaternion generation would be wasted on invisible planets.

			//Interpolate between the last and next position to get the current position. This way when the
			//gravity simulation runs at a very low frame rate (like 2 FPS), planet orbits will still move smoothly.
			planet.position.x = (planet.nextPosition.x - planet.lastPosition.x) * interpolationFactor + planet.lastPosition.x;
			planet.position.y = (planet.nextPosition.y - planet.lastPosition.y) * interpolationFactor + planet.lastPosition.y;
			planet.position.z = (planet.nextPosition.z - planet.lastPosition.z) * interpolationFactor + planet.lastPosition.z;

			//Calculate distance / screen space pixel size
			double dist = std::max(0.0, camPos.distance(planet.position) - planet.radius);
			double pixelSize = perspectiveScalingFactor * (2 * planet.radius) / dist;
	
			//Remember which planet is largest on-screen
			if (pixelSize > maxPlanetPixelSize) 
			{
				maxPlanetPixelSize = pixelSize;
				maxPlanet = &planet;
			}

			//Set this planet's appropriate LOD level
			PlanetProxy::PlanetLOD lodLevel;
			if (pixelSize <= 1) 
			{
				//LOD_Invisible is used when the planet is extremely far away, and would normally be smaller than one pixel
				planet.unloadLOD(PlanetProxy::LOD_Sprite);
				planet.unloadLOD(PlanetProxy::LOD_Simple);
				planet.unloadLOD(PlanetProxy::LOD_Complex);
				lodLevel = PlanetProxy::LOD_Invisible;
				planet.loadLOD(PlanetProxy::LOD_Invisible);
				if (fullyLoadedPlanet == &planet) fullyLoadedPlanet = NULL;
			}
			if (pixelSize <= 10) 
			{
				//LOD_Sprite is for long distances, where only a few pixels are rendered for the planet
				//At this LOD, all other LOD levels should be unloaded because the most likely won't be needed soon.
				planet.unloadLOD(PlanetProxy::LOD_Simple);
				planet.unloadLOD(PlanetProxy::LOD_Complex);
				lodLevel = PlanetProxy::LOD_Sprite;
				planet.loadLOD(PlanetProxy::LOD_Sprite);
				if (fullyLoadedPlanet == &planet) fullyLoadedPlanet = NULL;
			} else if (pixelSize <= 50) 
			{
				//LOD_Simple is for medium-ranged planets, needing moderate orbital-perspective detail, but not surface detail.
				lodLevel = PlanetProxy::LOD_Simple;
				//Load not only LOD_Simple, but also ensure LOD_Sprite is loaded
				planet.loadLOD(PlanetProxy::LOD_Sprite);
				planet.loadLOD(PlanetProxy::LOD_Simple);
			} else 
			{
				//LOD_Complex is the highest level of detail, which loads everything on the planet for surface view.
				//If this LOD has not been loaded yet, the planet should use LOD_Medium until it is.
				lodLevel = PlanetProxy::LOD_Complex;
				//Ensure LOD_Simple and LOD_Sprite are loaded - until LOD_Complex can load, it will fall back on these
				planet.loadLOD(PlanetProxy::LOD_Sprite);
				planet.loadLOD(PlanetProxy::LOD_Simple);
			}

			//If this LOD isn't loaded yet, use a lower detail one
			while (lodLevel < PlanetProxy::LOD_Invisible && !planet.isloadedLOD(lodLevel))
				lodLevel = (PlanetProxy::PlanetLOD)(((uint32)lodLevel) + 1);

			//If the planet will be rendered in LOD_Simple or LOD_Complex mode,
			//planet rotation will be visible, so it needs to be calculated here.
			if (lodLevel == PlanetProxy::LOD_Simple || lodLevel == PlanetProxy::LOD_Complex) {
				planet.currentRotation = Quaternion(planet.currentAngle, planet.axisOfRotation);
			}

			//Set the chosen LOD
			planet.setLOD(lodLevel);
		}

		//The largest on-screen planet should be fully loaded
		if (maxPlanet != NULL && maxPlanet != fullyLoadedPlanet) {
			//... but only if it's reasonably close (using at least 30 pixels on-screen)
			if (maxPlanetPixelSize >= 30) {
				//Unload the last full-LOD planet, and load the new one (only one planet should ever be fully loaded)
				if (fullyLoadedPlanet) fullyLoadedPlanet->unloadLOD(PlanetProxy::LOD_Complex);
				maxPlanet->loadLOD(PlanetProxy::LOD_Complex);
				fullyLoadedPlanet = maxPlanet;
			}
		}

		//Update floating origin if necessary
		if (maxPlanet != NULL && maxStar != NULL) {
			//Determine what should be used as the origin's center (planet, star, or camera)
			bool centerPlanet = false, centerStar = false;
			if (maxPlanet->getLOD() == PlanetProxy::LOD_Complex) {
				//If the closest planet is using LOD_Complex, it must be centered to render properly
				centerPlanet = true;
			} else {
				//Check which is larger on-screen - the planet or the star
				if (maxPlanetPixelSize >= maxStarPixelSize) {
					//Center the planet unless it's too far away
					if (maxPlanetPixelSize > 40) centerPlanet = true;
				} else {
					//Center the star unless it's too far away
					if (maxStarPixelSize > 40) centerStar = true;
				}
			}

			//Update the origin
			bool originChanged = false;
			FloatingOrigin lastOrigin = origin;
			if (centerPlanet) {
				//Center the origin around the planet
				origin.setPosition(maxPlanet->getPosition());
				origin.setRotation(maxPlanet->getRotation());
				if (origin.getTarget() != maxPlanet) {
					origin.setTarget(maxPlanet);
					originChanged = true;
				}
			}
			else if (centerStar) {
				//Center the origin around the star
				origin.setPosition(maxStar->getPosition());
				origin.setRotation(Quaternion::IDENTITY);
				if (origin.getTarget() != maxStar) {
					origin.setTarget(maxStar);
					originChanged = true;
				}
			}
			else {
				//Center the origin around the camera's current position if the camera is considerably
				//far away from the origin's current position.
				if (origin.getPosition().distanceSquared(camPos) > 1000 * 1000 || origin.getTarget() != NULL) {
					origin.setPosition(camPos);
					origin.setRotation(Quaternion::IDENTITY);
					origin.setTarget(NULL);
					originChanged = true;
				}
			}

			//If the origin has changed...
			if (originChanged) {
				//Notify all "origin change handlers" of the change in origin so appropriate
				//actions can be taken to keep objects in sync with the rest of the universe.
				std::vector<void(*)(FloatingOrigin&, FloatingOrigin&)>::iterator i;
				for (i = originChangeHandlers.begin(); i != originChangeHandlers.end(); ++i) {
					void (*handlerFunction)(FloatingOrigin &, FloatingOrigin &);
					handlerFunction = (*i);
					handlerFunction(lastOrigin, origin);
				}
			}
		}

		//Largest on-screen star should emit light
		//if (maxStar) {
		//	sunLightNode->setPosition(origin.translateGlobalToLocal(maxStar->getPosition()));
		//}

		//Ogre::Vector3 camePos = GalaxyEngine::Core::getSingleton().getCamera()->getDerivedPosition();
		//std::cout << camePos.x << "-" << camePos.y << "-" << camePos.z << "\n";


		//Update the LODs of every planet and star
		for (uint32 i = 0; i < numPlanets; ++i) {
			PlanetProxy &planet = planets[i];
			planet.update();
		}
		for (uint32 i = 0; i < numStars; ++i) {
			StarProxy &star = stars[i];
			star.update();
		}

		//Update planet renderer
		if (ChunkManager::getSingletonPtr() != NULL)
			ChunkManager::getSingleton().update();

		//Update starfield rotation
		//backdrop->setRotation(origin.translateGlobalToLocal(Quaternion::IDENTITY));

			}



	void PlanetProxy::initialize(const String &name)
	{
		cfgScript = ConfigScriptLoader::getSingleton().getConfigScript("planet", name);
		radius = cfgScript->findChild("terrain")->findChild("radius")->getValueF();

		currentLOD = LOD_Invisible;
		node = NULL;
		planet = NULL;
		simplePlanet = NULL;
		spritePlanet = NULL;
		atmosphere = NULL;

		nextPosition = DVector3(0, 0, 0);
		lastPosition = DVector3(0, 0, 0);
		position = DVector3(0, 0, 0);
		mass = 2.0;

		axisOfRotation = Vector3::UNIT_Y;
		angularMomentum = Radian(0.0f);
		currentAngle = Radian(0.0f);
		currentRotation = Quaternion::IDENTITY;
	}

	void PlanetProxy::destroy()
	{
		unloadLOD(LOD_Complex);
		unloadLOD(LOD_Simple);
		unloadLOD(LOD_Sprite);
		unloadLOD(LOD_Invisible);

		if (atmosphere) {
			delete atmosphere;
			atmosphere = NULL;
		}

		if (node)
			Core::getSingleton().getSceneManager()->destroySceneNode(node->getName());
	}

	void PlanetProxy::update()
	{
		if (node) {
			if (static_cast<PlanetProxy*>(universe->getCurrentOrigin().getTarget()) == this) {
				//If the current floating origin is set to this planet, it will be perfectly centered with no rotation
				node->setPosition(Vector3::ZERO);
				node->setOrientation(Quaternion::IDENTITY);
			} 
			else 
			{
				//Otherwise translate this planet according to the floating origin's position / rotation
				FloatingOrigin &origin = universe->getCurrentOrigin();
				node->setPosition(origin.translateGlobalToLocal(position));
				node->setOrientation(origin.translateGlobalToLocal(currentRotation));
				//node->setVisible(false);
				//node->showBoundingBox(true);
			}
		}
	}

	void PlanetProxy::loadLOD(PlanetLOD lod)
	{
		ConfigNode *cfgTerrain = cfgScript->findChild("terrain");
		switch (lod) {
			case LOD_Sprite:
				//Load the 2D, billboard version of the planet
				if (!spritePlanet) {
					float maxTerrainHeight = cfgTerrain->findChild("heightRange")->getValueF();
					spritePlanet = new SpritePlanet(radius + maxTerrainHeight * 0.5f, cfgScript->getValue());
					spritePlanet->setRenderQueueGroup(RENDER_QUEUE_TransparentPlanet);
					spritePlanet->setSunLight(universe->getLightSource());
					
					//spritePlanet->setQueryFlags(1<<0);
				}
				break;

			case LOD_Simple:
				//Load the simplified, low-res planet
				if (!simplePlanet) {
					float maxTerrainHeight = cfgTerrain->findChild("heightRange")->getValueF();
					simplePlanet = new SimplePlanet(radius + maxTerrainHeight * 0.5f, 6, cfgScript->getValue());
					simplePlanet->setRenderQueueGroup(RENDER_QUEUE_OpaquePlanet);
					simplePlanet->setSunLight(universe->getLightSource());

					//simplePlanet->setQueryFlags(1<<0);
				}
				break;

			case LOD_Complex:
				//Load the full-detail planet
				if (!planet) 
				{
					const String planetMediaPath = universe->getMediaPath() + cfgScript->findChild("mediaPath")->getValue();
					ResourceGroupManager::getSingleton().addResourceLocation(planetMediaPath, "FileSystem", "Planet");
					ResourceGroupManager::getSingleton().initialiseResourceGroup("Planet");

					float maxTerrainHeight = cfgTerrain->findChild("heightRange")->getValueF();
					int virtualRes = cfgTerrain->findChild("virtualRes")->getValueI();
					PlanetLoader_Hybrid *loader = new PlanetLoader_Hybrid(radius, maxTerrainHeight, 64);
					loader->setVirtualResolution(virtualRes);
					loader->setLightingEffects(true, false);
					loader->loadPlanet();
					//loader->generateNormalMaps();

					planet = new Planet();
                    planet->setRenderQueueGroup(RENDER_QUEUE_OpaquePlanet);
					planet->setMaxPixelError(20);
					planet->setChunkLoader(loader);
					planet->setSunLight(universe->getLightSource());	

					//planet->setQueryFlags(1<<0);
				}

				break;
		}
	}

	void PlanetProxy::unloadLOD(PlanetLOD lod)
	{
		switch (lod) {
			case LOD_Sprite:
				//Unload the billboard planet
				if (spritePlanet) {
					delete spritePlanet;
					spritePlanet = NULL;
				}
				break;

			case LOD_Simple:
				//Unload the low-res planet
				if (simplePlanet) {
					delete simplePlanet;
					simplePlanet = NULL;
				}
				break;

			case LOD_Complex:
				//Unload the full-detail planet
				if (planet) {
					delete planet->getChunkLoader();
					delete planet;
					ResourceGroupManager::getSingleton().destroyResourceGroup("Planet");
					planet = NULL;
				}
				break;
		}
	}

	bool PlanetProxy::isloadedLOD(PlanetLOD lod)
	{
		switch (lod) {
			case LOD_Invisible:
				return true;
				break;

			case LOD_Sprite:
				return (spritePlanet != NULL);
				break;

			case LOD_Simple:
				return (simplePlanet != NULL);
				break;

			case LOD_Complex:
				if (!planet)
					return false;
				else
					return ((PlanetLoader_Hybrid*)planet->getChunkLoader())->isFinishedLoading();
				break;
		}
		return false;
	}

	void PlanetProxy::setLOD(PlanetLOD lod)
	{
		if (currentLOD != lod)
		{
			currentLOD = lod;

			//Create main scene node if needed
			if (node == NULL && lod != LOD_Invisible)
				node = Core::getSingleton().getSceneManager()->getRootSceneNode()->createChildSceneNode();
 
			//Load/unload atmosphere as needed
			if (lod == LOD_Simple || lod == LOD_Complex) {
				//Load atmosphere
				if (!atmosphere) {
					ConfigNode *cfgAtmosphere = cfgScript->findChild("atmosphere");
					if (cfgAtmosphere) {
						float atmosphereRadius = cfgAtmosphere->findChild("radius")->getValueF();
						atmosphere = new Atmosphere(atmosphereRadius, 80, 40, 20, 1);
						atmosphere->setRenderQueueGroup(RENDER_QUEUE_Atmosphere);
						atmosphere->setSunLight(universe->getLightSource());

						atmosphereSubNode = node->createChildSceneNode();
						atmosphereSubNode->attachObject(atmosphere);
					}
				}
			} else {
				//Unload atmosphere
				if (atmosphere) {
					delete atmosphere;
					atmosphere = NULL;
					Core::getSingleton().getSceneManager()->destroySceneNode(atmosphereSubNode->getName());
					atmosphereSubNode = NULL;
				}
			}

			//Attach the proper LOD to the scene node
			switch (lod) {
				case LOD_Invisible:
					//Delete the scene node if not being used, or remove all objects from it (so the planet is "invisible)
					if (node != NULL) {
						if (planet == NULL && simplePlanet == NULL && spritePlanet == NULL && atmosphere == NULL) {
							assert(atmosphere == NULL && atmosphereSubNode == NULL);
							Core::getSingleton().getSceneManager()->destroySceneNode(node->getName());
							node = NULL;
						} else {
							node->detachAllObjects();
						}
					}
					break;

				case LOD_Sprite:
					node->detachAllObjects();
					node->attachObject(spritePlanet);
					
					break;

				case LOD_Simple:
					node->detachAllObjects();
					node->attachObject(simplePlanet);
					//node->showBoundingBox(true);
					break;

				case LOD_Complex:
					node->detachAllObjects();
					node->attachObject(planet);
					node->_updateBounds();
					//planet->setQueryFlags(1<<0);
					//planet->setVisible(false);
					
					break;
			}

			//node->_updateBounds();
			//node->showBoundingBox(true);
		}

		//node->_updateBounds();
		//node->showBoundingBox(true);
	}

	void PlanetProxy::setNextPosition(const DVector3 pos)
	{
		lastPosition.x += (pos.x - nextPosition.x);
		lastPosition.y += (pos.y - nextPosition.y);
		lastPosition.z += (pos.z - nextPosition.z);
		nextPosition = pos;
	}

	void PlanetProxy::setVelocity(const DVector3 velocity)
	{
		lastPosition.x = nextPosition.x - (velocity.x * universe->getSimStepSize());
		lastPosition.y = nextPosition.y - (velocity.y * universe->getSimStepSize());
		lastPosition.z = nextPosition.z - (velocity.z * universe->getSimStepSize());
	}



	void StarProxy::initialize(float radius, const String &colorMap)
	{
		this->radius = radius;
		starColorMap = colorMap;

		currentLOD = LOD_Invisible;
		node = NULL;
		star = NULL;

		nextPosition = DVector3(0, 0, 0);
		lastPosition = DVector3(0, 0, 0);
		position = DVector3(0, 0, 0);
		mass = 1000.0;
	}

	void StarProxy::destroy()
	{
		unloadLOD(LOD_Complex);
		unloadLOD(LOD_Sprite);
		unloadLOD(LOD_Invisible);

		if (node)
			Core::getSingleton().getSceneManager()->destroySceneNode(node->getName());
	}

	void StarProxy::update()
	{
		if (node) {
			if (static_cast<StarProxy*>(universe->getCurrentOrigin().getTarget()) == this) {
				//If the current floating origin is set to this star, it will be perfectly centered
				node->setPosition(Vector3::ZERO);
				node->setOrientation(Quaternion::IDENTITY);
			} else {
				//Otherwise translate this planet according to the floating origin's position / rotation
				FloatingOrigin &origin = universe->getCurrentOrigin();
				node->setPosition(origin.translateGlobalToLocal(position));
				node->setOrientation(origin.translateGlobalToLocal(Quaternion::IDENTITY));
			}
		}
	}

	void StarProxy::loadLOD(StarLOD lod)
	{
		switch (lod) {
			case LOD_Sprite:
				//Load the 2D, billboard version of the star
				break;

			case LOD_Complex:
				//Load the full-detail star
				if (!star) {
					star = new Star(12, starColorMap);
					star->setRenderQueueGroup(RENDER_QUEUE_Star);
				}
				break;
		}
	}

	void StarProxy::unloadLOD(StarLOD lod)
	{
		switch (lod) {
			case LOD_Sprite:
				//Unload the billboard star
				break;

			case LOD_Complex:
				//Unload the full-detail star
				if (star) {
					delete star;
					star = NULL;
				}
				break;
		}
	}

	bool StarProxy::isloadedLOD(StarLOD lod)
	{
		switch (lod) {
			case LOD_Invisible:
				return true;
				break;

			case LOD_Sprite:
				return false;
				break;

			case LOD_Complex:
				return (star != NULL);
				break;
		}
		return false;
	}

	void StarProxy::setLOD(StarLOD lod)
	{
		if (currentLOD != lod) {
			currentLOD = lod;

			//Create a scene node if needed
			if (lod != LOD_Invisible && node == NULL)
				node = Core::getSingleton().getSceneManager()->getRootSceneNode()->createChildSceneNode();

			//Attach the proper LOD to the scene node
			switch (lod) {
				case LOD_Invisible:
					//Delete the scene node if not being used, or remove all objects from it
					if (node != NULL) {
						if (star == NULL) {
							Core::getSingleton().getSceneManager()->destroySceneNode(node->getName());
							node = NULL;
						} else {
							node->detachAllObjects();
						}
					}
					break;

				case LOD_Sprite:
					node->detachAllObjects();
					break;

				case LOD_Complex:
					node->detachAllObjects();
					node->attachObject(star);
					node->setScale(radius, radius, radius);
					break;
			}
		}
	}

	void StarProxy::setNextPosition(const DVector3 pos)
	{
		lastPosition.x += (pos.x - nextPosition.x);
		lastPosition.y += (pos.y - nextPosition.y);
		lastPosition.z += (pos.z - nextPosition.z);
		nextPosition = pos;
	}

	void StarProxy::setVelocity(const DVector3 velocity)
	{
		lastPosition.x = nextPosition.x - (velocity.x * universe->getSimStepSize());
		lastPosition.y = nextPosition.y - (velocity.y * universe->getSimStepSize());
		lastPosition.z = nextPosition.z - (velocity.z * universe->getSimStepSize());
	}

	GalaxyEngine::DVector3 FloatingOrigin::translateLocalToGlobal(const Vector3 &pos)
	{
		DVector3 globalPos = rotation * pos;
		globalPos.x += position.x; globalPos.y += position.y; globalPos.z += position.z;
		return globalPos;
	}

	Quaternion FloatingOrigin::translateLocalToGlobal(const Quaternion &rot)
	{
		return rotation * rot;
	}

	Vector3 FloatingOrigin::translateGlobalToLocal(const DVector3 &pos)
	{
		Vector3 localPos((pos.x - position.x), (pos.y - position.y), (pos.z - position.z));
		return inverseRotation * localPos;
	}

	Quaternion FloatingOrigin::translateGlobalToLocal(const Quaternion &rot)
	{
		return inverseRotation * rot;
	}

}
