#pragma once
#include "raylib.h"

//plataforma que se mueve en linea recta (MRU) entre dos puntos, en cualquier direccion
//(horizontal, vertical o diagonal). Quien esta parado encima (jugador o enemigo) se
//arrastra sumandole el mismo desplazamiento que tuvo la plataforma en el frame (ver GetDelta()).
class movingPlatform
{
public:
	movingPlatform();
	~movingPlatform();

	void Init(Rectangle startBounds, Vector2 endOffset, float speed);
	void Update(float dt);
	void Draw();

	Rectangle GetHitbox();
	Vector2 GetDelta(); //cuanto se desplazo en el ultimo Update()

private:
	Vector2 startPos;
	Vector2 endPos;
	Vector2 pos;
	Vector2 size;
	Vector2 delta;
	float speed;
	bool goingToEnd;
};
