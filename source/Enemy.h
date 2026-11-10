#pragma once
#include <vector>
#include "raylib.h"

//lo que un enemigo necesita saber del mundo en cada frame
struct enemyWorld
{
	Vector2 playerPos;                 //esquina superior izquierda del jugador (para distancias y direccion)
	Vector2 playerCenter;              //centro del jugador: hacia ahi "miran" los enemigos para ver si lo ven
	const std::vector<Rectangle>* walls; //paredes macizas (puertas cerradas): no se atraviesan ni se ve a traves de ellas
};

//clase base: todo enemigo tiene posicion, hitbox, animacion y puede morir.
//al morir deja de colisionar y de contar (IsAlive() = false) pero sigue dibujandose un instante
//para mostrar su animacion de muerte (slime: se aplasta; el resto: nube de humo).
class enemy
{
public:
	enemy();
	virtual ~enemy();

	virtual void Update(float dt, const enemyWorld& world) = 0;
	void AdvanceAnim(float dt); //avanza el reloj de animacion y de muerte (se llama aunque este muerto)
	void Draw();

	Rectangle GetHitbox();
	bool IsAlive();
	void Kill();
	//resta vida; si llega a 0 muere. Devuelve true si este golpe lo mato. Si sobrevive parpadea en rojo un instante
	bool TakeHit(int damage);
	void Translate(Vector2 delta); //desplaza sin pasar por su propia logica de movimiento (plataformas moviles)

	//por defecto un enemigo no dispara; solo enemyRanged lo sobreescribe
	virtual bool TryConsumeShot(Vector2& outPos, bool& outFacingLeft);

protected:
	void InitBase(Vector2 startPos, Vector2 spriteSize);

	virtual void DrawAlive() = 0;
	virtual void DrawDeath(); //por defecto: humo centrado en la hitbox
	void SetHealth(int value); //vida maxima (cada tipo de enemigo define la suya en su Init)

	//true si el enemigo ve al jugador: el segmento entre sus centros no toca ninguna pared
	bool CanSeePlayer(const enemyWorld& world);
	//si despues de moverse en X quedo metido en una pared, lo deja pegado a ella. Devuelve +1 si choco yendo hacia
	//la derecha, -1 si yendo hacia la izquierda, 0 si no toco ninguna
	int ResolveWalls(const enemyWorld& world);
	Color HurtTint();          //blanco normal; rojizo un instante despues de recibir un golpe sin morir

	int health;
	float hurtTimer;

	Vector2 pos;
	Vector2 vel;
	Vector2 size;
	bool alive;

	float animTime;   //segundos desde que aparecio: define el cuadro de la animacion en loop
	float deathTime;  //segundos desde que murio
	bool facingLeft;
};

//MRU: patrulla entre dos puntos en linea recta a velocidad constante (slime: avanza a saltitos)
class enemyLinear : public enemy
{
public:
	void Init(Vector2 startPos, Vector2 spriteSize, float minX, float maxX, float speed);
	void Update(float dt, const enemyWorld& world) override;

protected:
	void DrawAlive() override;
	void DrawDeath() override;

private:
	float minX;
	float maxX;
	float speed;
};

//MRUA: acelera horizontalmente hacia el jugador cuando esta dentro de su rango.
//sus limites de movimiento son los bordes reales del mapa (no valores hardcodeados por instancia),
//asi que si el mapa cambia de tamaño no hace falta tocar nada aca.
class enemyAccelerated : public enemy
{
public:
	void Init(Vector2 startPos, Vector2 spriteSize, float accel, float maxSpeed);
	void Update(float dt, const enemyWorld& world) override;

protected:
	void DrawAlive() override;

private:
	float minX;
	float maxX;
	float accel;
	float maxSpeed;
};

//MCU: orbita un punto central a velocidad angular constante (abeja)
class enemyCircular : public enemy
{
public:
	void Init(Vector2 centerPos, Vector2 spriteSize, float radius, float angularSpeed);
	void Update(float dt, const enemyWorld& world) override;

protected:
	void DrawAlive() override;

private:
	Vector2 center;
	float radius;
	float angle;
	float angularSpeed;
};

//enemigo a distancia (orco): patrulla lento (MRU) y, cuando el jugador esta en rango, se frena y
//hace su animacion de disparo; el proyectil sale en el cuadro del lanzamiento, desde la mano.
//comportamiento realmente distinto a los otros 3: no ataca por contacto, ataca a distancia.
class enemyRanged : public enemy
{
public:
	void Init(Vector2 startPos, Vector2 spriteSize, float minX, float maxX, float speed, float shootRange, float shootCooldown);
	void Update(float dt, const enemyWorld& world) override;
	bool TryConsumeShot(Vector2& outPos, bool& outFacingLeft) override;

protected:
	void DrawAlive() override;

private:
	float minX;
	float maxX;
	float speed;
	float shootRange;
	float shootCooldown;
	float shootTimer;

	bool casting;      //reproduciendo la animacion de disparo (esta quieto)
	float castTime;
	bool shotFired;    //el proyectil de este disparo ya salio

	bool wantsShot;
	bool shotFacingLeft;
};
