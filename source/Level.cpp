#include "Level.h"
#include "Collisions.h"
#include "Art.h"
#include <algorithm>

//misma hoja de vuelo del disparo de fuego pero con los canales rojo/azul intercambiados:
//el fuego naranja pasa a azul, asi los disparos de los enemigos se distinguen de los del jugador
static Texture2D LoadBlueFire(const char* path)
{
	Image img = LoadImage(path);
	ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
	Color* px = (Color*)img.data;
	for (int i = 0; i < img.width * img.height; i++)
	{
		unsigned char r = px[i].r;
		px[i].r = px[i].b;
		px[i].b = r;
	}
	Texture2D tex = LoadTextureFromImage(img);
	UnloadImage(img);
	return tex;
}

level::level()
{
	score = 0;
	itemsTotal = 0;
	elapsedTime = 0;
	state = levelState::PLAYING;
	exitToMenu = false;
	wantsOptions = false;
	pendingCompletion = false;
	pendingAdvance = false;
	completionTimer = -1;
	paused = false;
	pauseContinueButton = { 0,0,0,0 };
	pauseOptionsButton = { 0,0,0,0 };
	pauseMenuButton = { 0,0,0,0 };
	swordPickupTexture = { 0 };
	staffPickupTexture = { 0 };
	fireLaunchTexture = { 0 };
	fireFlightTexture = { 0 };
	enemyFlightTexture = { 0 };
	currentLevelIndex = 0;
	levelStartWeapon = weaponType::NONE;
	exitDoorIndex = -1;
	arenaBounds = { 0,0,0,0 };

	hasBoss = false;
	lastBossState = bossState::INTRO;
}
level::~level()
{
}

// ---------------------------------------------------------------------------
// Carga / ciclo de vida
// ---------------------------------------------------------------------------

void level::Init(float fxVolume)
{
	//texturas y sistemas compartidos por TODOS los niveles: se cargan una sola vez por sesion
	//todo el arte del castillo (fondos, tiles, puertas, trampas y animaciones de enemigos y jefe)
	Art::Load();
	swordPickupTexture = LoadTexture("Assets/Sprites/Weapons/Great sword 1.png");
	staffPickupTexture = LoadTexture("Assets/Sprites/Weapons/magic staff 1.png");
	fireLaunchTexture = LoadTexture("Assets/Sprites/Weapons/Disparo fuego launch.png");
	fireFlightTexture = LoadTexture("Assets/Sprites/Weapons/Disparo fuego flight.png");
	enemyFlightTexture = LoadBlueFire("Assets/Sprites/Weapons/Disparo fuego flight.png");

	//los segmentos de la onda horizontal del jefe se dibujan con la hoja de fuego (el jefe no es dueño de la textura)
	levelBoss.SetWaveTexture(fireFlightTexture);

	mainPlayer.Init();
	enemies.Init();
	sounds.Init();
	sounds.SetVolume(fxVolume);

	//botones del menu de pausa (posiciones fijas, se calculan una sola vez)
	float centerX = GetScreenWidth() / 2.0f;
	float centerY = GetScreenHeight() / 2.0f;
	pauseContinueButton = { centerX - 150, centerY - 100, 300, 70 };
	pauseOptionsButton = { centerX - 150, centerY, 300, 70 };
	pauseMenuButton = { centerX - 150, centerY + 100, 300, 70 };
}
void level::StartLevel(int levelIndex, weaponType startingWeapon)
{
	currentLevelIndex = levelIndex;
	levelStartWeapon = startingWeapon;
	Reset();
}
void level::Reset()
{
	BuildLevel(currentLevelIndex);

	score = 0;
	elapsedTime = 0;
	state = levelState::PLAYING;
	pendingCompletion = false;
	pendingAdvance = false;
	completionTimer = -1;
	paused = false;
	wantsOptions = false;
}
void level::SetFxVolume(float volume01)
{
	sounds.SetVolume(volume01);
}
bool level::WantsExitToMenu()
{
	if (exitToMenu)
	{
		exitToMenu = false;
		return true;
	}
	return false;
}
bool level::WantsOptions()
{
	if (wantsOptions)
	{
		wantsOptions = false;
		return true;
	}
	return false;
}
bool level::ConsumePendingCompletion(int& outLevelNumber, weaponType& outWeapon)
{
	if (!pendingCompletion) return false;

	pendingCompletion = false;
	outLevelNumber = currentLevelIndex;
	outWeapon = mainPlayer.GetWeaponType();
	return true;
}
bool level::ConsumeAdvanceRequest(int& outCompletedLevel)
{
	if (!pendingAdvance) return false;

	pendingAdvance = false;
	outCompletedLevel = currentLevelIndex;
	return true;
}
void level::CompleteLevel()
{
	if (state != levelState::PLAYING) return; //evita completar mas de una vez el mismo intento

	state = levelState::LEVEL_COMPLETE;
	sounds.PlayVictory();

	//el nivel de prueba (0) nunca reporta progreso; los niveles 1-10 si
	if (currentLevelIndex >= 1)
	{
		pendingCompletion = true;
	}

	//los niveles 1-10 pasan solos al siguiente tras un breve festejo; el 11 (jefe final) y el de prueba esperan
	if (currentLevelIndex >= 1 && currentLevelIndex < 11)
	{
		completionTimer = 1.5f;
	}
	else
	{
		completionTimer = -1;
	}
}

// ---------------------------------------------------------------------------
// Armado del nivel: cada Build define su propio layout, el resto es el mismo motor
// ---------------------------------------------------------------------------

void level::BuildLevel(int levelIndex)
{
	platforms.clear();
	movingPlatforms.clear();
	items.clear();
	weaponPickups.clear();
	doors.clear();
	buttons.clear();
	hazards.clear();
	stones.clear();
	projectiles.clear();
	enemyProjectiles.clear();
	enemies.Clear();
	exitDoorIndex = -1;
	arenaBounds = { 0,0,0,0 };
	hasBoss = false;
	lastBossState = bossState::INTRO;

	//ambientacion del castillo segun el nivel (salon, mazmorra, torre o trono)
	Art::SetTheme(levelIndex);

	switch (levelIndex)
	{
	case 1: BuildLevel1(); break;
	case 2: BuildLevel2(); break;
	case 3: BuildLevel3(); break;
	case 4: BuildLevel4(); break;
	case 5: BuildLevel5(); break;
	case 6: BuildLevel6(); break;
	case 7: BuildLevel7(); break;
	case 8: BuildLevel8(); break;
	case 9: BuildLevel9(); break;
	case 10: BuildLevel10(); break;
	case 11: BuildLevel11(); break;
	default: BuildTestLevel(); break;
	}

	itemsTotal = (int)items.size();

	mainPlayer.FullReset();
	mainPlayer.EquipWeapon(levelStartWeapon);
}

