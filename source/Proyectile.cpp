#include "Proyectile.h"
#include <math.h>

namespace
{
	//hojas "Disparo fuego launch / flight": frames de 80x80, el sprite original (32x32) va centrado
	const float FIRE_FRAME = 80.0f;
	const float FIRE_SPRITE = 32.0f;
	const int   LAUNCH_FRAMES = 6;
	const int   FLIGHT_FRAMES = 6;
	const float LAUNCH_FRAME_TIME = 0.05f;
	const float FLIGHT_FRAME_TIME = 0.07f;
}

proyectile::proyectile()
{
	pos = { 0,0 };
	vel = { 0,0 };
	size = { 32,32 };
	texture = { 0 };
	alive = false;

	animated = false;
	launchTexture = { 0 };
	animTimer = 0;
	launchDuration = 0;
	angleDeg = 0;
}
proyectile::~proyectile()
{
}
void proyectile::Init(Vector2 startPos, bool facingLeft, Texture2D tex)
{
	pos = startPos;
	texture = tex;
	size = { 32,32 };
	animated = false;

	//MRU: velocidad constante en linea recta hacia donde mira el jugador
	float speed = 700.0f;
	vel = { facingLeft ? -speed : speed, 0 };
	alive = true;
}
void proyectile::Init(Vector2 startPos, Vector2 velocity, Vector2 sizeOverride, Texture2D tex)
{
	//variante de direccion/tamaño libres: la usan los patrones de ataque del jefe final
	//(tiros apuntados en cualquier angulo, rafagas en abanico, ondas anchas que cruzan la arena)
	pos = startPos;
	texture = tex;
	size = sizeOverride;
	vel = velocity;
	animated = false;
	alive = true;
}
void proyectile::InitFire(Vector2 center, Vector2 velocity, float hitboxSize, Texture2D launchTex, Texture2D flightTex)
{
	size = { hitboxSize, hitboxSize };
	pos = { center.x - hitboxSize / 2, center.y - hitboxSize / 2 };
	vel = velocity;
	texture = flightTex;
	launchTexture = launchTex;
	animated = true;
	animTimer = 0;
	launchDuration = (launchTex.id != 0) ? LAUNCH_FRAMES * LAUNCH_FRAME_TIME : 0.0f;

	//el sprite original viaja hacia arriba-derecha (-45 grados): se rota para que apunte a donde va
	angleDeg = atan2f(velocity.y, velocity.x) * 180.0f / PI + 45.0f;
	alive = true;
}
void proyectile::Update(float dt)
{
	if (!alive) return;

	pos.x += vel.x * dt;
	pos.y += vel.y * dt;
	if (animated) animTimer += dt;

	if (pos.x < -size.x || pos.x > 1920 + size.x || pos.y < -size.y || pos.y > 1080 + size.y)
	{
		alive = false;
	}
}
void proyectile::Draw()
{
	if (!alive) return;

	if (!animated)
	{
		Rectangle src = { 0, 0, (float)texture.width, (float)texture.height };
		Rectangle dst = { pos.x, pos.y, size.x, size.y };
		DrawTexturePro(texture, src, dst, { 0,0 }, 0, WHITE);
		return;
	}

	bool launching = (launchDuration > 0 && animTimer < launchDuration);
	Texture2D tex;
	int frame;
	if (launching)
	{
		tex = launchTexture;
		frame = (int)(animTimer / LAUNCH_FRAME_TIME);
		if (frame >= LAUNCH_FRAMES) frame = LAUNCH_FRAMES - 1;
	}
	else
	{
		tex = texture;
		frame = ((int)((animTimer - launchDuration) / FLIGHT_FRAME_TIME)) % FLIGHT_FRAMES;
	}

	//el frame (80x80) se dibuja centrado en el hitbox y rotado alrededor de su centro
	float scale = size.x / FIRE_SPRITE;
	float cx = pos.x + size.x / 2, cy = pos.y + size.y / 2;
	Rectangle src = { frame * FIRE_FRAME, 0, FIRE_FRAME, FIRE_FRAME };
	Rectangle dst = { cx, cy, FIRE_FRAME * scale, FIRE_FRAME * scale };
	Vector2 origin = { FIRE_FRAME * scale / 2, FIRE_FRAME * scale / 2 };
	DrawTexturePro(tex, src, dst, origin, angleDeg, WHITE);
}
Rectangle proyectile::GetHitbox()
{
	//durante el lanzamiento la bolita es chica: el hitbox crece con ella en vez de pegar con todo el tamaño de entrada
	if (animated && launchDuration > 0 && animTimer < launchDuration)
	{
		float t = animTimer / launchDuration;
		float s = size.x * (0.45f + 0.55f * t);
		float cx = pos.x + size.x / 2, cy = pos.y + size.y / 2;
		return { cx - s / 2, cy - s / 2, s, s };
	}
	return { pos.x, pos.y, size.x, size.y };
}
bool proyectile::IsAlive()
{
	return alive;
}
void proyectile::Kill()
{
	alive = false;
}
