#include "Player.h"
#include "Collisions.h"

//segundos que hay que mantener F para disparar el golpe fuerte de la espada
static const float HOLD_HEAVY_TIME = 0.15f;

player::player()
{
	attackHold = 0;
	sheet = { 0 };
	currentFrame = 0;
	frameTimer = 0;
	facingLeft = false;
	moving = false;

	pos = { 0,0 };
	vel = { 0,0 };
	moveSpeed = 350;
	gravity = 2200;
	//altura del salto = v^2 / (2*gravedad) (~236 px reales con la integracion por frame). Antes (-950) llegaba a ~197 px
	//y las plataformas que estan 200 px mas arriba (niveles 4, 8 y 9) quedaban fuera de alcance
	jumpForce = -1040;
	grounded = false;

	size = { 64,128 };
	spawnPoint = { 0,0 };

	lives = 3;
	invulnTimer = 0;
}
player::~player()
{
}
void player::Init()
{
	sheet = LoadTexture("Assets/Sprites/Character/PlayerSheet.png");

	equippedWeapon.Init();

	lives = 3;
	//el punto de spawn lo define cada nivel con SetSpawn() antes del primer FullReset()
}
void player::HandleInput()
{
	moving = false;

	if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
	{
		vel.x = moveSpeed;
		facingLeft = false;
		moving = true;
	}
	else if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
	{
		vel.x = -moveSpeed;
		facingLeft = true;
		moving = true;
	}
	else
	{
		vel.x = 0;
	}

	if (grounded && (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)))
	{
		vel.y = jumpForce;
		grounded = false;
	}

	//salto variable: manteniendo la tecla se llega a la altura maxima; soltandola mientras se sube, el ascenso se corta
	bool jumpHeld = IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_UP) || IsKeyDown(KEY_W);
	CutJumpIfReleased(jumpHeld);

	//F ataca; con la espada, apretarlo de nuevo durante/justo despues de un golpe encadena el siguiente del combo
	if (IsKeyPressed(KEY_F))
	{
		equippedWeapon.TryAttack();
	}

	//mantener F con la espada: pasa directo al 3er golpe (el fuerte, de 2 de daño). El umbral es menor que lo que
	//tarda el 1er golpe en conectar (~0.17 s), asi que el tajo comun se reemplaza antes de pegar. Si se sigue
	//manteniendo, repite el golpe fuerte cada vez que termina su pausa.
	if (IsKeyDown(KEY_F))
	{
		attackHold += GetFrameTime();
		if (attackHold >= HOLD_HEAVY_TIME) equippedWeapon.TryHeavyAttack();
	}
	else
	{
		attackHold = 0;
	}
}
void player::CutJumpIfReleased(bool jumpHeld)
{
	//Si la tecla de salto ya no esta apretada y el jugador todavia sube mas rapido que jumpCutSpeed, la velocidad de
	//subida se limita a ese valor: el resto del ascenso es solo el que le queda por inercia. Asi la altura final
	//depende de cuanto se mantuvo apretada la tecla (no hay carga: el salto sale igual al apretar).
	//Con jumpCutSpeed = -500 un toque rapido da ~57 px (v^2 / 2g) y mantenerla da los ~237 px completos.
	//Los empujones hacia arriba que no son salto (rebote al recibir daño, -500) no se ven afectados.
	const float jumpCutSpeed = -500.0f;
	if (!jumpHeld && vel.y < jumpCutSpeed)
	{
		vel.y = jumpCutSpeed;
	}
}
void player::MoveX(float dt, Rectangle* solids, int solidCount, float worldRight)
{
	//MRU: la posicion avanza en linea recta segun la velocidad constante del frame
	pos.x += vel.x * dt;

	if (pos.x < 0) pos.x = 0;
	//worldRight normalmente es el borde de la pantalla; el nivel lo agranda cuando la
	//puerta de salida esta desbloqueada, para permitir caminar fuera de la pantalla por ahi
	if (pos.x + size.x > worldRight) pos.x = worldRight - size.x;

	for (int i = 0; i < solidCount; i++)
	{
		Rectangle hitbox = GetHitbox();
		if (Collisions::CheckAABB(hitbox, solids[i]))
		{
			if (vel.x > 0) pos.x = solids[i].x - size.x;
			else if (vel.x < 0) pos.x = solids[i].x + solids[i].width;
		}
	}
}
void player::MoveY(float dt, Rectangle* solids, int solidCount, Rectangle* oneWay, int oneWayCount)
{
	float prevBottom = pos.y + size.y; //donde estaban los pies antes de moverse
	pos.y += vel.y * dt;
	grounded = false;

	for (int i = 0; i < solidCount; i++)
	{
		Rectangle hitbox = GetHitbox();
		if (Collisions::CheckAABB(hitbox, solids[i]))
		{
			if (vel.y > 0)
			{
				pos.y = solids[i].y - size.y;
				vel.y = 0;
				grounded = true;
			}
			else if (vel.y < 0)
			{
				pos.y = solids[i].y + solids[i].height;
				vel.y = 0;
			}
		}
	}

	//plataformas de un solo sentido: solo frenan al jugador cuando CAE sobre ellas y sus pies venian por
	//encima del borde superior. Si sube desde abajo (o de costado) la atraviesa, asi no se golpea la cabeza.
	if (vel.y >= 0)
	{
		for (int i = 0; i < oneWayCount; i++)
		{
			Rectangle hitbox = GetHitbox();
			if (Collisions::CheckAABB(hitbox, oneWay[i]) && prevBottom <= oneWay[i].y + 2.0f)
			{
				pos.y = oneWay[i].y - size.y;
				vel.y = 0;
				grounded = true;
			}
		}
	}
}
void player::UpdateAnimation(float dt)
{
	//el estado sale de lo que esta haciendo el jugador: aire (salto/caida), caminar o reposo
	const int FRAME_IDLE = 0, FRAME_WALK = 2, FRAME_JUMP_UP = 8, FRAME_JUMP_PEAK = 9, FRAME_FALL = 10;
	const int WALK_FRAMES = 6;

	if (!grounded)
	{
		//segun la velocidad vertical: subiendo, en el pico (casi sin velocidad) o cayendo
		if (vel.y < -250) currentFrame = FRAME_JUMP_UP;
		else if (vel.y <= 250) currentFrame = FRAME_JUMP_PEAK;
		else currentFrame = FRAME_FALL;
		frameTimer = 0;
	}
	else if (moving)
	{
		//al empezar a caminar arranca desde un cuadro de caminata (si venia de otro estado)
		if (currentFrame < FRAME_WALK || currentFrame >= FRAME_WALK + WALK_FRAMES)
		{
			currentFrame = FRAME_WALK;
			frameTimer = 0;
		}
		frameTimer += dt;
		if (frameTimer >= 0.09f)
		{
			frameTimer -= 0.09f;
			currentFrame = FRAME_WALK + (currentFrame - FRAME_WALK + 1) % WALK_FRAMES;
		}
	}
	else
	{
		//reposo: respira (dos cuadros que alternan lento)
		if (currentFrame != FRAME_IDLE && currentFrame != FRAME_IDLE + 1)
		{
			currentFrame = FRAME_IDLE;
			frameTimer = 0;
		}
		frameTimer += dt;
		if (frameTimer >= 0.6f)
		{
			frameTimer -= 0.6f;
			currentFrame = (currentFrame == FRAME_IDLE) ? FRAME_IDLE + 1 : FRAME_IDLE;
		}
	}
}
void player::Update(float dt, Rectangle* solids, int solidCount, Rectangle* oneWay, int oneWayCount, float worldRight)
{
	if (invulnTimer > 0) invulnTimer -= dt;

	HandleInput();

	//MRUA: la velocidad vertical crece con la aceleracion de la gravedad
	vel.y += gravity * dt;

	//la colision se resuelve por separado en cada eje (metodo estandar en plataformeros)
	MoveX(dt, solids, solidCount, worldRight);
	MoveY(dt, solids, solidCount, oneWay, oneWayCount);

	UpdateAnimation(dt);
	equippedWeapon.Update(dt, pos, size, facingLeft);
}
void player::Draw()
{
	//parpadeo mientras dura la invulnerabilidad
	if (invulnTimer > 0 && ((int)(invulnTimer * 10) % 2 == 0)) return;

	//cada cuadro mide 64x128 (igual que el jugador); la hoja mira a la derecha y se espeja para la izquierda
	float fw = 64.0f, fh = (float)sheet.height;
	Rectangle src = { currentFrame * fw, 0, facingLeft ? -fw : fw, fh };
	Rectangle dst = { pos.x, pos.y, size.x, size.y };
	DrawTexturePro(sheet, src, dst, { 0,0 }, 0, WHITE);

	equippedWeapon.Draw(pos, size, facingLeft);
}
void player::Unload()
{
	UnloadTexture(sheet);
	equippedWeapon.Unload();
}
Rectangle player::GetHitbox()
{
	return { pos.x, pos.y, size.x, size.y };
}
Vector2 player::GetPosition()
{
	return pos;
}
Vector2 player::GetSize()
{
	return size;
}
bool player::IsFacingLeft()
{
	return facingLeft;
}
void player::TakeDamage()
{
	if (invulnTimer > 0) return;
	lives--;
	invulnTimer = 1.5f;
	vel.y = -500;
}
bool player::IsDead()
{
	return lives <= 0;
}
int player::GetLives()
{
	return lives;
}
bool player::IsInvulnerable()
{
	return invulnTimer > 0;
}
void player::SetSpawn(Vector2 spawnPos)
{
	spawnPoint = spawnPos;
}
void player::Translate(Vector2 delta)
{
	pos.x += delta.x;
	pos.y += delta.y;
}
void player::Reset()
{
	pos = spawnPoint;
	vel = { 0,0 };
	grounded = false;
	invulnTimer = 1.0f;
}
void player::FullReset()
{
	lives = 3;
	Reset();
}
void player::EquipWeapon(weaponType type)
{
	equippedWeapon.Equip(type);
}
weaponType player::GetWeaponType()
{
	return equippedWeapon.GetType();
}
bool player::ConsumeWeaponEvent(weaponEvent& out)
{
	return equippedWeapon.ConsumeEvent(out);
}
