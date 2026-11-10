#pragma once
#include "raylib.h"

//funciones de colision reutilizables por todas las clases del juego
namespace Collisions
{
	bool CheckAABB(Rectangle a, Rectangle b);
	bool CheckCircle(Vector2 centerA, float radiusA, Vector2 centerB, float radiusB);
	bool CheckCircleRect(Vector2 center, float radius, Rectangle rect);
	bool IsRestingOn(Rectangle entity, Rectangle surface); //true si entity esta parado justo encima de surface
}
