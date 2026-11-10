#pragma once
#include "raylib.h"

//objeto recolectable (moneda/gema) del nivel
class item
{
public:
	item();
	~item();

	void Init(Vector2 position, float radius);
	bool Update(float dt, Rectangle playerHitbox); //devuelve true si fue recolectado en este frame
	void Draw();
	bool IsCollected();
	void Reset();

private:
	Vector2 pos;
	float radius;
	bool collected;
	float bobTimer;
};
