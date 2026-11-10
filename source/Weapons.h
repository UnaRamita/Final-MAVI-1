#pragma once
#include <vector>
#include "raylib.h"

enum class weaponType { NONE, SWORD, STAFF };

//eventos que genera la animacion de ataque del arma. Level los consume y aplica el efecto
//(sonido, daño, disparo) en el frame EXACTO de la animacion donde corresponde.
enum class weaponEventType
{
	SWING,      //arranca el tajo de la espada (suena SliceSwordN)
	SWORD_HIT,  //momento de contacto de la espada: aplica el daño en hitRect
	STAFF_SHOT  //el baston suelta el proyectil en la gema (muzzle)
};
struct weaponEvent
{
	weaponEventType kind;
	int comboStep;      //espada: 1, 2 o 3
	Rectangle hitRect;  //SWORD_HIT: zona de daño en coordenadas del mundo
	float damageMul;    //SWORD_HIT: 1 = golpe normal, 2 = golpe final del combo
	Vector2 muzzle;     //STAFF_SHOT: punto de salida del disparo, en el mundo
	bool facingLeft;
};

//arma actualmente equipada por el jugador: solo se puede portar una a la vez.
//El arma flota junto al jugador; al atacar reproduce su animacion:
//  espada: combo de 3 golpes encadenables (base -> de abajo hacia arriba -> final largo de arriba hacia abajo)
//  baston: se echa hacia atras cargando y tira hacia adelante soltando el proyectil
class weapon
{
public:
	weapon();
	~weapon();

	void Init();
	void Unload();

	void Equip(weaponType newType);
	weaponType GetType();

	//playerPos/playerSize/facingLeft: el arma flota pegada al jugador, asi que necesita saber donde esta
	void Update(float dt, Vector2 playerPos, Vector2 playerSize, bool facingLeft);
	bool TryAttack(); //inicia el ataque, o (espada) encadena el siguiente golpe del combo. true si se acepto
	//espada: salta directo al 3er golpe (el fuerte) cancelando lo que se este haciendo; lo usa el jugador al mantener el boton
	bool TryHeavyAttack();
	void Draw(Vector2 playerPos, Vector2 playerSize, bool facingLeft);

	bool ConsumeEvent(weaponEvent& out); //saca de a un evento de la cola; false cuando no quedan

private:
	void UpdateSword(float dt);
	void UpdateStaff(float dt);
	void StartSwordStep(int step);
	void OnSwordFrameEnter(int step, int frame);
	void OnStaffFrameEnter(int frame);
	Vector2 AnchorWorld(Vector2 playerPos, Vector2 playerSize, bool left, float bob);

	weaponType type;

	Texture2D swordAnim[3]; //hojas de animacion de los 3 golpes (frames de 320x320, ancla en (120,150))
	Texture2D staffAnim;    //hoja del baston (7 frames de 160x160, ancla en (80,80))

	//posicion del jugador del ultimo Update (para ubicar eventos en el mundo)
	Vector2 pPos;
	Vector2 pSize;
	bool pLeft;

	//estado de la animacion de ataque
	bool attacking;
	int frame;
	float frameTimer;
	float idleTime; //para el flotado suave en reposo

	//espada: combo
	int comboStep;       //0 = sin combo; 1..3 = golpe actual o el ultimo que se hizo
	bool queuedNext;     //se apreto ataque durante un golpe: encadena al terminar
	float comboWindow;   //tiempo restante para encadenar el siguiente golpe tras terminar uno
	float endCooldown;   //pausa tras el tercer golpe

	//baston
	float cooldownTimer; //tiempo minimo entre hechizos

	std::vector<weaponEvent> events;
};

//arma tirada en el nivel: al tocarla equipa su tipo y reemplaza la que se tenga
class weaponPickup
{
public:
	weaponPickup();
	~weaponPickup();

	void Init(Vector2 position, weaponType pickupType, Texture2D tex);
	bool CheckPickup(Rectangle playerHitbox); //true si se recogio en este frame (toca el arma Y se apreto E)
	bool IsPlayerNear(Rectangle playerHitbox); //el jugador esta tocando el arma (todavia sin recogerla)
	void Draw();
	void DrawPrompt(Rectangle playerHitbox);  //cartelito "E - Recoger" sobre el arma cuando se puede agarrar
	bool IsCollected();
	void Reset();
	weaponType GetType();

	//intercambio de armas: el arma que el jugador tenia equipada queda tirada en este mismo lugar
	void ReplaceWith(weaponType newType, Texture2D newTexture);

private:
	Vector2 pos;
	Vector2 size;
	weaponType type;
	Texture2D texture;
	bool collected;
};
