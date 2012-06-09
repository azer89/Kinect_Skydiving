
#include "Stdafx.h"
#include "CollisionDetector.h"

CollisionDetector::CollisionDetector(void)
{
}

CollisionDetector::~CollisionDetector(void)
{
}

void CollisionDetector::initCollisionDetector(Character* character, TargetCircles* tCircles)
{
	this->character = character;
	this->tCircles = tCircles;
}