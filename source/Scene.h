#pragma once
#include <iostream>
#include "raylib.h"
#include "Player.h"
#include "Enemy.h"
#include "Level.h"
#include "LevelSelect.h"
#include "SaveManager.h"
#include "SoundManager.h"

using namespace std;
class scene
{
public:
	scene();
	~scene();

	bool ShouldExit();


	void Inits();
	void Update();
private:
	int sceneN; //1=menu, 2=jugando, 3=selector de niveles, 4=opciones
	int fontSize;

	int musicVolume;
	int fxVolume;

	bool exitGame;
	bool canClick;

	Vector2 mousePos;


	Texture2D buttonTexture;
	Texture2D buttonLitRightTexture1;
	Texture2D buttonLitLeftTexture1;

	Rectangle playButton;
	Rectangle levelsButton;
	Rectangle optionButton;
	Rectangle exitButton;
	Rectangle volumeButton1;
	Rectangle volumeButton2;

	Music menuMusic;
	Music gameMusic;

	level gameLevel;
	bool levelLoaded;

	levelSelect selector;
	saveManager saveData;

	int optionsReturnTo; //a que sceneN volver al salir de Opciones (1=menu, 2=pausa dentro del nivel)

	//preview del volumen de FX en Opciones: soundManager propio de la escena, siempre disponible
	//(gameLevel puede no existir todavia si el jugador nunca entro a un nivel esta sesion)
	soundManager previewSounds;
	float fxPreviewCooldown; //evita que se acumulen previews si se clickea muy rapido

	void Menu();
	void Options();
	void Game();
	void LevelSelectScreen();
	void SetVolume();
	void PreviewFxVolume();

	void EnterLevel(int levelIndex);

	void Load();
	void Inputs();
};

