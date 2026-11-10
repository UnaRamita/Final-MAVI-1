#include "Enemy.h"
#include "Art.h"
#include <math.h>

//el orco lanza el proyectil en el cuadro 5 (indice 4) de su animacion de disparo, y la animacion dura 6 cuadros
static const int ORC_SHOT_FRAME = 4;

enemy::enemy()
{
	pos = { 0,0 };
	vel = { 0,0 };
	size = { 0,0 };
	alive = true;
	health = 1;
	hurtTimer = 0;
	animTime = 0;
	deathTime = 0;
	facingLeft = true;
}
enemy::~enemy()
{
}
void enemy::InitBase(Vector2 startPos, Vector2 spriteSize)
{
	pos = startPos;
	size = spriteSize;
	alive = true;
	health = 1;      //por defecto cae de un golpe; cada tipo puede cambiarlo con SetHealth()
	hurtTimer = 0;
	vel = { 0,0 };
	deathTime = 0;
	facingLeft = true;
	//fase inicial distinta segun la posicion: varios enemigos iguales no se mueven al unisono
	animTime = fmodf(fabsf(startPos.x) * 0.37f + fabsf(startPos.y) * 0.11f, 1.0f);
}
void enemy::AdvanceAnim(float dt)
{
	animTime += dt;
	if (!alive) deathTime += dt;
	if (hurtTimer > 0) hurtTimer -= dt;
}
void enemy::SetHealth(int value)
{
	health = value;
}
//segmento (a-b) contra rectangulo (recorte de Liang-Barsky): true si lo cruza o termina adentro
static bool SegmentHitsRect(Vector2 a, Vector2 b, Rectangle r)
{
	float dx = b.x - a.x;
	float dy = b.y - a.y;
	float p[4] = { -dx, dx, -dy, dy };
	float q[4] = { a.x - r.x, r.x + r.width - a.x, a.y - r.y, r.y + r.height - a.y };
	float t0 = 0.0f, t1 = 1.0f;

	for (int i = 0; i < 4; i++)
	{
		if (p[i] == 0.0f)
		{
			if (q[i] < 0.0f) return false; //paralelo y por fuera de ese borde
		}
		else
		{
			float t = q[i] / p[i];
			if (p[i] < 0.0f) { if (t > t1) return false; if (t > t0) t0 = t; }
			else { if (t < t0) return false; if (t < t1) t1 = t; }
		}
	}
	return true;
}
bool enemy::CanSeePlayer(const enemyWorld& world)
{
	if (world.walls == nullptr) return true;

	Vector2 eyes = { pos.x + size.x / 2, pos.y + size.y / 2 };
	for (unsigned int i = 0; i < world.walls->size(); i++)
	{
		if (SegmentHitsRect(eyes, world.playerCenter, (*world.walls)[i])) return false;
	}
	return true;
}
int enemy::ResolveWalls(const enemyWorld& world)
{
	if (world.walls == nullptr) return 0;

	int hit = 0;
	for (unsigned int i = 0; i < world.walls->size(); i++)
	{
		const Rectangle& w = (*world.walls)[i];
		if (!CheckCollisionRecs(GetHitbox(), w)) continue;

		if (vel.x > 0) { pos.x = w.x - size.x; hit = 1; }
		else if (vel.x < 0) { pos.x = w.x + w.width; hit = -1; }
	}
	return hit;
}
Color enemy::HurtTint()
{
	//parpadeo rojizo al recibir un golpe que no lo mata
	return (hurtTimer > 0) ? Color{ 255, 110, 110, 255 } : WHITE;
}
bool enemy::TakeHit(int damage)
{
	if (!alive) return false;

	health -= damage;
	if (health <= 0)
	{
		Kill();
		return true;
	}
	hurtTimer = 0.18f;
	return false;
}
void enemy::Draw()
{
	if (alive) DrawAlive();
	else DrawDeath();
}
void enemy::DrawDeath()
{
	//humo comun: 6 cuadros de 48x48 centrados en el enemigo, a una escala acorde a su tamaño
	const animSheet& a = Art::puff;
	float duration = a.frames * a.frameTime;
	if (deathTime >= duration) return;

	int frame = (int)(deathTime / a.frameTime);
	float biggest = (size.x > size.y) ? size.x : size.y;
	Vector2 center = { pos.x + size.x / 2, pos.y + size.y / 2 };
	Art::DrawFrame(a, frame, center, biggest / 32.0f, false, true, WHITE);
}
Rectangle enemy::GetHitbox()
{
	return { pos.x, pos.y, size.x, size.y };
}
bool enemy::IsAlive()
{
	return alive;
}
void enemy::Kill()
{
	if (!alive) return;
	alive = false;
	deathTime = 0;
}
void enemy::Translate(Vector2 delta)
{
	pos.x += delta.x;
	pos.y += delta.y;
}
bool enemy::TryConsumeShot(Vector2& outPos, bool& outFacingLeft)
{
	(void)outPos;
	(void)outFacingLeft;
	return false;
}

