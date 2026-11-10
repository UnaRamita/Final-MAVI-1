#pragma once
#include <vector>
#include "raylib.h"
#include "Player.h"
#include "EnemyManager.h"
#include "Item.h"
#include "Hud.h"
#include "SoundManager.h"
#include "Weapons.h"
#include "Proyectile.h"
#include "Door.h"
#include "Button.h"
#include "Hazard.h"
#include "Stone.h"
#include "MovingPlatform.h"
#include "Boss.h"

enum class levelState { PLAYING, GAMEOVER, LEVEL_COMPLETE };

//motor de gameplay reutilizado por todos los niveles (0 = nivel de prueba, 1-10 = progresion).
//cada nivel solo define su propio layout en un metodo BuildLevelX(); el resto (fisica,
//colisiones, HUD, sonido, armas) es exactamente el mismo sistema para todos.
class level
{
public:
	level();
	~level();

	void Init(float fxVolume);                          //carga texturas compartidas (una sola vez por sesion)
	void StartLevel(int levelIndex, weaponType startingWeapon); //arma el nivel elegido y lo deja listo para jugar
	void Reset();                                        //reinicia el nivel actual (mismo indice, misma arma inicial)
	void Update();
	void Unload();

	bool WantsExitToMenu(); //true una vez cuando se elige "Volver al menu" desde la pausa
	bool WantsOptions();    //true una vez cuando se elige "Opciones" desde la pausa
	void SetFxVolume(float volume01);

	//true una sola vez cuando se completa un nivel; entrega el numero de nivel y el arma final
	bool ConsumePendingCompletion(int& outLevelNumber, weaponType& outWeapon);
	//true una sola vez cuando ya se puede cargar el siguiente nivel (niveles 1-9, tras un breve festejo)
	bool ConsumeAdvanceRequest(int& outCompletedLevel);

private:
	void BuildLevel(int levelIndex);
	void BuildTestLevel();
	void BuildLevel1();
	void BuildLevel2();
	void BuildLevel3();
	void BuildLevel4();
	void BuildLevel5();
	void BuildLevel6();
	void BuildLevel7();
	void BuildLevel8();
	void BuildLevel9();
	void BuildLevel10();
	void BuildLevel11();

	void UpdateGameplay(float dt);
	void UpdateObjectives(float dt); //logica de desbloqueo especifica de cada nivel (puertas/piedras)
	void HandleAttack();
	void CompleteLevel();
	void DrawWorld();
	void DrawEndScreen(const char* title, Color color, const char* hint);

	void PauseInputs();
	void DrawPauseMenu();
	void DrawPauseButton(Rectangle b, const char* label, Vector2 mouse);

	player mainPlayer;
	enemyManager enemies;
	std::vector<item> items;
	std::vector<weaponPickup> weaponPickups;
	std::vector<proyectile> projectiles;      //disparos del jugador
	std::vector<proyectile> enemyProjectiles; //disparos de enemigos a distancia
	std::vector<door> doors;
	std::vector<button> buttons;
	std::vector<hazard> hazards;
	std::vector<stone> stones;
	hud hudDisplay;
	soundManager sounds;

	std::vector<Rectangle> platforms;
	std::vector<movingPlatform> movingPlatforms;
	Texture2D swordPickupTexture;
	Texture2D staffPickupTexture;

	//disparo de fuego animado (hojas de frames 80x80): lanzamiento (solo al salir del baston) y vuelo en loop
	Texture2D fireLaunchTexture;
	Texture2D fireFlightTexture;      //fuego rojo: baston del jugador y disparos chicos del jefe
	Texture2D enemyFlightTexture;     //mismo vuelo con colores azules: disparos de los enemigos a distancia

	//jefe final (nivel 11): un unico boss opcional, no forma parte de enemyManager porque
	//su logica (fases, ciclos, barra de vida) es totalmente distinta a los enemigos comunes
	boss levelBoss;
	bool hasBoss;
	bossState lastBossState; //para detectar cambios de fase y disparar sonidos una sola vez

	int score;
	int itemsTotal;
	float elapsedTime;

	int currentLevelIndex;   //0 = nivel de prueba, 1..10 = progresion
	weaponType levelStartWeapon;
	int exitDoorIndex;       //indice en doors[] que actua como salida del nivel; -1 = sin salida fija
	Rectangle arenaBounds;   //zona de "encierro" reutilizada por los niveles con arena (7, 9, 10)

	levelState state;
	bool exitToMenu;
	bool wantsOptions;
	bool pendingCompletion;
	bool pendingAdvance;
	float completionTimer; //cuenta regresiva antes de pasar solo al siguiente nivel (1-9)

	bool paused;
	Rectangle pauseContinueButton;
	Rectangle pauseOptionsButton;
	Rectangle pauseMenuButton;
};
