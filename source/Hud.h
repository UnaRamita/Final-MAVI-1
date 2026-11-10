#pragma once
#include <iostream>
#include <string>
#include "raylib.h"

class hud
{
public:
	hud();
	~hud();

	void Draw(int lives, int score, int itemsCollected, int itemsTotal, float elapsedTime, const char* weaponName);

private:
	int fontSize;
};
