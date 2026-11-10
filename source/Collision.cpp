#include "Collisions.h"
#include <math.h>

bool Collisions::CheckAABB(Rectangle a, Rectangle b)
{
	return (a.x < b.x + b.width && a.x + a.width > b.x &&
		a.y < b.y + b.height && a.y + a.height > b.y);
}

bool Collisions::CheckCircle(Vector2 centerA, float radiusA, Vector2 centerB, float radiusB)
{
	float dx = centerB.x - centerA.x;
	float dy = centerB.y - centerA.y;
	float distSq = dx * dx + dy * dy;
	float radiusSum = radiusA + radiusB;
	return distSq <= (radiusSum * radiusSum);
}

bool Collisions::CheckCircleRect(Vector2 center, float radius, Rectangle rect)
{
	//punto del rectangulo mas cercano al centro del circulo
	float closestX = center.x;
	if (closestX < rect.x) closestX = rect.x;
	if (closestX > rect.x + rect.width) closestX = rect.x + rect.width;

	float closestY = center.y;
	if (closestY < rect.y) closestY = rect.y;
	if (closestY > rect.y + rect.height) closestY = rect.y + rect.height;

	float dx = center.x - closestX;
	float dy = center.y - closestY;

	return (dx * dx + dy * dy) <= (radius * radius);
}

bool Collisions::IsRestingOn(Rectangle entity, Rectangle surface)
{
	bool horizontallyOverlapping = entity.x < surface.x + surface.width && entity.x + entity.width > surface.x;
	float entityBottom = entity.y + entity.height;
	bool verticallyResting = fabsf(entityBottom - surface.y) <= 4.0f; //pequeno margen de tolerancia

	return horizontallyOverlapping && verticallyResting;
}
