#pragma once
#include "raylib.h"

//hoja de animacion: cuadros del mismo tamaño en fila (de izquierda a derecha)
struct animSheet
{
	Texture2D tex;
	int frames;
	int fw;
	int fh;
	float frameTime; //segundos que dura cada cuadro
};

//todo el arte del castillo (fondos, tiles, puertas, trampas, decoracion y animaciones de enemigos)
//se carga UNA sola vez aca y lo comparten todas las clases. Los sprites del mapa ya vienen a
//tamaño de pantalla (no hace falta escalarlos); los de los enemigos vienen a tamaño original.
namespace Art
{
	void Load();   //idempotente: si ya se cargo no hace nada
	void Unload();

	//elige la ambientacion segun el nivel: 0-3 salon, 4-6 mazmorra, 7-10 torre, 11 trono
	void SetTheme(int levelIndex);

	//fondo + decoracion de pared (halos, estandartes, antorchas); va antes que todo lo demas
	void DrawBackground();

	//fondo de los menus: el salon del castillo deslizandose de izquierda a derecha sin parar
	void DrawMenuBackground();

	//rectangulo de colision dibujado como piso (tile repetido) o plataforma (tapas + medio)
	void DrawSolid(Rectangle r);
	//plataforma movil: acero con runas animadas
	void DrawMovingPlatform(Rectangle r);

	//dibuja un cuadro de una hoja. anchor = abajo-centro (o centro si centered). scale = zoom entero del sprite
	void DrawFrame(const animSheet& a, int frame, Vector2 anchor, float scale, bool flipX, bool centered, Color tint);
	//cuadro que le toca a un reloj (en segundos) para una animacion en loop
	int LoopFrame(const animSheet& a, float time);

	//--- hojas de objetos del mapa ---
	extern Texture2D doorSheet;      //8 cuadros de 30x600
	extern Texture2D buttonSheet;    //2 cuadros de 60x20
	extern Texture2D spikeSheet;     //4 cuadros de 30x30
	extern Texture2D orbPending;     //6 cuadros de 64x64
	extern Texture2D orbCleared;     //6 cuadros de 64x64
	extern Texture2D coinSheet;      //6 cuadros de 28x28

	//--- animaciones de enemigos ---
	extern animSheet slimeIdle, slimeHop, slimeDeath;
	extern animSheet beeFly;
	extern animSheet dogIdle, dogRun;
	extern animSheet orcIdle, orcWalk, orcCast;
	extern animSheet puff;
	extern animSheet boss1Idle, boss1Slam, boss2Idle, boss2Slam;
}
