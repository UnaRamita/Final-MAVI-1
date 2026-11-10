#include "Door.h"
#include "Art.h"

//hoja de la puerta: 8 cuadros de 30x600 (0 = cerrada ... 7 = abierta)
static const int DOOR_FRAMES = 8;
static const float DOOR_FRAME_TIME = 0.06f;

door::door()
{
	bounds = { 0,0,0,0 };
	locked = true;
	openTimer = 0;
}
door::~door()
{
}
void door::Init(Rectangle boundsVal)
{
	bounds = boundsVal;
	locked = true;
	openTimer = 0;
}
void door::Unlock()
{
	if (locked) openTimer = 0; //arranca la animacion de apertura recien al desbloquear
	locked = false;
}
void door::Reset()
{
	locked = true;
	openTimer = 0;
}
void door::Update(float dt)
{
	if (!locked) openTimer += dt;
}
bool door::IsLocked()
{
	return locked;
}
Rectangle door::GetHitbox()
{
	return bounds;
}
void door::Draw()
{
	int frame = 0;
	if (!locked)
	{
		frame = (int)(openTimer / DOOR_FRAME_TIME);
		if (frame > DOOR_FRAMES - 1) frame = DOOR_FRAMES - 1;
	}

	Rectangle src = { (float)(frame * 30), 0, 30, 600 };
	DrawTexturePro(Art::doorSheet, src, bounds, { 0,0 }, 0, WHITE);
}
