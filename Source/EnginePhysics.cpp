#include "EngineIncludes.h"

#if 0

namespace Physics
{
	std::vector<b2Body*> bodies;

	b2Vec2 b2Gravity;

	b2World* b2PhysWorld;
}

S2DPhysics::S2DPhysics()
{
	Physics::b2Gravity = b2Vec2(0, -10.0f);

	Physics::b2PhysWorld = new b2World(Physics::b2Gravity);
}

S2DPhysics::S2DPhysics(Vec2 gravity)
{
	Physics::b2Gravity = b2Vec2(gravity.x, gravity.y);

	Physics::b2PhysWorld = new b2World(Physics::b2Gravity);
}

S2DPhysics::~S2DPhysics()
{
	delete Physics::b2PhysWorld;
}

void S2DPhysics::DeleteBox(uintptr_t pointer)
{
	b2Body* b = (b2Body*)pointer;

	Physics::b2PhysWorld->DestroyBody(b);
}

uintptr_t S2DPhysics::CreateStaticBox(Vec2 position, Vec2 size, Vec2 center, float angle)
{
	b2BodyDef bodyDef;
	bodyDef.position.Set(position.x, position.y);

	b2Body* body = Physics::b2PhysWorld->CreateBody(&bodyDef);

	b2PolygonShape shape;

	shape.SetAsBox(size.x / 2, size.y / 2, b2Vec2(size.x * center.x, size.y * center.y), S2D_DEGREES_TO_RADIANS(angle));

	body->CreateFixture(&shape, 0.0f);

	return reinterpret_cast<uintptr_t>(body);
}

uintptr_t S2DPhysics::CreateDynamicBox(Vec2 position, Vec2 size, Vec2 center, float angle)
{
	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(position.x, position.y);

	b2Body* body = Physics::b2PhysWorld->CreateBody(&bodyDef);

	b2PolygonShape shape;

	shape.SetAsBox(size.x / 2, size.y / 2, b2Vec2(size.x * center.x, size.y * center.y), S2D_DEGREES_TO_RADIANS(angle));

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &shape;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.3f;

	body->CreateFixture(&fixtureDef);

	return reinterpret_cast<uintptr_t>(body);
}

Vec2 S2DPhysics::GetBoxPos(uintptr_t pointer)
{
	auto vec = reinterpret_cast<b2Body*>(pointer)->GetPosition();

	return Vec2(vec.x, vec.y);
}

float S2DPhysics::GetBoxAngle(uintptr_t pointer)
{
	return reinterpret_cast<b2Body*>(pointer)->GetAngle();
}

void S2DPhysics::Update(float timeStep)
{
	Physics::b2PhysWorld->Step(timeStep, VelocityIterations, PositionIterations);
}

#endif