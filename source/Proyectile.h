#pragma once
#include "raylib.h"

//proyectil que se mueve en linea recta a velocidad constante (MRU). Hay dos modos de dibujo:
//  - estatico (Init): una textura estirada al tamaño del hitbox (ondas anchas del jefe)
//  - fuego animado (InitFire): disparo de fuego con fase de lanzamiento (bolita -> cometa) y luego
//    vuelo en loop, rotado hacia donde viaja. Lo usan el baston, los enemigos a distancia y el jefe.
class proyectile
{
public:
	proyectile();
	~proyectile();

	void Init(Vector2 startPos, bool facingLeft, Texture2D tex);
	void Init(Vector2 startPos, Vector2 velocity, Vector2 sizeOverride, Texture2D tex); //direccion/tamaño libres (jefes, patrones)

	//center = centro del hitbox; hitboxSize = lado del hitbox cuadrado (32 = tamaño original del sprite).
	//launchTex puede ser {0} para saltear el lanzamiento (disparos que no salen de un arma)
	void InitFire(Vector2 center, Vector2 velocity, float hitboxSize, Texture2D launchTex, Texture2D flightTex);

	void Update(float dt);
	void Draw();

	Rectangle GetHitbox();
	bool IsAlive();
	void Kill();

private:
	Vector2 pos;
	Vector2 vel;
	Vector2 size;
	Texture2D texture;   //modo estatico: la textura; modo fuego: la hoja de vuelo
	bool alive;

	//fuego animado
	bool animated;
	Texture2D launchTexture;
	float animTimer;
	float launchDuration; //0 si no hay fase de lanzamiento
	float angleDeg;       //rotacion con la que se dibuja (el sprite original mira arriba-derecha)
};
