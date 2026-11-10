#pragma once
#include "raylib.h"

//gestiona los efectos de sonido del gameplay (la musica se maneja en Scene)
class soundManager
{
public:
	soundManager();
	~soundManager();

	void Init();
	void Unload();

	void PlayJump();
	void PlayCoin();
	void PlayHit();
	void PlayVictory();
	void PlayGameOver();
	void PlayShoot();
	void PlayButton();
	void PlayEquip();
	void PlaySword(int comboStep); //tajo de la espada: 1, 2 o 3 (un sonido distinto por golpe del combo)

	void SetVolume(float volume01);

private:
	Sound GenerateTone(float freqStart, float freqEnd, float duration, float volume);

	Sound jumpSound;
	Sound coinSound;
	Sound hitSound;
	Sound victorySound;
	Sound gameOverSound;
	Sound shootSound;
	Sound buttonSound;
	Sound equipSound;
	Sound swordSounds[3];
};