//nivel de prueba: sandbox exclusivo de desarrollo con un poco de todo. No es parte de la
//progresion (no llama CompleteLevel, no tiene salida fija: se sale con ESC). Se va a
//borrar antes de la version final.
void level::BuildTestLevel()
{
	mainPlayer.SetSpawn({ 100, 872 });

	platforms.push_back({ 0, 1000, 1920, 80 });    //suelo
	platforms.push_back({ 350, 820, 260, 30 });
	platforms.push_back({ 750, 680, 260, 30 });
	platforms.push_back({ 1150, 820, 260, 30 });
	platforms.push_back({ 1550, 620, 260, 30 });

	Vector2 itemPositions[] = { {150,940}, {450,780}, {850,640}, {1250,780}, {1650,580}, {1850,940} };
	for (int i = 0; i < 6; i++)
	{
		item newItem;
		newItem.Init(itemPositions[i], 14);
		items.push_back(newItem);
	}

	weaponPickup sword;
	sword.Init({ 300, 952 }, weaponType::SWORD, swordPickupTexture);
	weaponPickups.push_back(sword);

	weaponPickup staff;
	staff.Init({ 1880, 952 }, weaponType::STAFF, staffPickupTexture);
	weaponPickups.push_back(staff);

	door gate;
	gate.Init({ 1830, 400, 30, 600 });
	doors.push_back(gate); //index 0, solo de prueba (no es salida del nivel)

	button unlockButton;
	unlockButton.Init({ 850, 660, 60, 20 }, 0);
	buttons.push_back(unlockButton);

	hazard spikes;
	spikes.Init({ 1700, 970, 100, 30 }, 0); //estatico
	hazards.push_back(spikes);

	hazard blinking;
	blinking.Init({ 500, 970, 130, 30 }, 1.6f); //temporizado
	hazards.push_back(blinking);

	stone testStone;
	testStone.Init({ 960, 300 });
	stones.push_back(testStone);

	enemies.AddLinear({ 500,936 }, 500, 900, 100);
	enemies.AddAccelerated({ 1600,920 }, 400, 320);
	enemies.AddCircular({ 900,500 }, 140, 1.5f);
	enemies.AddRanged({ 200,904 }, 150, 280, 60, 600, 1.6f);

	//plataforma movil de prueba (horizontal) con un enemigo montado encima, para probar
	//que el enemigo acompaña el movimiento igual que el jugador
	movingPlatform demoPlatform;
	demoPlatform.Init({ 1050, 480, 200, 30 }, { 300, 0 }, 120);
	movingPlatforms.push_back(demoPlatform);
	enemies.AddLinear({ 1080,416 }, 1050, 1550, 0); //speed 0: se queda quieto y viaja con la plataforma

	exitDoorIndex = -1; //sin salida fija: se sale con ESC
}

//Nivel 1 - Combate basico: agarrar la espada, derrotar al enemigo, salir.
void level::BuildLevel1()
{
	mainPlayer.SetSpawn({ 100, 872 });

	platforms.push_back({ 0, 1000, 1920, 80 });

	weaponPickup sword;
	sword.Init({ 400, 952 }, weaponType::SWORD, swordPickupTexture);
	weaponPickups.push_back(sword);

	enemies.AddLinear({ 700,936 }, 700, 1100, 100);

	door exitGate;
	exitGate.Init({ 1700, 400, 30, 600 });
	doors.push_back(exitGate);
	exitDoorIndex = 0;

	item i1; i1.Init({ 250,950 }, 14); items.push_back(i1);
	item i2; i2.Init({ 1500,950 }, 14); items.push_back(i2);
}

//Nivel 2 - Plataformas + combate: subir una escalera, activar un boton arriba, bajar y salir.
void level::BuildLevel2()
{
	mainPlayer.SetSpawn({ 100, 872 });

	platforms.push_back({ 0, 1000, 1920, 80 });
	platforms.push_back({ 200, 860, 220, 30 });
	platforms.push_back({ 500, 720, 220, 30 });
	platforms.push_back({ 200, 580, 220, 30 });
	platforms.push_back({ 500, 440, 220, 30 });
	platforms.push_back({ 200, 300, 220, 30 }); //cima

	button topButton;
	topButton.Init({ 260, 280, 60, 20 }, 0); //desbloquea directamente la puerta 0 (salida)
	buttons.push_back(topButton);

	door exitGate;
	exitGate.Init({ 1800, 400, 30, 600 });
	doors.push_back(exitGate);
	exitDoorIndex = 0;

	enemies.AddLinear({ 520,656 }, 520, 680, 60);   //en la plataforma de y=720
	enemies.AddLinear({ 520,376 }, 520, 680, 70);   //en la plataforma de y=440
	enemies.AddLinear({ 1200,936 }, 1200, 1600, 90); //guardia final en el suelo

	item i1; i1.Init({ 300,830 }, 14); items.push_back(i1);
	item i2; i2.Init({ 300,270 }, 14); items.push_back(i2);
}

//Nivel 3 - Fácil/medio: segundo tipo de enemigo (a distancia). Un recorrido con 4 plataformas
//a distintas alturas y DOS enemigos a distancia obligan a decidir cuándo cerrar distancia,
//cuándo esperar su ventana de recarga y cuándo simplemente correr entre coberturas.
void level::BuildLevel3()
{
	mainPlayer.SetSpawn({ 100, 872 });

	platforms.push_back({ 0, 1000, 1920, 80 });
	platforms.push_back({ 450, 880, 180, 30 });  //paso bajo
	platforms.push_back({ 800, 820, 280, 30 });  //primer francotirador
	platforms.push_back({ 1250, 850, 220, 30 });
	platforms.push_back({ 1550, 760, 220, 30 }); //segundo francotirador, mas arriba

	weaponPickup staff; //arma opcional: probar el combate a distancia contra enemigos a distancia
	staff.Init({ 250, 952 }, weaponType::STAFF, staffPickupTexture);
	weaponPickups.push_back(staff);

	enemies.AddLinear({ 600,936 }, 550, 800, 100);
	enemies.AddRanged({ 850,724 }, 820, 1040, 45, 650, 1.5f);
	enemies.AddRanged({ 1580,664 }, 1570, 1730, 50, 650, 1.4f);

	door exitGate;
	exitGate.Init({ 1850, 400, 30, 600 });
	doors.push_back(exitGate);
	exitDoorIndex = 0;

	item i1; i1.Init({ 280,950 }, 14); items.push_back(i1);
	item i2; i2.Init({ 950,750 }, 14); items.push_back(i2);
	item i3; i3.Init({ 1650,700 }, 14); items.push_back(i3);
}

//Nivel 4 - Medio: la piedra roja/verde ahora exige recorrer todo el nivel. Los 4 tipos de
//enemigo estan repartidos en zonas bien separadas (esquina superior izquierda, centro, derecha)
//para que limpiar el objetivo signifique explorar, no pararse en un solo lugar.
void level::BuildLevel4()
{
	mainPlayer.SetSpawn({ 100, 872 });

	platforms.push_back({ 0, 1000, 1920, 80 });
	platforms.push_back({ 150, 820, 220, 30 }); //L1
	platforms.push_back({ 150, 620, 220, 30 }); //L2, arriba de L1
	platforms.push_back({ 850, 880, 220, 30 }); //paso central
	platforms.push_back({ 1550, 820, 220, 30 }); //R1

	enemies.AddLinear({ 300,936 }, 250, 550, 110);
	enemies.AddAccelerated({ 960,920 }, 380, 300); //recorre todo el mapa: puede aparecer en cualquier zona
	enemies.AddCircular({ 260,480 }, 140, 1.6f);   //obliga a subir a L1/L2 para encararla de cerca
	enemies.AddRanged({ 1570,724 }, 1570, 1750, 45, 650, 1.5f); //en R1

	stone objective;
	objective.Init({ 960, 300 });
	stones.push_back(objective);

	door exitGate;
	exitGate.Init({ 1800, 400, 30, 600 });
	doors.push_back(exitGate);
	exitDoorIndex = 0;

	item i1; i1.Init({ 260,600 }, 14); items.push_back(i1);
	item i2; i2.Init({ 900,860 }, 14); items.push_back(i2);
	item i3; i3.Init({ 1650,800 }, 14); items.push_back(i3);
	item i4; i4.Init({ 960,950 }, 14); items.push_back(i4);
}

