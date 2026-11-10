#pragma once
#include "raylib.h"
#include "Weapons.h"

class player
{
public:
	player();
	~player();

	void Init();
	//solids: bloques macizos (puertas cerradas), chocan por todos los lados.
	//oneWay: plataformas que solo se pisan desde arriba; desde abajo o de costado se atraviesan
	void Update(float dt, Rectangle* solids, int solidCount, Rectangle* oneWay, int oneWayCount, float worldRight);
	void Draw();
	void Unload();

	Rectangle GetHitbox();
	Vector2 GetPosition();
	Vector2 GetSize();
	bool IsFacingLeft();

	void TakeDamage();
	bool IsDead();
	int GetLives();
	bool IsInvulnerable();
	void SetSpawn(Vector2 spawnPos); //cada nivel define su propio punto de aparicion
	void Translate(Vector2 delta);   //desplaza sin pasar por fisica (para acompañar plataformas moviles)
	void Reset();     //vuelve al ultimo spawn (sin tocar vidas ni arma)
	void FullReset(); //vidas al maximo + Reset() (el arma la fija Level con EquipWeapon)

	void EquipWeapon(weaponType type);
	weaponType GetWeaponType();
	//el arma genera eventos (tajo, contacto, disparo) en los frames de su animacion; Level los consume de a uno
	bool ConsumeWeaponEvent(weaponEvent& out);

private:
	void HandleInput();
	void CutJumpIfReleased(bool jumpHeld); //salto variable: soltar la tecla en el ascenso acorta el salto
	void MoveX(float dt, Rectangle* solids, int solidCount, float worldRight);
	void MoveY(float dt, Rectangle* solids, int solidCount, Rectangle* oneWay, int oneWayCount);
	void UpdateAnimation(float dt);

	//hoja de animacion (11 cuadros de 64x128, mirando a la derecha):
	//  0-1 reposo (respira), 2-7 caminata, 8 salto subiendo, 9 pico del salto, 10 cayendo
	Texture2D sheet;
	int currentFrame;
	float frameTimer;
	bool facingLeft;
	bool moving;

	//MRU en el eje X (velocidad constante), MRUA en el eje Y (gravedad)
	Vector2 pos;
	Vector2 vel;
	float moveSpeed;
	float gravity;
	float jumpForce;
	bool grounded;
	float attackHold; //segundos que lleva apretado el boton de ataque

	Vector2 size;
	Vector2 spawnPoint;

	int lives;
	float invulnTimer;

	weapon equippedWeapon;
};