//---------------- enemyLinear (MRU) ----------------
void enemyLinear::Init(Vector2 startPos, Vector2 spriteSize, float minXVal, float maxXVal, float speedVal)
{
	InitBase(startPos, spriteSize);
	minX = minXVal;
	maxX = maxXVal;
	speed = speedVal;
	vel = { speed, 0 };
	SetHealth(2); //slime: aguanta 2 golpes comunes, 1 golpe fuerte de espada o 2 proyectiles magicos
}
void enemyLinear::Update(float dt, const enemyWorld& world)
{
	if (!alive) return;

	//MRU: posicion = posicion + velocidad * tiempo, con velocidad constante
	pos.x += vel.x * dt;

	//si patrullando se topa con una pared (puerta cerrada) no la atraviesa: se da la vuelta
	int wallHit = ResolveWalls(world);
	if (wallHit != 0) vel.x = (wallHit > 0) ? -speed : speed;

	if (pos.x <= minX) { pos.x = minX; vel.x = speed; }
	if (pos.x + size.x >= maxX) { pos.x = maxX - size.x; vel.x = -speed; }
}
void enemyLinear::DrawAlive()
{
	//slime: la hoja es de 40x34 (el sprite original de 32x32 mas margen para el salto), dibujada al doble
	Vector2 anchor = { pos.x + size.x / 2, pos.y + size.y };
	Art::DrawFrame(Art::slimeHop, Art::LoopFrame(Art::slimeHop, animTime), anchor, 2.0f, false, false, HurtTint());
}
void enemyLinear::DrawDeath()
{
	//slime: se aplasta y salpica en vez de la nube de humo
	const animSheet& a = Art::slimeDeath;
	if (deathTime >= a.frames * a.frameTime) return;

	Vector2 anchor = { pos.x + size.x / 2, pos.y + size.y };
	Art::DrawFrame(a, (int)(deathTime / a.frameTime), anchor, 2.0f, false, false, WHITE);
}

//---------------- enemyAccelerated (MRUA) ----------------
void enemyAccelerated::Init(Vector2 startPos, Vector2 spriteSize, float accelVal, float maxSpeedVal)
{
	InitBase(startPos, spriteSize);
	//limites reales del mapa (no hardcodeados): si el mapa cambia de ancho, esto se ajusta solo
	minX = 0.0f;
	maxX = (float)GetScreenWidth() - spriteSize.x;
	accel = accelVal;
	maxSpeed = maxSpeedVal;
	vel = { 0,0 };
}
void enemyAccelerated::Update(float dt, const enemyWorld& world)
{
	if (!alive) return;

	Vector2 playerPos = world.playerPos;

	if (CanSeePlayer(world))
	{
		//MRUA: velocidad = velocidad + aceleracion * tiempo, acelerando hacia el jugador
		float dir = (playerPos.x < pos.x) ? -1.0f : 1.0f;
		vel.x += dir * accel * dt;
	}
	else
	{
		//no lo ve (hay una pared en el medio): deja de perseguirlo y frena hasta quedarse quieto
		float brake = accel * dt;
		if (fabsf(vel.x) <= brake) vel.x = 0;
		else vel.x -= (vel.x > 0 ? brake : -brake);
	}

	if (vel.x > maxSpeed) vel.x = maxSpeed;
	if (vel.x < -maxSpeed) vel.x = -maxSpeed;

	pos.x += vel.x * dt;

	//no atraviesa las paredes: se frena contra ellas
	if (ResolveWalls(world) != 0) vel.x = 0;

	if (pos.x < minX) { pos.x = minX; vel.x = 0; }
	if (pos.x + size.x > maxX) { pos.x = maxX - size.x; vel.x = 0; }

	//mira hacia donde corre; si casi esta quieto, mira al jugador
	if (vel.x < -8.0f) facingLeft = true;
	else if (vel.x > 8.0f) facingLeft = false;
	else facingLeft = (playerPos.x < pos.x);
}
void enemyAccelerated::DrawAlive()
{
	//perro: hojas de 26x34 dibujadas a 2.5x; el sprite original mira a la izquierda (se espeja para ir a la derecha)
	bool running = fabsf(vel.x) > 20.0f;
	const animSheet& a = running ? Art::dogRun : Art::dogIdle;
	Vector2 anchor = { pos.x + size.x / 2, pos.y + size.y };
	Art::DrawFrame(a, Art::LoopFrame(a, animTime), anchor, 2.5f, !facingLeft, false, HurtTint());
}

