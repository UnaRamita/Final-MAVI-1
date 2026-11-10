#pragma once
#include "raylib.h"

//boton de presion en el suelo: al tocarlo desbloquea su puerta asociada
class button
{
public:
	button();
	~button();

	void Init(Rectangle bounds, int linkedDoorIndex);
	bool CheckPressed(Rectangle playerHitbox); //true si se activo en este frame
	void Reset();

	int GetLinkedDoorIndex();
	bool IsPressed(); //estado persistente (a diferencia de CheckPressed, no consume nada)
	void Draw();

private:
	Rectangle bounds;
	bool pressed;
	int linkedDoorIndex;
};
