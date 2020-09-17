#include "EngineIncludes.h"

S2DPhysics::S2DPhysics()
{
	b2Vec2 gravity(Gravity.x, Gravity.y);
	b2World world(gravity);
}