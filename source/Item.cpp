#include "Item.h"
#include "Collisions.h"
#include "Art.h"
#include <math.h>

item::item()
{
	pos = { 0,0 };
	radius = 10;
	collected = false;
	bobTimer = 0;
}
item::~item()
{
}
void item::Init(Vector2 position, float r)
{
	pos = position;
	radius = r;
	collected = false;
	bobTimer = 0;
}
bool item::Update(float dt, Rectangle playerHitbox)
{
	if (collected) return false;

	bobTimer += dt;

	if (Collisions::CheckCircleRect(pos, radius, playerHitbox))
	{
		collected = true;
		return true;
	}
	return false;
}
void item::Draw()
{
	if (collected) return;

	//pequeno movimiento vertical para dar vida al sprite
	float offset = sinf(bobTimer * 3.0f) * 5.0f;

	//moneda girando (6 cuadros de 28x28), escalada al diametro de la moneda
	//cada moneda gira con su propia fase (usa el tiempo desde que arranco el nivel + su posicion)
	int frame = ((int)((bobTimer + pos.x * 0.002f) / 0.09f)) % 6;
	Rectangle src = { (float)(frame * 28), 0, 28, 28 };
	Rectangle dst = { pos.x - radius, pos.y + offset - radius, radius * 2, radius * 2 };
	DrawTexturePro(Art::coinSheet, src, dst, { 0,0 }, 0, WHITE);
}
bool item::IsCollected()
{
	return collected;
}
void item::Reset()
{
	collected = false;
	bobTimer = 0;
}
