#pragma once
#include "raylib.h"

//piedra/cristal objetivo: roja mientras haya enemigos vivos, verde cuando se limpia la zona.
//es puramente visual + de estado; quien decide cuando esta "limpia" es el nivel (level).
class stone
{
public:
	stone();
	~stone();

	void Init(Vector2 position);
	void SetCleared(bool clearedVal);
	bool IsCleared();
	void Draw();
	void Reset();

private:
	Vector2 pos;
	bool cleared;
};
