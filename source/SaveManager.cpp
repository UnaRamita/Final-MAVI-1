#include "SaveManager.h"
#include <fstream>
#include <string>
#include <cstdlib>
#include <cstdio>

static const char* SAVE_FILE_PATH = "save.dat";

saveManager::saveManager()
{
	SetDefaults();
}
saveManager::~saveManager()
{
}
void saveManager::SetDefaults()
{
	for (int i = 0; i < TOTAL_LEVELS; i++)
	{
		unlocked[i] = false;
		completed[i] = false;
	}
	unlocked[0] = true; //nivel 1 desbloqueado desde el principio
	savedWeapon = weaponType::NONE;
	gameCompleted = false;
}
void saveManager::Load()
{
	SetDefaults();

	std::ifstream file(SAVE_FILE_PATH);
	if (!file.is_open()) return; //no hay partida guardada todavia: se usan los valores por defecto

	std::string line;
	while (std::getline(file, line))
	{
		size_t sep = line.find('=');
		if (sep == std::string::npos) continue;

		std::string key = line.substr(0, sep);
		std::string value = line.substr(sep + 1);

		if (key == "UNLOCKED")
		{
			for (int i = 0; i < TOTAL_LEVELS && i < (int)value.size(); i++)
			{
				unlocked[i] = (value[i] == '1');
			}
		}
		else if (key == "COMPLETED")
		{
			for (int i = 0; i < TOTAL_LEVELS && i < (int)value.size(); i++)
			{
				completed[i] = (value[i] == '1');
			}
		}
		else if (key == "WEAPON")
		{
			savedWeapon = (weaponType)atoi(value.c_str());
		}
		else if (key == "GAMECOMPLETED")
		{
			gameCompleted = (value == "1");
		}
	}
}
void saveManager::Save()
{
	std::ofstream file(SAVE_FILE_PATH);
	if (!file.is_open()) return;

	file << "UNLOCKED=";
	for (int i = 0; i < TOTAL_LEVELS; i++) file << (unlocked[i] ? '1' : '0');
	file << "\n";

	file << "COMPLETED=";
	for (int i = 0; i < TOTAL_LEVELS; i++) file << (completed[i] ? '1' : '0');
	file << "\n";

	file << "WEAPON=" << (int)savedWeapon << "\n";
	file << "GAMECOMPLETED=" << (gameCompleted ? 1 : 0) << "\n";
}
bool saveManager::IsUnlocked(int levelNumber)
{
	int i = levelNumber - 1;
	if (i < 0 || i >= TOTAL_LEVELS) return false;
	return unlocked[i];
}
bool saveManager::IsCompleted(int levelNumber)
{
	int i = levelNumber - 1;
	if (i < 0 || i >= TOTAL_LEVELS) return false;
	return completed[i];
}
void saveManager::CompleteLevel(int levelNumber)
{
	int i = levelNumber - 1;
	if (i < 0 || i >= TOTAL_LEVELS) return;

	completed[i] = true;
	unlocked[i] = true;

	//completar un nivel desbloquea el siguiente
	if (i + 1 < TOTAL_LEVELS)
	{
		unlocked[i + 1] = true;
	}

	if (levelNumber == TOTAL_LEVELS)
	{
		gameCompleted = true;
	}

	Save();
}
void saveManager::SetSavedWeapon(weaponType type)
{
	savedWeapon = type;
}
weaponType saveManager::GetSavedWeapon()
{
	return savedWeapon;
}
bool saveManager::IsGameCompleted()
{
	return gameCompleted;
}
void saveManager::SetGameCompleted()
{
	gameCompleted = true;
	Save();
}
void saveManager::ResetProgress()
{
	SetDefaults();                 //estado en memoria: igual que una instalacion nueva
	std::remove(SAVE_FILE_PATH);   //y se elimina el archivo (si no existe, no pasa nada)
}
void saveManager::DebugUnlockAll()
{
	//--- HERRAMIENTA DE DEBUG: quitar este metodo (y su boton en levelSelect) antes de la version final ---
	for (int i = 0; i < TOTAL_LEVELS; i++)
	{
		unlocked[i] = true;
	}
	Save();
}
