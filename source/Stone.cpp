#include "Stone.h"
#include "Art.h"

stone::stone()
{
	pos = { 0,0 };
	cleared = false;
}
stone::~stone()
{
}
void stone::Init(Vector2 position)
{
	pos = position;
	cleared = false;
}
void stone::SetCleared(bool clearedVal)
{
	cleared = clearedVal;
}
bool stone::IsCleared()
{
	return cleared;
}
void stone::Draw()
{
	//orbe de cristal animado (6 cuadros de 64x64): rojo mientras falten enemigos, verde al limpiar la zona
	int frame = ((int)((float)GetTime() / 0.1f)) % 6;
	Texture2D sheet = cleared ? Art::orbCleared : Art::orbPending;
	Rectangle src = { (float)(frame * 64), 0, 64, 64 };
	Rectangle dst = { pos.x - 32, pos.y - 32, 64, 64 };
	DrawTexturePro(sheet, src, dst, { 0,0 }, 0, WHITE);
}
void stone::Reset()
{
	cleared = false;
}
