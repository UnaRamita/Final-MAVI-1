#include "LevelSelect.h"
#include "Art.h"

levelSelect::levelSelect()
{
	for (int i = 0; i < 12; i++) levelButtons[i] = { 0,0,0,0 };
	unlockAllButton = { 0,0,0,0 };
	resetButton = { 0,0,0,0 };
	backButton = { 0,0,0,0 };
	confirmingReset = false;
	confirmPanel = { 0,0,0,0 };
	confirmYesButton = { 0,0,0,0 };
	confirmNoButton = { 0,0,0,0 };
	buttonTexture = { 0 };
	mousePos = { 0,0 };
	selectedLevel = -1;
	exitToMenu = false;
}
levelSelect::~levelSelect()
{
}
void levelSelect::Init()
{
	buttonTexture = LoadTexture("Assets/Sprites/Others/StoneButton.png");

	int cols = 4;
	int rows = 3;
	float buttonW = 320;
	float buttonH = 100;
	float gapX = 40;
	float gapY = 30;

	float totalW = cols * buttonW + (cols - 1) * gapX;
	float startX = GetScreenWidth() / 2.0f - totalW / 2.0f;
	float startY = 200;

	for (int i = 0; i < 12; i++)
	{
		int col = i % cols;
		int row = i / cols;
		levelButtons[i] = { startX + col * (buttonW + gapX), startY + row * (buttonH + gapY), buttonW, buttonH };
	}

	float gridBottom = startY + rows * buttonH + (rows - 1) * gapY;
	unlockAllButton = { GetScreenWidth() / 2.0f - 300, gridBottom + 40, 600, 70 };
	resetButton = { GetScreenWidth() / 2.0f - 300, gridBottom + 130, 600, 70 };
	backButton = { GetScreenWidth() / 2.0f - 150, gridBottom + 220, 300, 70 };

	//ventana de confirmacion centrada en la pantalla
	confirmPanel = { GetScreenWidth() / 2.0f - 440, GetScreenHeight() / 2.0f - 200, 880, 400 };
	confirmYesButton = { confirmPanel.x + 50, confirmPanel.y + confirmPanel.height - 110, 380, 70 };
	confirmNoButton = { confirmPanel.x + confirmPanel.width - 50 - 380, confirmPanel.y + confirmPanel.height - 110, 380, 70 };
	confirmingReset = false;

	selectedLevel = -1;
	exitToMenu = false;
}
void levelSelect::Inputs(saveManager& save)
{
	mousePos = GetMousePosition();

	//con la confirmacion abierta solo responden sus dos botones (y ESC = cancelar); no se toca nada del progreso hasta confirmar
	if (confirmingReset)
	{
		if (IsKeyPressed(KEY_ESCAPE))
		{
			confirmingReset = false;
		}
		else if (CheckCollisionPointRec(mousePos, confirmNoButton) && IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
		{
			confirmingReset = false;
		}
		else if (CheckCollisionPointRec(mousePos, confirmYesButton) && IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
		{
			//se borra el progreso (archivo + estado en memoria) y arranca una partida nueva en el nivel 1 sin arma
			save.ResetProgress();
			confirmingReset = false;
			selectedLevel = 1;
		}
		return;
	}

	if (CheckCollisionPointRec(mousePos, resetButton) && IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
	{
		confirmingReset = true;
		return;
	}

	for (int i = 0; i < 12; i++)
	{
		bool unlocked = (i == 0) || save.IsUnlocked(i);
		if (!unlocked) continue;

		Rectangle b = levelButtons[i];
		if (mousePos.x >= b.x && mousePos.y >= b.y && mousePos.x <= b.x + b.width && mousePos.y <= b.y + b.height
			&& IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
		{
			selectedLevel = i;
		}
	}

	Rectangle u = unlockAllButton;
	if (mousePos.x >= u.x && mousePos.y >= u.y && mousePos.x <= u.x + u.width && mousePos.y <= u.y + u.height
		&& IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
	{
		//--- HERRAMIENTA DE DEBUG: desbloquea 1-11 para poder probarlos libremente ---
		save.DebugUnlockAll();
	}

	Rectangle back = backButton;
	if (mousePos.x >= back.x && mousePos.y >= back.y && mousePos.x <= back.x + back.width && mousePos.y <= back.y + back.height
		&& IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
	{
		exitToMenu = true;
	}
}
void levelSelect::Draw(saveManager& save)
{
	Art::DrawMenuBackground();

	const char* screenTitle = "SELECCIONAR NIVEL";
	int titleSize = 50;
	DrawText(screenTitle, GetScreenWidth() / 2 - MeasureText(screenTitle, titleSize) / 2, 60, titleSize, RAYWHITE);

	Rectangle srcFull = { 0,0,(float)buttonTexture.width,(float)buttonTexture.height };

	for (int i = 0; i < 12; i++)
	{
		Rectangle b = levelButtons[i];
		bool isTest = (i == 0);
		bool unlocked = isTest || save.IsUnlocked(i);
		bool completed = !isTest && save.IsCompleted(i);

		Color tint = RAYWHITE;
		if (!unlocked) tint = Fade(GRAY, 0.55f);
		else if (completed) tint = Fade(GREEN, 0.55f);
		else if (isTest) tint = Fade(SKYBLUE, 0.5f);

		DrawTexturePro(buttonTexture, srcFull, b, { 0,0 }, 0, tint);

		const char* label = isTest ? "Prueba" : TextFormat("Nivel %d", i);
		int labelSize = 28;
		Color labelColor = unlocked ? BLACK : DARKGRAY;
		DrawText(label, (int)(b.x + b.width / 2 - MeasureText(label, labelSize) / 2), (int)(b.y + b.height / 2 - labelSize - 2), labelSize, labelColor);

		const char* statusText;
		Color statusColor;
		if (!unlocked) { statusText = "Bloqueado"; statusColor = MAROON; }
		else if (completed) { statusText = "Completado"; statusColor = DARKGREEN; }
		else if (isTest) { statusText = "Debug"; statusColor = DARKBLUE; }
		else { statusText = "Disponible"; statusColor = DARKGRAY; }

		int statusSize = 18;
		DrawText(statusText, (int)(b.x + b.width / 2 - MeasureText(statusText, statusSize) / 2), (int)(b.y + b.height / 2 + 8), statusSize, statusColor);

		if (completed)
		{
			//checkmark simple dibujado a mano (sin depender de glifos de fuente)
			Vector2 p1 = { b.x + b.width - 40, b.y + 20 };
			Vector2 p2 = { b.x + b.width - 30, b.y + 30 };
			Vector2 p3 = { b.x + b.width - 14, b.y + 10 };
			DrawLineEx(p1, p2, 4, DARKGREEN);
			DrawLineEx(p2, p3, 4, DARKGREEN);
		}
		else if (!unlocked)
		{
			//candado simple dibujado con formas primitivas
			float lockX = b.x + b.width - 34;
			float lockY = b.y + 12;
			DrawRectangleLinesEx({ lockX + 3, lockY, 14, 12 }, 3, BLACK);   //arco
			DrawRectangleRec({ lockX, lockY + 10, 20, 16 }, BLACK);        //cuerpo
		}
	}

	//--- HERRAMIENTA DE DEBUG: boton para desbloquear todo, facil de quitar antes de entregar ---
	DrawRectangleRec(unlockAllButton, Fade(ORANGE, 0.85f));
	DrawRectangleLinesEx(unlockAllButton, 3, BLACK);
	const char* unlockLabel = "DESBLOQUEAR TODOS (debug)";
	DrawText(unlockLabel, (int)(unlockAllButton.x + unlockAllButton.width / 2 - MeasureText(unlockLabel, 24) / 2), (int)(unlockAllButton.y + unlockAllButton.height / 2 - 12), 24, BLACK);

	//borrar progreso: boton rojo, pide confirmacion antes de hacer nada
	bool overReset = !confirmingReset && CheckCollisionPointRec(mousePos, resetButton);
	DrawRectangleRec(resetButton, overReset ? Fade(RED, 0.95f) : Fade(MAROON, 0.9f));
	DrawRectangleLinesEx(resetButton, 3, BLACK);
	const char* resetLabel = "Borrar progreso y empezar de cero";
	DrawText(resetLabel, (int)(resetButton.x + resetButton.width / 2 - MeasureText(resetLabel, 28) / 2), (int)(resetButton.y + resetButton.height / 2 - 14), 28, RAYWHITE);

	Color backColor = RAYWHITE;
	if (mousePos.x >= backButton.x && mousePos.y >= backButton.y && mousePos.x <= backButton.x + backButton.width && mousePos.y <= backButton.y + backButton.height)
	{
		backColor = BLACK;
	}
	DrawTexturePro(buttonTexture, srcFull, backButton, { 0,0 }, 0, RAYWHITE);
	DrawText("Volver", (int)(backButton.x + backButton.width / 2 - MeasureText("Volver", 30) / 2), (int)(backButton.y + backButton.height / 2 - 15), 30, backColor);

	if (confirmingReset) DrawConfirmReset();
}
void levelSelect::DrawConfirmReset()
{
	//velo oscuro sobre toda la pantalla + panel con el aviso y los dos botones
	DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.7f));
	DrawRectangleRec(confirmPanel, Color{ 38, 30, 44, 255 });
	DrawRectangleLinesEx(confirmPanel, 4, MAROON);

	const char* title = "BORRAR PROGRESO";
	DrawText(title, (int)(confirmPanel.x + confirmPanel.width / 2 - MeasureText(title, 48) / 2), (int)confirmPanel.y + 30, 48, RAYWHITE);

	const char* lines[3] = {
		"Se perdera todo tu progreso: niveles desbloqueados,",
		"niveles completados y el arma conseguida.",
		"Esta accion NO se puede deshacer."
	};
	for (int i = 0; i < 3; i++)
	{
		Color c = (i == 2) ? Color{ 255, 120, 120, 255 } : LIGHTGRAY;
		DrawText(lines[i], (int)(confirmPanel.x + confirmPanel.width / 2 - MeasureText(lines[i], 28) / 2), (int)confirmPanel.y + 120 + i * 42, 28, c);
	}

	bool overYes = CheckCollisionPointRec(mousePos, confirmYesButton);
	DrawRectangleRec(confirmYesButton, overYes ? RED : MAROON);
	DrawRectangleLinesEx(confirmYesButton, 3, BLACK);
	const char* yes = "Si, borrar y empezar";
	DrawText(yes, (int)(confirmYesButton.x + confirmYesButton.width / 2 - MeasureText(yes, 30) / 2), (int)(confirmYesButton.y + confirmYesButton.height / 2 - 15), 30, RAYWHITE);

	bool overNo = CheckCollisionPointRec(mousePos, confirmNoButton);
	DrawRectangleRec(confirmNoButton, overNo ? Color{ 90, 110, 90, 255 } : Color{ 60, 80, 60, 255 });
	DrawRectangleLinesEx(confirmNoButton, 3, BLACK);
	const char* no = "Cancelar";
	DrawText(no, (int)(confirmNoButton.x + confirmNoButton.width / 2 - MeasureText(no, 30) / 2), (int)(confirmNoButton.y + confirmNoButton.height / 2 - 15), 30, RAYWHITE);
}
void levelSelect::Update(saveManager& save)
{
	Inputs(save);
	Draw(save);
}
bool levelSelect::WantsExitToMenu()
{
	if (exitToMenu)
	{
		exitToMenu = false;
		return true;
	}
	return false;
}
int levelSelect::ConsumeSelectedLevel()
{
	int lvl = selectedLevel;
	selectedLevel = -1;
	return lvl;
}
void levelSelect::Unload()
{
	UnloadTexture(buttonTexture);
}
