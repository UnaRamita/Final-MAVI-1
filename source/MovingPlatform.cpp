#include "MovingPlatform.h"
#include "Art.h"
#include <math.h>

movingPlatform::movingPlatform()
{
	startPos = { 0,0 };
	endPos = { 0,0 };
	pos = { 0,0 };
	size = { 0,0 };
	delta = { 0,0 };
	speed = 0;
	goingToEnd = true;
}
movingPlatform::~movingPlatform()
{
}
void movingPlatform::Init(Rectangle startBounds, Vector2 endOffset, float speedVal)
{
	startPos = { startBounds.x, startBounds.y };
	endPos = { startBounds.x + endOffset.x, startBounds.y + endOffset.y };
	pos = startPos;
	size = { startBounds.width, startBounds.height };
	speed = speedVal;
	goingToEnd = true;
	delta = { 0,0 };
}
void movingPlatform::Update(float dt)
{
	//MRU: se mueve en linea recta a velocidad constante entre startPos y endPos, y vuelve
	Vector2 target = goingToEnd ? endPos : startPos;
	Vector2 prevPos = pos;

	float dx = target.x - pos.x;
	float dy = target.y - pos.y;
	float dist = sqrtf(dx * dx + dy * dy);

	if (dist <= speed * dt)
	{
		pos = target;
		goingToEnd = !goingToEnd;
	}
	else
	{
		pos.x += (dx / dist) * speed * dt;
		pos.y += (dy / dist) * speed * dt;
	}

	delta = { pos.x - prevPos.x, pos.y - prevPos.y };
}
void movingPlatform::Draw()
{
	Art::DrawMovingPlatform(GetHitbox());
}
Rectangle movingPlatform::GetHitbox()
{
	return { pos.x, pos.y, size.x, size.y };
}
Vector2 movingPlatform::GetDelta()
{
	return delta;
}
