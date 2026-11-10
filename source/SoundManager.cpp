#include "SoundManager.h"
#include <stdlib.h>
#include <math.h>

soundManager::soundManager()
{
	jumpSound = { 0 };
	coinSound = { 0 };
	hitSound = { 0 };
	victorySound = { 0 };
	gameOverSound = { 0 };
	shootSound = { 0 };
	buttonSound = { 0 };
	equipSound = { 0 };
	for (int i = 0; i < 3; i++) swordSounds[i] = { 0 };
}
soundManager::~soundManager()
{
}

//genera un tono sintetico (barrido de frecuencia) para no depender de archivos .wav externos
Sound soundManager::GenerateTone(float freqStart, float freqEnd, float duration, float volume)
{
	int sampleRate = 44100;
	int frameCount = (int)(duration * sampleRate);
	short* samples = (short*)malloc(frameCount * sizeof(short));
	if (samples == nullptr) return Sound{ 0 }; //sin memoria: devuelve un sonido vacio en vez de escribir en un puntero nulo

	for (int i = 0; i < frameCount; i++)
	{
		float t = (float)i / sampleRate;
		float freq = freqStart + (freqEnd - freqStart) * (t / duration);
		float envelope = 1.0f - (t / duration);
		float value = sinf(2.0f * PI * freq * t) * envelope * volume;
		samples[i] = (short)(value * 32000);
	}

	Wave wave = { 0 };
	wave.frameCount = frameCount;
	wave.sampleRate = sampleRate;
	wave.sampleSize = 16;
	wave.channels = 1;
	wave.data = samples;

	Sound sound = LoadSoundFromWave(wave);
	UnloadWave(wave);
	return sound;
}
void soundManager::Init()
{
	//los sonidos se generan por codigo porque todavia no hay archivos de audio propios.
	//si agregas un .wav/.ogg para alguno, comenta la linea de GenerateTone correspondiente
	//y descomenta/completa la de LoadSound con el nombre de archivo real.

	//jumpSound = LoadSound("Assets/Sounds/jump.wav");
	jumpSound = GenerateTone(350, 700, 0.15f, 0.5f);

	//coinSound = LoadSound("Assets/Sounds/coin.wav");
	coinSound = GenerateTone(800, 1400, 0.12f, 0.5f);

	//hitSound = LoadSound("Assets/Sounds/hit.wav");
	hitSound = GenerateTone(300, 100, 0.25f, 0.6f);

	//victorySound = LoadSound("Assets/Sounds/victory.wav");
	victorySound = GenerateTone(500, 1100, 0.6f, 0.5f);

	//gameOverSound = LoadSound("Assets/Sounds/gameover.wav");
	gameOverSound = GenerateTone(400, 120, 0.7f, 0.6f);

	//shootSound = LoadSound("Assets/Sounds/shoot.wav");
	shootSound = GenerateTone(900, 500, 0.1f, 0.45f);

	//buttonSound = LoadSound("Assets/Sounds/button.wav");
	buttonSound = GenerateTone(600, 650, 0.15f, 0.5f);

	//equipSound = LoadSound("Assets/Sounds/equip.wav");
	equipSound = GenerateTone(500, 950, 0.2f, 0.5f);

	//tajos de la espada: un archivo real por cada golpe del combo
	swordSounds[0] = LoadSound("Assets/Fx/SliceSword1.mp3");
	swordSounds[1] = LoadSound("Assets/Fx/SliceSword2.mp3");
	swordSounds[2] = LoadSound("Assets/Fx/SliceSword3.mp3");
}
void soundManager::Unload()
{
	UnloadSound(jumpSound);
	UnloadSound(coinSound);
	UnloadSound(hitSound);
	UnloadSound(victorySound);
	UnloadSound(gameOverSound);
	UnloadSound(shootSound);
	UnloadSound(buttonSound);
	UnloadSound(equipSound);
	for (int i = 0; i < 3; i++) UnloadSound(swordSounds[i]);
}
void soundManager::PlayJump() { PlaySound(jumpSound); }
void soundManager::PlayCoin() { PlaySound(coinSound); }
void soundManager::PlayHit() { PlaySound(hitSound); }
void soundManager::PlayVictory() { PlaySound(victorySound); }
void soundManager::PlayGameOver() { PlaySound(gameOverSound); }
void soundManager::PlayShoot() { PlaySound(shootSound); }
void soundManager::PlayButton() { PlaySound(buttonSound); }
void soundManager::PlayEquip() { PlaySound(equipSound); }
void soundManager::PlaySword(int comboStep)
{
	if (comboStep < 1 || comboStep > 3) return;
	PlaySound(swordSounds[comboStep - 1]);
}
void soundManager::SetVolume(float volume01)
{
	SetSoundVolume(jumpSound, volume01);
	SetSoundVolume(coinSound, volume01);
	SetSoundVolume(hitSound, volume01);
	SetSoundVolume(victorySound, volume01);
	SetSoundVolume(gameOverSound, volume01);
	SetSoundVolume(shootSound, volume01);
	SetSoundVolume(buttonSound, volume01);
	SetSoundVolume(equipSound, volume01);
	for (int i = 0; i < 3; i++) SetSoundVolume(swordSounds[i], volume01);
}