//Nivel 5 - Medio: cadena de TRES interruptores (antes eran 2). Boton1 abre paso a la zona de
//boton2 (plataforma custodiada), boton2 abre paso a la zona de boton3 (mas arriba, otro guardia),
//boton3 recien abre la salida. El jugador tiene que pensar que activo y hacia donde ir despues.
void level::BuildLevel5()
{
	mainPlayer.SetSpawn({ 100, 872 });

	platforms.push_back({ 0, 1000, 1920, 80 });
	platforms.push_back({ 650, 850, 220, 30 }); //P1: boton2
	platforms.push_back({ 950, 900, 150, 30 }); //escalon
	platforms.push_back({ 1150, 760, 220, 30 }); //P2: boton3

	button button1;
	button1.Init({ 280, 980, 60, 20 }, 0);
	buttons.push_back(button1);

	door door0;
	door0.Init({ 550, 400, 30, 600 });
	doors.push_back(door0); //index 0

	button button2;
	button2.Init({ 700, 830, 60, 20 }, 1);
	buttons.push_back(button2);

	door door1;
	door1.Init({ 900, 400, 30, 600 });
	doors.push_back(door1); //index 1

	button button3;
	button3.Init({ 1200, 740, 60, 20 }, 2);
	buttons.push_back(button3);

	door exitGate;
	exitGate.Init({ 1800, 400, 30, 600 });
	doors.push_back(exitGate); //index 2
	exitDoorIndex = 2;

	enemies.AddLinear({ 670,786 }, 670, 850, 90);               //guarda boton2 en P1
	enemies.AddRanged({ 1170,664 }, 1170, 1350, 45, 600, 1.5f);  //guarda boton3 en P2

	item i1; i1.Init({ 380,950 }, 14); items.push_back(i1);
	item i2; i2.Init({ 750,800 }, 14); items.push_back(i2);
	item i3; i3.Init({ 1650,950 }, 14); items.push_back(i3);
}

//Nivel 6 - Medio/alto: las trampas ahora se combinan con un mecanismo y un tramo de combate,
//no aparecen solas. Puente opcional sobre los primeros picos (esquivar por arriba o arriesgarse
//por el suelo), un boton que abre paso al resto del nivel, un guardia justo despues del boton,
//y una segunda trampa temporizada antes de la salida.
void level::BuildLevel6()
{
	mainPlayer.SetSpawn({ 100, 872 });

	platforms.push_back({ 0, 1000, 1920, 80 });
	platforms.push_back({ 350, 860, 220, 30 }); //Bridge1
	platforms.push_back({ 700, 860, 220, 30 }); //Bridge2 (boton)
	platforms.push_back({ 1300, 850, 220, 30 }); //Bridge3

	hazard spikes1;
	spikes1.Init({ 380, 970, 150, 30 }, 0);
	hazards.push_back(spikes1);

	hazard timedTrap1;
	timedTrap1.Init({ 620, 970, 100, 30 }, 1.5f);
	hazards.push_back(timedTrap1);

	button midButton;
	midButton.Init({ 770, 840, 60, 20 }, 0);
	buttons.push_back(midButton);

	door door0;
	door0.Init({ 1000, 400, 30, 600 });
	doors.push_back(door0); //index 0

	hazard timedTrap2;
	timedTrap2.Init({ 1100, 970, 150, 30 }, 1.8f);
	hazards.push_back(timedTrap2);

	enemies.AddLinear({ 1080,936 }, 1030, 1280, 100); //guardia justo despues del mecanismo
	enemies.AddAccelerated({ 1650,920 }, 380, 300);

	door exitGate;
	exitGate.Init({ 1850, 400, 30, 600 });
	doors.push_back(exitGate); //index 1
	exitDoorIndex = 1;

	item i1; i1.Init({ 280,950 }, 14); items.push_back(i1);
	item i2; i2.Init({ 800,800 }, 14); items.push_back(i2);
	item i3; i3.Init({ 1750,950 }, 14); items.push_back(i3);
}

//Nivel 7 - Alto: la arena (piedra + 3 tipos de enemigo con posicion acotada: lineal, circular y
//a distancia sobre una plataforma) bloquea el paso hasta limpiarla. Despues, un perro suelto
//(recorre todo el mapa, no cuenta para el objetivo de la arena) persigue al jugador durante una
//seccion vertical de dos plataformas hasta el interruptor final, con una trampa justo a la salida
//de la arena para que la tension no se corte de golpe.
void level::BuildLevel7()
{
	mainPlayer.SetSpawn({ 100, 872 });

	platforms.push_back({ 0, 1000, 1920, 80 });
	platforms.push_back({ 650, 850, 200, 30 });  //dentro de la arena, para el francotirador
	platforms.push_back({ 1150, 820, 220, 30 }); //PostA
	platforms.push_back({ 1450, 700, 220, 30 }); //PostB (interruptor final)

	arenaBounds = { 450, 600, 600, 400 }; //zona de la arena (para CountAliveInRect)

	stone objective;
	objective.Init({ 750, 300 }); //centrada arriba de la arena, bien visible
	stones.push_back(objective);

	enemies.AddLinear({ 550,936 }, 500, 850, 110);
	enemies.AddCircular({ 750,750 }, 130, 1.8f);
	enemies.AddRanged({ 700,754 }, 670, 830, 45, 600, 1.5f); //sobre la plataforma de la arena

	door arenaGate;
	arenaGate.Init({ 1080, 400, 30, 600 });
	doors.push_back(arenaGate); //index 0: se abre al limpiar la arena

	hazard spikesAfterArena;
	spikesAfterArena.Init({ 1090, 970, 55, 30 }, 0); //justo al salir de la arena, mantiene la tension

	hazards.push_back(spikesAfterArena);

	//perro suelto: recorre todo el mapa, es una amenaza libre durante la escalada final
	//(a proposito NO forma parte del conteo de la arena, que necesita posiciones acotadas)
	enemies.AddAccelerated({ 1600,920 }, 380, 280);

	button finalButton;
	finalButton.Init({ 1500, 680, 60, 20 }, 1); //en PostB, arriba de la escalada
	buttons.push_back(finalButton);

	door exitGate;
	exitGate.Init({ 1850, 400, 30, 600 });
	doors.push_back(exitGate); //index 1: salida, la abre el interruptor final
	exitDoorIndex = 1;

	item i1; i1.Init({ 300,950 }, 14); items.push_back(i1);
	item i2; i2.Init({ 1200,780 }, 14); items.push_back(i2);
	item i3; i3.Init({ 1550,650 }, 14); items.push_back(i3);
}