//---------------- enemyCircular (MCU) ----------------
void enemyCircular::Init(Vector2 centerPos, Vector2 spriteSize, float radiusVal, float angularSpeedVal)
{
	InitBase(centerPos, spriteSize);
	center = centerPos;
	radius = radiusVal;
	angularSpeed = angularSpeedVal;
	angle = 0;
}
void enemyCircular::Update(float dt, const enemyWorld& world)
{
	(void)world; //la abeja vuela en una orbita fija: no depende del jugador ni de las paredes
	if (!alive) return;

	//MCU: angulo = angulo + velocidad angular * tiempo; posicion sobre la circunferencia
	angle += angularSpeed * dt;
	pos.x = center.x + cosf(angle) * radius - size.x / 2;
	pos.y = center.y + sinf(angle) * radius - size.y / 2;
}
void enemyCircular::DrawAlive()
{
	//abeja: hoja de 22x22 (sprite original de 18x18 mas margen para las alas), dibujada a 3x y centrada
	Vector2 center2 = { pos.x + size.x / 2, pos.y + size.y / 2 };
	Art::DrawFrame(Art::beeFly, Art::LoopFrame(Art::beeFly, animTime), center2, 3.0f, false, true, HurtTint());
}

//---------------- enemyRanged ----------------
void enemyRanged::Init(Vector2 startPos, Vector2 spriteSize, float minXVal, float maxXVal, float speedVal, float shootRangeVal, float shootCooldownVal)
{
	InitBase(startPos, spriteSize);
	minX = minXVal;
	maxX = maxXVal;
	speed = speedVal;
	shootRange = shootRangeVal;
	shootCooldown = shootCooldownVal;
	shootTimer = 0;
	casting = false;
	castTime = 0;
	shotFired = false;
	wantsShot = false;
	shotFacingLeft = false;
	vel = { speed, 0 };
}
void enemyRanged::Update(float dt, const enemyWorld& world)
{
	if (!alive) return;

	Vector2 playerPos = world.playerPos;
	float dx = playerPos.x - pos.x;
	float dy = playerPos.y - pos.y;
	float dist = sqrtf(dx * dx + dy * dy);

	if (casting)
	{
		//disparando: se queda quieto mirando al jugador; el proyectil sale en el cuadro del lanzamiento
		castTime += dt;
		if (!shotFired && castTime >= ORC_SHOT_FRAME * Art::orcCast.frameTime)
		{
			wantsShot = true;
			shotFired = true;
		}
		if (castTime >= Art::orcCast.frames * Art::orcCast.frameTime) casting = false;
		return;
	}

	//MRU: patrulla lento mientras vigila
	pos.x += vel.x * dt;
	int wallHit = ResolveWalls(world); //tampoco atraviesa paredes patrullando: se da la vuelta
	if (wallHit != 0) vel.x = (wallHit > 0) ? -speed : speed;
	if (pos.x <= minX) { pos.x = minX; vel.x = speed; }
	if (pos.x + size.x >= maxX) { pos.x = maxX - size.x; vel.x = -speed; }
	facingLeft = (vel.x < 0);

	if (shootTimer > 0) shootTimer -= dt;

	//solo dispara si ve al jugador: con una pared en el medio no lo "ve" y no gasta el disparo
	if (shootTimer <= 0 && dist <= shootRange && CanSeePlayer(world))
	{
		casting = true;
		castTime = 0;
		shotFired = false;
		shotFacingLeft = (dx < 0);
		facingLeft = shotFacingLeft;
		shootTimer = shootCooldown;
	}
}
bool enemyRanged::TryConsumeShot(Vector2& outPos, bool& outFacingLeft)
{
	if (!wantsShot) return false;

	wantsShot = false;

	//el proyectil nace en la mano del orco (la del garrote): 40 px al costado del centro, a la altura del pecho.
	//outPos es la esquina de un cuadrado de 32 (Level le suma 16 para tener el centro)
	float muzzleX = pos.x + size.x / 2 + (shotFacingLeft ? -40.0f : 40.0f);
	float muzzleY = pos.y + 38.0f;
	outPos = { muzzleX - 16, muzzleY - 16 };
	outFacingLeft = shotFacingLeft;
	return true;
}
void enemyRanged::DrawAlive()
{
	//orco: hojas de 48x60 dibujadas a 2x; el sprite original mira a la izquierda (se espeja para mirar a la derecha)
	Vector2 anchor = { pos.x + size.x / 2, pos.y + size.y };
	if (casting)
	{
		int frame = (int)(castTime / Art::orcCast.frameTime);
		Art::DrawFrame(Art::orcCast, frame, anchor, 2.0f, !facingLeft, false, HurtTint());
	}
	else
	{
		Art::DrawFrame(Art::orcWalk, Art::LoopFrame(Art::orcWalk, animTime), anchor, 2.0f, !facingLeft, false, HurtTint());
	}
}
