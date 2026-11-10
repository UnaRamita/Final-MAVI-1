#include "Hazard.h"
#include "Collisions.h"
#include "Art.h"

hazard::hazard()
{
	bounds = { 0,0,0,0 };
	active = true;
	togglePeriod = 0;
	timer = 0;
}
hazard::~hazard()
{
}
void hazard::Init(Rectangle boundsVal, float togglePeriodVal)
{
	bounds = boundsVal;
	togglePeriod = togglePeriodVal;
	active = true;
	timer = 0;
}
void hazard::Update(float dt)
{
	if (togglePeriod <= 0) return; //peligro estatico: siempre activo

	//trampa temporizada: se prende y apaga cada mitad del periodo (MRU aplicado a un ciclo on/off)
	timer += dt;
	if (timer >= togglePeriod / 2.0f)
	{
		timer = 0;
		active = !active;
	}
}
void hazard::Draw()
{
	//hoja de 4 cuadros de 30x30: 0 = escondidos, 1 = aviso (brillo naranja), 2 = a medias, 3 = salidos
	int frame = 3; //los pinchos fijos siempre estan salidos
	if (togglePeriod > 0)
	{
		float remaining = togglePeriod / 2.0f - timer; //tiempo que falta para el proximo cambio
		if (timer < 0.08f) frame = 2;                  //recien cambio de estado: a medias (sube o baja)
		else if (active) frame = 3;
		else if (remaining < 0.35f) frame = 1;         //por salir: avisa un instante antes de hacer daño
		else frame = 0;
	}

	//el mosaico se ajusta al alto de la caja de daño (30 normalmente) y se repite a lo ancho
	float s = bounds.height / 30.0f;
	float tileW = 30.0f * s;
	for (float x = bounds.x; x < bounds.x + bounds.width; x += tileW)
	{
		float w = (bounds.x + bounds.width) - x;
		if (w > tileW) w = tileW;
		Rectangle src = { (float)(frame * 30), 0, w / s, 30 };
		Rectangle dst = { x, bounds.y, w, bounds.height };
		DrawTexturePro(Art::spikeSheet, src, dst, { 0,0 }, 0, WHITE);
	}
}
void hazard::Reset()
{
	active = true;
	timer = 0;
}
bool hazard::CheckDamage(Rectangle playerHitbox)
{
	if (!active) return false;
	return Collisions::CheckAABB(bounds, playerHitbox);
}
