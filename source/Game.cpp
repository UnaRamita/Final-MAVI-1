#include "Game.h"
game::game()
{
}

void game::Inits() {
	//ventana sin bordes de 1920x1080 pegada a la esquina: ocupa todo el monitor y no queda tapada por la
	//barra de titulo ni por la barra de tareas de Windows (el nivel mide exactamente 1920x1080)
	SetConfigFlags(FLAG_WINDOW_UNDECORATED);
	InitWindow(1920,1080,"Tp Final MAVI");
	SetWindowPosition(0, 0);
	SetExitKey(KEY_NULL); //ESC no debe cerrar la ventana: el juego ya usa ESC para pausa/menus
	SetTargetFPS(60);
	InitAudioDevice();

	scene.Inits();
}
void game::Update(){
	scene.Update();
}
bool game::Run() {
	ClearBackground(BLACK);
	Update();
	if (scene.ShouldExit())
	{
		return false;
	}
	else {
		return true;
	}
}
game::~game()
{
	CloseAudioDevice();
	CloseWindow();
}