#pragma once
#include "raylib.h"
#include "SaveManager.h"

//pantalla de seleccion de niveles: dibuja el estado (bloqueado/desbloqueado/completado)
//de cada nivel leyendo directamente el saveManager que le pasa scene (no duplica el progreso).
class levelSelect
{
public:
	levelSelect();
	~levelSelect();

	void Init();
	void Update(saveManager& save);
	void Unload();

	bool WantsExitToMenu();
	int ConsumeSelectedLevel(); //-1 si no se eligio nada este frame; si no, 0=prueba, 1..11=nivel

private:
	void Inputs(saveManager& save);
	void Draw(saveManager& save);
	void DrawConfirmReset();

	Rectangle levelButtons[12]; //0 = nivel de prueba, 1..11 = progresion (11 = jefe final)
	Rectangle unlockAllButton;  //--- HERRAMIENTA DE DEBUG: quitar antes de la version final ---
	Rectangle resetButton;      //"Borrar progreso y empezar de cero"
	Rectangle backButton;

	//confirmacion del borrado de progreso (ventana modal: mientras esta abierta, el resto de la pantalla no responde)
	bool confirmingReset;
	Rectangle confirmPanel;
	Rectangle confirmYesButton;
	Rectangle confirmNoButton;

	Texture2D buttonTexture;

	Vector2 mousePos;
	int selectedLevel;
	bool exitToMenu;
};