//Nivel 8 - Alto: exploracion + puzzle + combate, ahora en secuencia estricta (ya no "3 botones
//en cualquier orden"): cada interruptor desbloquea la puerta que da acceso al siguiente tramo,
//obligando a explorar de izquierda a derecha, resolver una pequeña seccion de plataformas,
//combatir, y recien ahi encontrar el siguiente mecanismo.
void level::BuildLevel8()
{
	mainPlayer.SetSpawn({ 100, 872 });

	platforms.push_back({ 0, 1000, 1920, 80 });
	platforms.push_back({ 250, 800, 220, 30 });  //A: primer interruptor
	platforms.push_back({ 600, 830, 180, 30 });  //escalon hacia B
	platforms.push_back({ 850, 650, 220, 30 });  //B: segundo interruptor (guardado)
	platforms.push_back({ 1450, 800, 220, 30 }); //C: interruptor final

	button buttonA;
	buttonA.Init({ 330, 780, 60, 20 }, 0);
	buttons.push_back(buttonA);

	door door0;
	door0.Init({ 500, 400, 30, 600 });
	doors.push_back(door0); //index 0: da acceso al escalon y a B

	enemies.AddLinear({ 870,586 }, 870, 1050, 90); //guarda B

	button buttonB;
	buttonB.Init({ 930, 630, 60, 20 }, 1);
	buttons.push_back(buttonB);

	door door1;
	door1.Init({ 1100, 400, 30, 600 });
	doors.push_back(door1); //index 1: da acceso al tramo final y a C

	enemies.AddAccelerated({ 1000,920 }, 300, 260); //ronda por todo el mapa

	button buttonC;
	buttonC.Init({ 1530, 780, 60, 20 }, 2);
	buttons.push_back(buttonC);

	door exitGate;
	exitGate.Init({ 1800, 400, 30, 600 });
	doors.push_back(exitGate); //index 2
	exitDoorIndex = 2;

	item i1; i1.Init({ 300,780 }, 14); items.push_back(i1);
	item i2; i2.Init({ 900,630 }, 14); items.push_back(i2);
	item i3; i3.Init({ 1500,780 }, 14); items.push_back(i3);
}

//Nivel 9 - Muy alto: penultimo desafio, dividido en 5 secciones bien diferenciadas que combinan
//practicamente todo lo aprendido:
//  1) plataformas + enemigo a distancia (eleccion de arma incluida)
//  2) trampas + mecanismo (hay que cruzar picos, pararse en un escalon y activar un boton)
//  3) combate con varios tipos de enemigo + plataforma movil
//  4) objetivo de eliminar enemigos + piedra roja/verde (la misma seccion 3, vista como objetivo)
//  5) recorrido final hacia la salida
void level::BuildLevel9()
{
	mainPlayer.SetSpawn({ 100, 872 });

	// --- Seccion 1: plataformas + enemigo a distancia ---
	platforms.push_back({ 0, 1000, 1920, 80 });
	platforms.push_back({ 300, 800, 220, 30 }); //SA
	platforms.push_back({ 540, 620, 220, 30 }); //SB

	weaponPickup sword; sword.Init({ 300, 952 }, weaponType::SWORD, swordPickupTexture); weaponPickups.push_back(sword);
	weaponPickup staff; staff.Init({ 480, 952 }, weaponType::STAFF, staffPickupTexture); weaponPickups.push_back(staff);

	enemies.AddRanged({ 620,524 }, 560, 740, 50, 650, 1.4f); //sobre SB

	// --- Seccion 2: trampas + mecanismo ---
	hazard spikes; spikes.Init({ 780, 970, 110, 30 }, 0); hazards.push_back(spikes);

	platforms.push_back({ 900, 880, 150, 30 }); //escalon con el interruptor
	button mechButton;
	mechButton.Init({ 940, 860, 60, 20 }, 0);
	buttons.push_back(mechButton);

	hazard timedTrap; timedTrap.Init({ 1010, 970, 60, 30 }, 1.5f); hazards.push_back(timedTrap);

	door door0;
	door0.Init({ 1080, 400, 30, 600 });
	doors.push_back(door0); //index 0: da acceso a la seccion 3

	// --- Seccion 3/4: combate con varios tipos + plataforma movil + piedra roja/verde ---
	arenaBounds = { 1090, 600, 480, 400 };

	movingPlatform shuttle;
	shuttle.Init({ 1150, 800, 180, 30 }, { 250, 0 }, 110);
	movingPlatforms.push_back(shuttle);

	enemies.AddLinear({ 1200,936 }, 1150, 1450, 110);
	enemies.AddCircular({ 1300,700 }, 120, 1.8f);
	enemies.AddRanged({ 1150,904 }, 1150, 1350, 45, 600, 1.4f);

	//perro suelto: recorre todo el mapa, amenaza libre e impredecible (a proposito NO cuenta
	//para el objetivo de la seccion 3, que necesita enemigos con posicion acotada)
	enemies.AddAccelerated({ 400,920 }, 380, 300);

	stone objective;
	objective.Init({ 1330, 300 });
	stones.push_back(objective);

	door door1;
	door1.Init({ 1580, 400, 30, 600 });
	doors.push_back(door1); //index 1: se abre al limpiar la seccion 3

	// --- Seccion 5: recorrido final hacia la salida ---
	door exitGate;
	exitGate.Init({ 1850, 400, 30, 600 });
	exitGate.Unlock(); //la seccion 4 ya controla el acceso; la salida en si no necesita otro candado
	doors.push_back(exitGate); //index 2
	exitDoorIndex = 2;

	item i1; i1.Init({ 250,780 }, 14); items.push_back(i1);
	item i2; i2.Init({ 1250,760 }, 14); items.push_back(i2);
	item i3; i3.Init({ 1780,950 }, 14); items.push_back(i3);
}

//Nivel 10 - Maximo desafio: progresion interna clara (inicio moderado -> escalada alta con un
//puzzle de dos interruptores que exige usar una plataforma movil con precision -> tramo previo
//con trampa -> combate final maximo con los 4 tipos de enemigo + un segundo perro suelto +
//una trampa temporizada dentro de la propia arena -> salida). Usa practicamente todas las
//mecanicas del juego: perros, plataformas, plataforma movil, puertas, interruptores, trampas,
//piedra roja/verde, combate, armas y una seccion vertical.
void level::BuildLevel10()
{
	mainPlayer.SetSpawn({ 100, 872 });

	// --- Inicio: dificultad moderada ---
	item introItem; introItem.Init({ 200,950 }, 14); items.push_back(introItem);
	platforms.push_back({ 0, 1000, 1920, 80 });
	enemies.AddAccelerated({ 700,920 }, 380, 300); //perro suelto desde el inicio

	// --- Escalada: dificultad alta. Puzzle de DOS interruptores: uno en la cima de la
	//     escalera y el otro solo alcanzable abordando una plataforma movil con timing ---
	platforms.push_back({ 550, 850, 220, 30 }); //C1
	platforms.push_back({ 850, 720, 220, 30 }); //C2
	platforms.push_back({ 550, 590, 220, 30 }); //C3
	platforms.push_back({ 850, 460, 220, 30 }); //C4: primer interruptor
	platforms.push_back({ 1150, 380, 130, 26 }); //Ciso: segundo interruptor, aislada (termina antes de la puerta)

	enemies.AddRanged({ 870,624 }, 870, 1050, 45, 600, 1.4f); //en C2, durante la escalada

	hazard toggling;
	toggling.Init({ 560, 570, 120, 20 }, 1.4f); //en C3
	hazards.push_back(toggling);

	movingPlatform shuttle;
	shuttle.Init({ 900, 420, 140, 26 }, { 230, 0 }, 90); //conecta C4 con Ciso
	movingPlatforms.push_back(shuttle);

	button topButtonA;
	topButtonA.Init({ 870, 440, 60, 20 }, -1); //en C4
	buttons.push_back(topButtonA);

	button topButtonB;
	topButtonB.Init({ 1200, 360, 60, 20 }, -1); //en Ciso: exige usar la plataforma movil
	buttons.push_back(topButtonB);

	//la puerta arranca en y=400 y Ciso termina en x=1280, asi que ninguna plataforma de la
	//escalada queda por encima ni cruzando el mecanismo: no se puede saltear saltando por arriba
	door mechanismGate;
	mechanismGate.Init({ 1300, 400, 30, 600 });
	doors.push_back(mechanismGate); //index 0: exige ambos interruptores (ver UpdateObjectives)

	// --- Tramo previo al desafio final ---
	hazard preArenaSpikes;
	preArenaSpikes.Init({ 1330, 970, 60, 30 }, 0);
	hazards.push_back(preArenaSpikes);

	// --- Desafio final: dificultad maxima ---
	platforms.push_back({ 1500, 800, 260, 30 }); //Pfinal, dentro de la arena final

	arenaBounds = { 1330, 600, 570, 400 };

	stone finalObjective;
	finalObjective.Init({ 1600, 300 }); //centrada arriba de la arena final, bien visible
	stones.push_back(finalObjective);

	hazard finalTrap;
	finalTrap.Init({ 1500, 970, 140, 30 }, 1.6f); //trampa temporizada dentro de la propia arena
	hazards.push_back(finalTrap);

	enemies.AddLinear({ 1450,936 }, 1380, 1650, 130);
	enemies.AddCircular({ 1600,700 }, 140, 2.0f);
	enemies.AddRanged({ 1520,704 }, 1520, 1720, 55, 650, 1.2f);
	//segundo perro: amenaza libre adicional durante el combate final (a proposito no cuenta
	//para el objetivo de la arena, que necesita enemigos con posicion acotada)
	enemies.AddAccelerated({ 1700,920 }, 420, 340);

	// --- Salida ---
	door exitGate;
	exitGate.Init({ 1880, 400, 30, 600 });
	doors.push_back(exitGate); //index 1: se abre al limpiar la arena final
	exitDoorIndex = 1;

	item finalItem; finalItem.Init({ 1800,950 }, 14); items.push_back(finalItem);
}

