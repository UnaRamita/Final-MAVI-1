#pragma once
#include <vector>
#include "raylib.h"
#include "Enemy.h"

//administra la lista de enemigos del nivel actual. Las texturas se cargan una sola vez
//en Init(); cada nivel arma su propio elenco llamando Clear() + los metodos Add*.
class enemyManager
{
public:
	enemyManager();
	~enemyManager();

	void Init();
	void Clear();

	void AddLinear(Vector2 pos, float minX, float maxX, float speed);
	void AddAccelerated(Vector2 pos, float accel, float maxSpeed); //sus limites son los bordes del mapa, no valores manuales
	void AddCircular(Vector2 center, float radius, float angularSpeed);
	void AddRanged(Vector2 pos, float minX, float maxX, float speed, float shootRange, float shootCooldown);

	void Update(float dt, const enemyWorld& world);
	void Draw(Rectangle viewBounds);
	bool CheckPlayerCollision(Rectangle playerHitbox);
	//le saca "damage" de vida a los enemigos vivos que tocan el rectangulo; devuelve cuantos murieron.
	//outHits (opcional) recibe cuantos fueron tocados, hayan muerto o no (para el sonido de golpe)
	int AttackRect(Rectangle attackHitbox, int damage, int* outHits = nullptr);
	int CountAlive();
	int CountAliveInRect(Rectangle area); //enemigos vivos cuyo centro cae dentro del area (para arenas/objetivos parciales)
	bool CollectShot(Vector2& outPos, bool& outFacingLeft); //true si algun enemigo a distancia quiere disparar este frame
	void CarryRiders(Rectangle beforePlatformRect, Vector2 delta); //arrastra a los enemigos parados sobre una plataforma movil

	void Unload();

private:
	std::vector<enemy*> enemies;
};
