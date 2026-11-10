#pragma once
#include <vector>
#include "raylib.h"

//estados del jefe final, claramente separados como pide la consigna
enum class bossState { INTRO, ATTACK_PHASE, VULNERABLE_PHASE, DEATH, DEFEATED };

//pedido de proyectil que el jefe quiere disparar: Level lo convierte en un "proyectile" real
//(se reutiliza la misma clase proyectile que ya usan el jugador y los enemigos a distancia)
struct bossProjectileRequest
{
	Vector2 pos;
	Vector2 vel;
	Vector2 size;
};

//un trozo de la onda horizontal del jefe. Cada segmento tiene su propia colision: mientras esta vivo
//lastima al jugador; al recibir un golpe del arma deja de existir (sin colision) y queda un hueco.
struct waveSegment
{
	Rectangle rect;
	bool alive;
	float breakTimer; //segundos desde que se rompio (solo para el efecto visual; no colisiona)
};

//una onda en pantalla: columna de segmentos apilados que avanza en horizontal
struct bossWave
{
	float dirX; //1 = hacia la derecha, -1 = hacia la izquierda
	std::vector<waveSegment> segments;
};

//jefe final del nivel 11. Combate en 3 ciclos identicos: fase de ataque (esquivar) seguida de
//una fase vulnerable (atacarlo). Solo puede morir al recibir suficiente daño durante el ultimo ciclo.
class boss
{
public:
	boss();
	~boss();

	//parametros configurables (equivalente al "inspector"): los define quien arma el nivel,
	//no estan escritos a mano dentro de la logica de estados
	void Init(Vector2 spawnPos, Vector2 spriteSize, float attackPhaseDurationSeconds, int cyclesRequiredVal, int maxHPVal);
	void Reset(); //vuelve a INTRO con la config ya cargada (para reintentar el nivel)

	void Update(float dt, Vector2 playerPos);
	void Draw();     //sprite + telegrafos de ataque (va con el resto del mundo)
	void DrawHUD();  //barra de vida + texto de fase (se dibuja al final, siempre encima)

	void TakeDamage(int amount);
	bool CanBeDamaged();
	bool IsDefeated();
	Rectangle GetHitbox();
	bossState GetState();

	int CollectAttacks(std::vector<bossProjectileRequest>& outRequests); //vacia y devuelve los disparos pedidos este frame

	//--- onda horizontal destruible ---
	void SetWaveTexture(Texture2D fireFlightSheet); //hoja de fuego (frames de 80x80) con la que se dibujan los segmentos
	bool WaveTouches(Rectangle area);               //true si algun segmento VIVO toca el rectangulo (no modifica nada)
	int BreakWave(Rectangle area);                  //destruye los segmentos vivos que toca el rectangulo; devuelve cuantos
	int CountWaveSegments();                        //segmentos vivos en pantalla (para pruebas)
	int CountWaves();                               //ondas activas (para pruebas)

private:
	void UpdateIntro(float dt);
	void UpdateAttackPhase(float dt, Vector2 playerPos);
	void UpdateDeath(float dt);

	void QueueProjectile(Vector2 pos, Vector2 vel, Vector2 size);
	void PatternAimedShot(Vector2 playerPos); //proyectil dirigido a la posicion del jugador
	void PatternFanBurst();                   //rafaga en abanico (patron de varios proyectiles)
	void SpawnWave(float startX, float dirX); //crea una onda nueva (columna de segmentos) en el borde de la arena
	void UpdateWaves(float dt);               //avanza las ondas, anima los segmentos rotos y descarta lo que ya termino
	void DrawWaves();
	void DrawHealthBarInternal();

	Vector2 pos;
	Vector2 size;

	bossState state;
	float stateTimer;

	float introDuration;
	float attackPhaseDuration; //configurable
	float deathDuration;

	int cyclesRequired; //configurable
	int currentCycle;   //1..cyclesRequired

	int maxHP;    //configurable
	int currentHP;
	int hpPerCycle;

	//tiro dirigido
	bool aimedCharging;
	float aimedChargeTimer;
	float aimedCooldown;

	//rafaga en abanico
	bool fanCharging;
	float fanChargeTimer;
	float fanCooldown;

	//onda expansiva (atraviesa toda la arena)
	bool waveTelegraphing;
	float waveTelegraphTimer;
	float waveCooldown;
	float waveStartX;
	float waveDirX;

	std::vector<bossProjectileRequest> pendingAttacks;

	std::vector<bossWave> waves; //ondas horizontales en vuelo
	Texture2D waveTexture;       //hoja de fuego prestada por Level (no es propietario: Level la descarga)

	float flashTimer;  //parpadeo breve al recibir daño
	float bobTimer;    //animacion de flotado, solo visual

	//animacion de ataque: se agacha mientras carga un ataque y salta/golpea al soltarlo
	float chargeTime;   //segundos cargando un ataque (0 = no esta cargando)
	float releaseTime;  //segundos desde que solto el ataque (-1 = no esta en esa animacion)
	bool wasCharging;
};