//Nivel 11 - Jefe final: arena de combate contra el jefe. 3 ciclos identicos de
//"esquivar 30s -> fase vulnerable -> daño" (ver clase boss). Al derrotarlo se abre la salida
//y, al atravesarla, el juego queda marcado como completado (no hay Nivel 12).
void level::BuildLevel11()
{
	mainPlayer.SetSpawn({ 860, 872 });

	//arena cerrada: suelo + dos plataformas bajas para reposicionarse durante la esquiva
	//y para poder alcanzar al jefe cuerpo a cuerpo durante la fase vulnerable
	platforms.push_back({ 0, 1000, 1920, 80 });
	platforms.push_back({ 260, 880, 220, 24 });
	platforms.push_back({ 1440, 880, 220, 24 });

	//red de seguridad: si por algun motivo el jugador llega sin arma equipada, puede tomar una aca
	weaponPickup sword; sword.Init({ 700, 952 }, weaponType::SWORD, swordPickupTexture); weaponPickups.push_back(sword);
	weaponPickup staff; staff.Init({ 1150, 952 }, weaponType::STAFF, staffPickupTexture); weaponPickups.push_back(staff);

	//--- configuracion del jefe (equivalente al "inspector": valores nombrados aca, no perdidos
	//    dentro de la logica de estados de la clase boss) ---
	float bossAttackPhaseSeconds = 30.0f; //duracion de cada fase de esquiva
	int bossCyclesRequired = 3;           //cantidad de ciclos para derrotarlo (nunca hay un 4to)
	int bossMaxHP = 300;                  //vida total, repartida en partes iguales entre los ciclos

	levelBoss.Init({ 860, 550 }, { 200, 200 }, bossAttackPhaseSeconds, bossCyclesRequired, bossMaxHP);
	hasBoss = true;
	lastBossState = bossState::INTRO;

	door exitGate;
	exitGate.Init({ 1850, 400, 30, 600 });
	doors.push_back(exitGate); //se desbloquea recien cuando el jefe es derrotado
	exitDoorIndex = 0;
}

// ---------------------------------------------------------------------------
// Objetivos: logica de desbloqueo propia de cada nivel (centralizada aca)
// ---------------------------------------------------------------------------

void level::UpdateObjectives(float dt)
{
	(void)dt;

	switch (currentLevelIndex)
	{
	case 0: //nivel de prueba: la piedra es solo decorativa, no bloquea nada
		if (!stones.empty())
		{
			stones[0].SetCleared(enemies.CountAlive() == 0);
		}
		break;

	case 1: //combate basico: derrotar al enemigo abre la salida
	case 3: //nuevo enemigo: limpiar la zona abre la salida
	case 6: //trampas: derrotar al guardia final abre la salida
		if (exitDoorIndex >= 0 && enemies.CountAlive() == 0)
		{
			doors[exitDoorIndex].Unlock();
		}
		break;

	case 4: //piedra roja/verde
	{
		bool clear = enemies.CountAlive() == 0;
		if (!stones.empty()) stones[0].SetCleared(clear);
		if (clear && exitDoorIndex >= 0) doors[exitDoorIndex].Unlock();
		break;
	}

	case 7: //arena de la seccion 1 (puerta 0) con piedra roja/verde; la salida (puerta 1) la maneja el boton generico
	{
		bool clear7 = enemies.CountAliveInRect(arenaBounds) == 0;
		if (!stones.empty()) stones[0].SetCleared(clear7);
		if (doors.size() > 0 && doors[0].IsLocked() && clear7)
		{
			doors[0].Unlock();
		}
		break;
	}

	//Nivel 8: la secuencia de interruptores ahora es una cadena de puertas 1 a 1 (explorar en
	//orden), maneja todo el bucle generico de botones -> no hace falta logica especial aca.

	case 9: //seccion 3/4: la piedra refleja la arena, que al limpiarse abre la puerta 1
	{
		bool clear9 = enemies.CountAliveInRect(arenaBounds) == 0;
		if (!stones.empty()) stones[0].SetCleared(clear9);

		if (doors.size() > 1 && doors[1].IsLocked() && clear9)
		{
			doors[1].Unlock();
		}
		break;
	}

	case 10: //la puerta del mecanismo (0) exige los DOS interruptores de la escalada (uno solo
	{        //alcanzable con la plataforma movil); la salida (1) exige limpiar la arena final
		if (doors.size() > 0 && doors[0].IsLocked() && buttons.size() >= 2)
		{
			if (buttons[0].IsPressed() && buttons[1].IsPressed())
			{
				doors[0].Unlock();
			}
		}

		bool clear10 = enemies.CountAliveInRect(arenaBounds) == 0;
		if (!stones.empty()) stones[0].SetCleared(clear10);
		if (exitDoorIndex >= 0 && doors[exitDoorIndex].IsLocked() && clear10)
		{
			doors[exitDoorIndex].Unlock();
		}
		break;
	}

	case 11: //jefe final: la salida se desbloquea recien cuando el jefe queda derrotado
		if (hasBoss && exitDoorIndex >= 0 && doors[exitDoorIndex].IsLocked() && levelBoss.IsDefeated())
		{
			doors[exitDoorIndex].Unlock();
		}
		break;

	default:
		break;
	}
}

// ---------------------------------------------------------------------------
// Gameplay
// ---------------------------------------------------------------------------

