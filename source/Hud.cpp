#include "Hud.h"

hud::hud()
{
	fontSize = 30;
}
hud::~hud()
{
}
void hud::Draw(int lives, int score, int itemsCollected, int itemsTotal, float elapsedTime, const char* weaponName)
{
	DrawText(TextFormat("Vidas: %d", lives), 20, 20, fontSize, RAYWHITE);
	DrawText(TextFormat("Puntos: %d", score), 20, 20 + (fontSize + 10), fontSize, RAYWHITE);
	DrawText(TextFormat("Items: %d/%d", itemsCollected, itemsTotal), 20, 20 + (fontSize + 10) * 2, fontSize, RAYWHITE);
	DrawText(TextFormat("Arma: %s", weaponName), 20, 20 + (fontSize + 10) * 3, fontSize, RAYWHITE);

	int minutes = (int)elapsedTime / 60;
	int seconds = (int)elapsedTime % 60;
	const char* timeText = TextFormat("Tiempo: %02d:%02d", minutes, seconds);
	DrawText(timeText, GetScreenWidth() - MeasureText(timeText, fontSize) - 20, 20, fontSize, RAYWHITE);
}
