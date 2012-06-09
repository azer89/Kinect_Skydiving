

#ifndef __CollisionDetector_h_
#define __CollisionDetector_h_

#include "Stdafx.h"
#include "Character.h"
#include "TargetCircles.h"

class CollisionDetector
{
public:
	CollisionDetector(void);
	virtual ~CollisionDetector(void);

	void initCollisionDetector(Character* character, TargetCircles* tCircles);

private:
	Character* character;
	TargetCircles* tCircles;
};

#endif // #ifndef __CollisionDetector_h_