void level::HandleAttack()
{
	const int BOSS_SWORD_DAMAGE = 25;
	const float STAFF_SHOT_SPEED = 700.0f;
	//tamaño del proyectil magico (hitbox y dibujo). Con 64 px alcanza a tocar a los slimes que estan en el piso:
	//la bola sale a la altura del pecho y su borde inferior llega por debajo del borde superior del slime
	const float STAFF_SHOT_SIZE = 64.0f;

	//el arma avisa en que frame de su animacion pasa cada cosa: aca se aplica el efecto
	weaponEvent ev;
	while (mainPlayer.ConsumeWeaponEvent(ev))
	{
		if (ev.kind == weaponEventType::SWING)
		{
			//arranca el tajo: suena el SliceSword de ese golpe del combo
			sounds.PlaySword(ev.comboStep);
		}
		else if (ev.kind == weaponEventType::SWORD_HIT)
		{
			//la hoja conecta: daño en el area de ese golpe (el 3ro es mas grande y de mas alcance)
			//golpe comun = 1 de daño; el golpe final del combo (damageMul 2) = 2 de daño
			int hits = 0;
			int killed = enemies.AttackRect(ev.hitRect, (int)ev.damageMul, &hits);
			if (killed > 0) score += killed * 20;
			if (hits > 0) sounds.PlayHit();

			//jefe final: solo se le puede pegar mientras esta en su fase vulnerable
			//onda horizontal del jefe: el tajo rompe solo los segmentos que toca su zona de daño
			if (hasBoss && levelBoss.BreakWave(ev.hitRect) > 0)
			{
				sounds.PlayHit();
			}

			if (hasBoss && levelBoss.CanBeDamaged() && Collisions::CheckAABB(ev.hitRect, levelBoss.GetHitbox()))
			{
				levelBoss.TakeDamage((int)(BOSS_SWORD_DAMAGE * ev.damageMul));
				sounds.PlayHit();
			}
		}
		else if (ev.kind == weaponEventType::STAFF_SHOT)
		{
			//el disparo nace en la gema del baston. El sprite del fuego tiene la cabeza ~8.5px adelante
			//de su centro, asi que se retrocede esa distancia para que la bolita arranque justo en la gema
			//(la distancia crece con el tamaño del proyectil, que se dibuja escalado)
			float dir = ev.facingLeft ? -1.0f : 1.0f;
			Vector2 center = { ev.muzzle.x - dir * 8.5f * (STAFF_SHOT_SIZE / 32.0f), ev.muzzle.y };

			proyectile shot;
			shot.InitFire(center, { dir * STAFF_SHOT_SPEED, 0 }, STAFF_SHOT_SIZE, fireLaunchTexture, fireFlightTexture);
			projectiles.push_back(shot);
			sounds.PlayShoot();
		}
	}
}
void level::UpdateGameplay(float dt)
{
	elapsedTime += dt;

	//animacion de apertura de las puertas desbloqueadas
	for (unsigned int i = 0; i < doors.size(); i++)
	{
		doors[i].Update(dt);
	}

	//plataformas moviles: se actualizan primero y arrastran (translate) a quien este parado
	//encima, usando su posicion ANTES de moverse para detectar quien la esta pisando
	for (unsigned int i = 0; i < movingPlatforms.size(); i++)
	{
		Rectangle beforeRect = movingPlatforms[i].GetHitbox();
		bool carriesPlayer = Collisions::IsRestingOn(mainPlayer.GetHitbox(), beforeRect);

		movingPlatforms[i].Update(dt);
		Vector2 delta = movingPlatforms[i].GetDelta();

		if (carriesPlayer)
		{
			mainPlayer.Translate(delta);
		}
		enemies.CarryRiders(beforeRect, delta);
	}

	//plataformas fijas y moviles: se pisan desde arriba pero se atraviesan desde abajo (no golpean la cabeza).
	//Las puertas cerradas, en cambio, son paredes macizas.
	std::vector<Rectangle> oneWay = platforms;
	for (unsigned int i = 0; i < movingPlatforms.size(); i++)
	{
		oneWay.push_back(movingPlatforms[i].GetHitbox());
	}
	std::vector<Rectangle> solids;
	for (unsigned int i = 0; i < doors.size(); i++)
	{
		if (doors[i].IsLocked())
		{
			solids.push_back(doors[i].GetHitbox());
		}
	}

	//la puerta de salida desbloqueada abre el borde derecho de la pantalla para poder salir por ahi
	float worldRight = (float)GetScreenWidth();
	if (exitDoorIndex >= 0 && exitDoorIndex < (int)doors.size() && !doors[exitDoorIndex].IsLocked())
	{
		worldRight = (float)GetScreenWidth() + 300.0f;
	}

	mainPlayer.Update(dt, solids.data(), (int)solids.size(), oneWay.data(), (int)oneWay.size(), worldRight);
	//los enemigos conocen las paredes (puertas cerradas): no las atraviesan ni ven al jugador a traves de ellas
	enemyWorld enemyView;
	enemyView.playerPos = mainPlayer.GetPosition();
	Rectangle playerBody = mainPlayer.GetHitbox();
	enemyView.playerCenter = { playerBody.x + playerBody.width / 2, playerBody.y + playerBody.height / 2 };
	enemyView.walls = &solids;
	enemies.Update(dt, enemyView);

	if (hasBoss)
	{
		levelBoss.Update(dt, mainPlayer.GetPosition());

		//sonido al cambiar de fase, una sola vez por transicion (el jefe no sabe nada de audio)
		bossState currentBossState = levelBoss.GetState();
		if (currentBossState != lastBossState)
		{
			if (currentBossState == bossState::VULNERABLE_PHASE) sounds.PlayButton();
			else if (currentBossState == bossState::DEATH) sounds.PlayVictory();
			lastBossState = currentBossState;
		}
	}

	//el jugador se cae fuera del mapa: pierde una vida y respawnea
	if (mainPlayer.GetPosition().y > GetScreenHeight() + 100 && !mainPlayer.IsInvulnerable())
	{
		mainPlayer.TakeDamage();
		mainPlayer.Reset();
		sounds.PlayHit();
	}

	//colision jugador-enemigo (contacto)
	if (!mainPlayer.IsInvulnerable() && enemies.CheckPlayerCollision(mainPlayer.GetHitbox()))
	{
		mainPlayer.TakeDamage();
		sounds.PlayHit();
	}

	Rectangle playerHitbox = mainPlayer.GetHitbox();

	//peligros ambientales (picos / trampas temporizadas)
	for (unsigned int i = 0; i < hazards.size(); i++)
	{
		hazards[i].Update(dt);
		if (!mainPlayer.IsInvulnerable() && hazards[i].CheckDamage(playerHitbox))
		{
			mainPlayer.TakeDamage();
			sounds.PlayHit();
		}
	}

	//colision jugador-items
	for (unsigned int i = 0; i < items.size(); i++)
	{
		if (items[i].Update(dt, playerHitbox))
		{
			score += 10;
			sounds.PlayCoin();
		}
	}

	//armas: hay que pararse encima y apretar E para recogerlas. Se intercambia con la que se tenga
	//equipada (si habia alguna, queda tirada en el mismo lugar en vez de desaparecer)
	for (unsigned int i = 0; i < weaponPickups.size(); i++)
	{
		if (weaponPickups[i].CheckPickup(playerHitbox))
		{
			weaponType pickedType = weaponPickups[i].GetType();
			weaponType previousType = mainPlayer.GetWeaponType();

			mainPlayer.EquipWeapon(pickedType);
			sounds.PlayEquip();

			if (previousType != weaponType::NONE)
			{
				Texture2D previousTexture = (previousType == weaponType::SWORD) ? swordPickupTexture : staffPickupTexture;
				weaponPickups[i].ReplaceWith(previousType, previousTexture);
			}

			break; //un solo arma por pulsacion de E
		}
	}

	//botones que desbloquean directamente su puerta asociada (mecanismos simples 1 a 1)
	for (unsigned int i = 0; i < buttons.size(); i++)
	{
		if (buttons[i].CheckPressed(playerHitbox))
		{
			int doorIndex = buttons[i].GetLinkedDoorIndex();
			if (doorIndex >= 0 && doorIndex < (int)doors.size())
			{
				doors[doorIndex].Unlock();
			}
			sounds.PlayButton();
		}
	}

	//condiciones de desbloqueo propias de cada nivel (piedras, arenas, puzzles de varios botones)
	UpdateObjectives(dt);

	HandleAttack();

	//un enemigo a distancia decide disparar este frame
	Vector2 shotPos;
	bool shotFacingLeft;
	if (enemies.CollectShot(shotPos, shotFacingLeft))
	{
		//fuego azul en vuelo (sin lanzamiento: no sale de un arma). shotPos es la esquina del cuadrado de 32
		Texture2D noLaunch = { 0 };
		Vector2 shotCenter = { shotPos.x + 16, shotPos.y + 16 };
		float shotSpeed = shotFacingLeft ? -700.0f : 700.0f;

		proyectile enemyShot;
		enemyShot.InitFire(shotCenter, { shotSpeed, 0 }, 32.0f, noLaunch, enemyFlightTexture);
		enemyProjectiles.push_back(enemyShot);
	}

	//ataques del jefe (tiros dirigidos y rafagas): se convierten en proyectiles normales de fuego
	//y se suman al mismo sistema de proyectiles enemigos que ya danan al jugador.
	//(la onda horizontal NO pasa por aca: son segmentos propios del jefe, ver boss::BreakWave)
	if (hasBoss)
	{
		std::vector<bossProjectileRequest> bossRequests;
		if (levelBoss.CollectAttacks(bossRequests) > 0)
		{
			for (unsigned int i = 0; i < bossRequests.size(); i++)
			{
				proyectile bossShot;
				Texture2D noLaunch = { 0 };
				Vector2 reqCenter = { bossRequests[i].pos.x + bossRequests[i].size.x / 2, bossRequests[i].pos.y + bossRequests[i].size.y / 2 };
				bossShot.InitFire(reqCenter, bossRequests[i].vel, bossRequests[i].size.x, noLaunch, fireFlightTexture);
				enemyProjectiles.push_back(bossShot);
			}
		}

		//onda horizontal: cada segmento vivo lastima al jugador al tocarlo. El segmento que lo golpeo se rompe
		//(igual que un proyectil que da en el blanco), asi un mismo paso por la pared cuenta como un solo golpe
		if (!mainPlayer.IsInvulnerable() && levelBoss.WaveTouches(playerHitbox))
		{
			levelBoss.BreakWave(playerHitbox);
			mainPlayer.TakeDamage();
			sounds.PlayHit();
		}
	}

	const int BOSS_STAFF_DAMAGE = 12;

	//proyectiles del jugador: se mueven y matan al primer enemigo que tocan
	for (unsigned int i = 0; i < projectiles.size(); i++)
	{
		projectiles[i].Update(dt);
		if (projectiles[i].IsAlive())
		{
			int hits = 0;
			int killed = enemies.AttackRect(projectiles[i].GetHitbox(), 1, &hits); //cada proyectil = 1 de daño
			if (hits > 0)
			{
				projectiles[i].Kill();
				score += killed * 20;
				sounds.PlayHit();
			}

			//la bola de fuego del baston tambien rompe los segmentos que toca (y se consume en el impacto)
			if (hasBoss && projectiles[i].IsAlive() && levelBoss.BreakWave(projectiles[i].GetHitbox()) > 0)
			{
				projectiles[i].Kill();
				sounds.PlayHit();
			}

			if (hasBoss && projectiles[i].IsAlive() && levelBoss.CanBeDamaged() && Collisions::CheckAABB(projectiles[i].GetHitbox(), levelBoss.GetHitbox()))
			{
				projectiles[i].Kill();
				levelBoss.TakeDamage(BOSS_STAFF_DAMAGE);
				sounds.PlayHit();
			}
		}
	}
	projectiles.erase(std::remove_if(projectiles.begin(), projectiles.end(),
		[](proyectile& p) { return !p.IsAlive(); }), projectiles.end());

	//proyectiles enemigos: se mueven y danan al jugador
	for (unsigned int i = 0; i < enemyProjectiles.size(); i++)
	{
		enemyProjectiles[i].Update(dt);
		if (enemyProjectiles[i].IsAlive() && Collisions::CheckAABB(enemyProjectiles[i].GetHitbox(), playerHitbox))
		{
			enemyProjectiles[i].Kill();
			if (!mainPlayer.IsInvulnerable())
			{
				mainPlayer.TakeDamage();
				sounds.PlayHit();
			}
		}

		//los disparos enemigos (tiros dirigidos y rafagas del jefe, magia de los orcos) no atraviesan el piso, las
		//plataformas (fijas y moviles) ni las paredes: se destruyen al tocarlas
		if (enemyProjectiles[i].IsAlive())
		{
			Rectangle shotBox = enemyProjectiles[i].GetHitbox();
			bool blocked = false;
			for (unsigned int j = 0; j < oneWay.size() && !blocked; j++)
			{
				blocked = Collisions::CheckAABB(shotBox, oneWay[j]);
			}
			for (unsigned int j = 0; j < solids.size() && !blocked; j++)
			{
				blocked = Collisions::CheckAABB(shotBox, solids[j]);
			}
			if (blocked) enemyProjectiles[i].Kill();
		}
	}
	enemyProjectiles.erase(std::remove_if(enemyProjectiles.begin(), enemyProjectiles.end(),
		[](proyectile& p) { return !p.IsAlive(); }), enemyProjectiles.end());

	//el nivel se completa recien cuando el jugador atraviesa la puerta desbloqueada Y sale
	//fisicamente de la pantalla por el borde derecho (tocar la puerta ya no alcanza)
	if (exitDoorIndex >= 0 && exitDoorIndex < (int)doors.size() && !doors[exitDoorIndex].IsLocked())
	{
		if (mainPlayer.GetPosition().x > GetScreenWidth())
		{
			CompleteLevel();
		}
	}

	if (mainPlayer.IsDead())
	{
		state = levelState::GAMEOVER;
		sounds.PlayGameOver();
	}
}

