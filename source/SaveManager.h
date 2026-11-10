#pragma once
#include "Weapons.h"

const int TOTAL_LEVELS = 11; //niveles 1 a 11 (11 = jefe final; el nivel de prueba no forma parte de la progresion)

//progreso del jugador: niveles desbloqueados/completados, arma guardada entre niveles,
//y su persistencia en disco entre ejecuciones del juego (formato de texto simple)
class saveManager
{
public:
	saveManager();
	~saveManager();

	void Load();
	void Save();

	bool IsUnlocked(int levelNumber);    //levelNumber: 1..11
	bool IsCompleted(int levelNumber);
	void CompleteLevel(int levelNumber); //marca completado y desbloquea el siguiente

	void SetSavedWeapon(weaponType type);
	weaponType GetSavedWeapon();

	bool IsGameCompleted();
	void SetGameCompleted();

	//borra TODO el progreso: elimina el archivo de guardado y vuelve a los valores iniciales
	//(solo el nivel 1 desbloqueado, ninguno completado, sin arma). No toca volumenes ni otras opciones.
	void ResetProgress();

	//--- herramientas de debug: facil de quitar antes de la version final ---
	void DebugUnlockAll(); //desbloquea 1-11 sin marcarlos completados

private:
	bool unlocked[TOTAL_LEVELS];
	bool completed[TOTAL_LEVELS];
	weaponType savedWeapon;
	bool gameCompleted;

	void SetDefaults();
};
