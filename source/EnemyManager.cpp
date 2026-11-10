#include "EnemyManager.h"
#include "Collisions.h"
#include "Art.h"

enemyManager::enemyManager()
{
}
enemyManager::~enemyManager()
{
	for (unsigned int i = 0; i < enemies.size(); i++)
	{
		delete enemies[i];
	}
}
void enemyManager::Init()
{
	//las animaciones de los enemigos viven en Art (se cargan una sola vez para todo el juego)
	Art::Load();
}
void enemyManager::Clear()
{
	for (unsigned int i = 0; i < enemies.size(); i++)
	{
		delete enemies[i];
	}
	enemies.clear();
}
void enemyManager::AddLinear(Vector2 pos, float minX, float maxX, float speed)
{
	//enemigo de movimiento lineal (MRU): patrulla entre dos puntos
	enemyLinear* e = new enemyLinear();
	e->Init(pos, { 64,64 }, minX, maxX, speed);
	enemies.push_back(e);
}
void enemyManager::AddAccelerated(Vector2 pos, float accel, float maxSpeed)
{
	//enemigo de movimiento acelerado (MRUA): persigue al jugador cuando esta cerca.
	//sus limites de movimiento son los bordes reales del mapa (ver enemyAccelerated::Init)
	enemyAccelerated* e = new enemyAccelerated();
	e->Init(pos, { 45,80 }, accel, maxSpeed);
	enemies.push_back(e);
}
void enemyManager::AddCircular(Vector2 center, float radius, float angularSpeed)
{
	//enemigo de movimiento circular (MCU): orbita un punto fijo
	enemyCircular* e = new enemyCircular();
	e->Init(center, { 54,54 }, radius, angularSpeed);
	enemies.push_back(e);
}
void enemyManager::AddRanged(Vector2 pos, float minX, float maxX, float speed, float shootRange, float shootCooldown)
{
	//enemigo a distancia: patrulla lento y dispara hacia el jugador en rango
	enemyRanged* e = new enemyRanged();
	e->Init(pos, { 64,96 }, minX, maxX, speed, shootRange, shootCooldown);
	enemies.push_back(e);
}
void enemyManager::Update(float dt, const enemyWorld& world)
{
	for (unsigned int i = 0; i < enemies.size(); i++)
	{
		enemies[i]->Update(dt, world);
		enemies[i]->AdvanceAnim(dt); //tambien avanza la animacion de muerte de los que ya no estan vivos
	}
}
void enemyManager::Draw(Rectangle viewBounds)
{
	//optimizacion: solo se dibujan los enemigos visibles dentro de la pantalla
	for (unsigned int i = 0; i < enemies.size(); i++)
	{
		if (CheckCollisionRecs(enemies[i]->GetHitbox(), viewBounds))
		{
			enemies[i]->Draw();
		}
	}
}
bool enemyManager::CheckPlayerCollision(Rectangle playerHitbox)
{
	for (unsigned int i = 0; i < enemies.size(); i++)
	{
		if (enemies[i]->IsAlive() && Collisions::CheckAABB(playerHitbox, enemies[i]->GetHitbox()))
		{
			return true;
		}
	}
	return false;
}
int enemyManager::AttackRect(Rectangle attackHitbox, int damage, int* outHits)
{
	int killed = 0;
	int hits = 0;
	for (unsigned int i = 0; i < enemies.size(); i++)
	{
		if (enemies[i]->IsAlive() && Collisions::CheckAABB(attackHitbox, enemies[i]->GetHitbox()))
		{
			hits++;
			if (enemies[i]->TakeHit(damage)) killed++;
		}
	}
	if (outHits) *outHits = hits;
	return killed;
}
int enemyManager::CountAlive()
{
	int count = 0;
	for (unsigned int i = 0; i < enemies.size(); i++)
	{
		if (enemies[i]->IsAlive()) count++;
	}
	return count;
}
int enemyManager::CountAliveInRect(Rectangle area)
{
	int count = 0;
	for (unsigned int i = 0; i < enemies.size(); i++)
	{
		if (enemies[i]->IsAlive() && Collisions::CheckAABB(area, enemies[i]->GetHitbox())) count++;
	}
	return count;
}
bool enemyManager::CollectShot(Vector2& outPos, bool& outFacingLeft)
{
	for (unsigned int i = 0; i < enemies.size(); i++)
	{
		if (enemies[i]->IsAlive() && enemies[i]->TryConsumeShot(outPos, outFacingLeft))
		{
			return true;
		}
	}
	return false;
}
void enemyManager::CarryRiders(Rectangle beforePlatformRect, Vector2 delta)
{
	for (unsigned int i = 0; i < enemies.size(); i++)
	{
		if (enemies[i]->IsAlive() && Collisions::IsRestingOn(enemies[i]->GetHitbox(), beforePlatformRect))
		{
			enemies[i]->Translate(delta);
		}
	}
}
void enemyManager::Unload()
{
	Clear();
	//las texturas las libera Art::Unload() (Level::Unload)
}