// ---------------------------------------------------------------------------
// Dibujado
// ---------------------------------------------------------------------------

void level::DrawWorld()
{
	//fondo del castillo + antorchas y estandartes de la ambientacion actual
	Art::DrawBackground();

	for (unsigned int i = 0; i < platforms.size(); i++)
	{
		Art::DrawSolid(platforms[i]); //piso (tile repetido) o plataforma (tapas + medio)
	}
	for (unsigned int i = 0; i < movingPlatforms.size(); i++)
	{
		movingPlatforms[i].Draw();
	}

	for (unsigned int i = 0; i < hazards.size(); i++)
	{
		hazards[i].Draw();
	}
	for (unsigned int i = 0; i < stones.size(); i++)
	{
		stones[i].Draw();
	}
	for (unsigned int i = 0; i < buttons.size(); i++)
	{
		buttons[i].Draw();
	}
	for (unsigned int i = 0; i < doors.size(); i++)
	{
		doors[i].Draw();
	}

	for (unsigned int i = 0; i < items.size(); i++)
	{
		items[i].Draw();
	}
	for (unsigned int i = 0; i < weaponPickups.size(); i++)
	{
		weaponPickups[i].Draw();
	}

	Rectangle screenBounds = { 0,0,(float)GetScreenWidth(),(float)GetScreenHeight() };
	enemies.Draw(screenBounds);

	if (hasBoss)
	{
		levelBoss.Draw();
	}

	for (unsigned int i = 0; i < projectiles.size(); i++)
	{
		projectiles[i].Draw();
	}
	for (unsigned int i = 0; i < enemyProjectiles.size(); i++)
	{
		enemyProjectiles[i].Draw();
	}

	mainPlayer.Draw();

	//cartel "E - Recoger" sobre la cabeza del jugador cuando esta encima de un arma (va despues del jugador para que no lo tape)
	for (unsigned int i = 0; i < weaponPickups.size(); i++)
	{
		weaponPickups[i].DrawPrompt(mainPlayer.GetHitbox());
	}

	int collected = 0;
	for (unsigned int i = 0; i < items.size(); i++)
	{
		if (items[i].IsCollected()) collected++;
	}

	const char* weaponName = "Ninguna";
	if (mainPlayer.GetWeaponType() == weaponType::SWORD) weaponName = "Espada";
	else if (mainPlayer.GetWeaponType() == weaponType::STAFF) weaponName = "Baston magico";

	hudDisplay.Draw(mainPlayer.GetLives(), score, collected, itemsTotal, elapsedTime, weaponName);

	//la barra de vida del jefe se dibuja al final para que siempre quede encima del resto del mundo
	if (hasBoss)
	{
		levelBoss.DrawHUD();
	}

	const char* levelLabel = (currentLevelIndex == 0) ? "Nivel de prueba" : TextFormat("Nivel %d", currentLevelIndex);
	DrawText(levelLabel, GetScreenWidth() - MeasureText(levelLabel, 24) - 20, 60, 24, LIGHTGRAY);

	DrawText("Flechas/WASD: mover | Espacio: saltar | F: atacar | E: recoger", 20, GetScreenHeight() - 40, 20, LIGHTGRAY);
}
void level::DrawEndScreen(const char* title, Color color, const char* hint)
{
	DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.6f));

	int titleSize = 90;
	int titleWidth = MeasureText(title, titleSize);
	DrawText(title, GetScreenWidth() / 2 - titleWidth / 2, GetScreenHeight() / 2 - 150, titleSize, color);

	const char* scoreText = TextFormat("Puntaje: %d", score);
	int scoreWidth = MeasureText(scoreText, 40);
	DrawText(scoreText, GetScreenWidth() / 2 - scoreWidth / 2, GetScreenHeight() / 2 - 30, 40, RAYWHITE);

	int hintWidth = MeasureText(hint, 30);
	DrawText(hint, GetScreenWidth() / 2 - hintWidth / 2, GetScreenHeight() / 2 + 40, 30, RAYWHITE);
}
void level::DrawPauseButton(Rectangle b, const char* label, Vector2 mouse)
{
	bool hover = mouse.x >= b.x && mouse.y >= b.y && mouse.x <= b.x + b.width && mouse.y <= b.y + b.height;
	DrawRectangleRec(b, hover ? Fade(RAYWHITE, 0.9f) : Fade(RAYWHITE, 0.6f));
	DrawRectangleLinesEx(b, 2, BLACK);

	int size = 26;
	DrawText(label, (int)(b.x + b.width / 2 - MeasureText(label, size) / 2), (int)(b.y + b.height / 2 - size / 2), size, BLACK);
}
void level::DrawPauseMenu()
{
	//el mundo ya fue dibujado antes de llamar a esto: queda visible pero congelado detras del overlay
	DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.65f));

	const char* title = "PAUSA";
	int titleSize = 70;
	DrawText(title, GetScreenWidth() / 2 - MeasureText(title, titleSize) / 2, GetScreenHeight() / 2 - 220, titleSize, RAYWHITE);

	Vector2 mouse = GetMousePosition();
	DrawPauseButton(pauseContinueButton, "CONTINUAR", mouse);
	DrawPauseButton(pauseOptionsButton, "OPCIONES", mouse);
	DrawPauseButton(pauseMenuButton, "VOLVER AL MENU", mouse);
}
void level::PauseInputs()
{
	if (!IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) return;

	Vector2 m = GetMousePosition();

	if (m.x >= pauseContinueButton.x && m.y >= pauseContinueButton.y && m.x <= pauseContinueButton.x + pauseContinueButton.width && m.y <= pauseContinueButton.y + pauseContinueButton.height)
	{
		paused = false;
	}
	else if (m.x >= pauseOptionsButton.x && m.y >= pauseOptionsButton.y && m.x <= pauseOptionsButton.x + pauseOptionsButton.width && m.y <= pauseOptionsButton.y + pauseOptionsButton.height)
	{
		wantsOptions = true;
	}
	else if (m.x >= pauseMenuButton.x && m.y >= pauseMenuButton.y && m.x <= pauseMenuButton.x + pauseMenuButton.width && m.y <= pauseMenuButton.y + pauseMenuButton.height)
	{
		paused = false;
		exitToMenu = true;
	}
}
void level::Update()
{
	float dt = GetFrameTime();

	if (state == levelState::PLAYING)
	{
		//ESC alterna la pausa mientras se esta jugando; el resto de los estados no se pausan
		if (IsKeyPressed(KEY_ESCAPE))
		{
			paused = !paused;
		}

		if (paused)
		{
			PauseInputs();
		}
		else
		{
			UpdateGameplay(dt);
		}
	}
	else
	{
		//pantallas de fin (game over / nivel completado): R reintenta, ESC vuelve al menu
		if (IsKeyPressed(KEY_R))
		{
			Reset();
		}
		else if (IsKeyPressed(KEY_ESCAPE))
		{
			exitToMenu = true;
		}

		//cuenta regresiva para pasar solo al siguiente nivel (solo niveles 1-9)
		if (state == levelState::LEVEL_COMPLETE && completionTimer > 0)
		{
			completionTimer -= dt;
			if (completionTimer <= 0)
			{
				pendingAdvance = true;
			}
		}
	}

	DrawWorld();

	if (state == levelState::GAMEOVER)
	{
		DrawEndScreen("GAME OVER", RED, "Presiona R para reintentar - ESC para volver al menu");
	}
	else if (state == levelState::LEVEL_COMPLETE)
	{
		if (currentLevelIndex == 11) //ultimo nivel de la progresion (jefe final)
		{
			DrawEndScreen("JUEGO COMPLETADO", GOLD, "Presiona R para reintentar - ESC para volver al menu");
		}
		else
		{
			DrawEndScreen("NIVEL COMPLETADO", GREEN, "Cargando el siguiente nivel...");
		}
	}
	else if (state == levelState::PLAYING && paused)
	{
		DrawPauseMenu();
	}
}
void level::Unload()
{
	mainPlayer.Unload();
	enemies.Unload();
	sounds.Unload();
	UnloadTexture(swordPickupTexture);
	UnloadTexture(staffPickupTexture);
	UnloadTexture(fireLaunchTexture);
	UnloadTexture(fireFlightTexture);
	UnloadTexture(enemyFlightTexture);
	Art::Unload();
}
