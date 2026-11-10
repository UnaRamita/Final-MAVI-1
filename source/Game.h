#pragma once
#include <iostream>
#include "raylib.h"	
#include "Scene.h"
using namespace std;
class game
{
public:
	game();
	~game();

	void Inits();
	bool Run();
	
private:
	void Update();
	void Inputs();

	scene scene;
};
