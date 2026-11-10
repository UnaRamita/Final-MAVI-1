#pragma once
#include "raylib.h"

//peligro ambiental (picos, lava, trampa): daña al jugador al tocarlo.
//si togglePeriod > 0, se prende/apaga solo (trampa temporizada); si es 0, esta siempre activo.
class hazard
{
public:
	hazard();
	~hazard();

	void Init(Rectangle bounds, float togglePeriod);
	void Update(float dt);
	void Draw();
	void Reset();

	bool CheckDamage(Rectangle playerHitbox); //true si el jugador toca el peligro activo

private:
	Rectangle bounds;
	bool active;
	float togglePeriod;
	float timer;
};
