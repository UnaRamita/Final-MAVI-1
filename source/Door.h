#pragma once
#include "raylib.h"

//puerta (rastrillo de castillo) que bloquea el paso hasta que se activa el boton que la desbloquea
class door
{
public:
	door();
	~door();

	void Init(Rectangle bounds);
	void Unlock();
	void Reset();
	void Update(float dt); //avanza la animacion de apertura (la reja sube hacia el dintel)

	bool IsLocked();
	Rectangle GetHitbox();
	void Draw();

private:
	Rectangle bounds;
	bool locked;
	float openTimer; //segundos desde que se desbloqueo; define el cuadro de la animacion
};
