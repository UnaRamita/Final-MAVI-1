#include "Scene.h"
#include "Art.h"

scene::scene()
{
	sceneN = 1;
	fontSize = 40;//probar tama�os dsp con el meno ya hecho

	musicVolume = 5;
	fxVolume = 5;

	exitGame = false;
	canClick = true;

	mousePos = GetMousePosition();
	//a todo esto le pongo un valor y despues lo inicializo en inits para que se carge una vez creada la ventana

	buttonTexture = { 0 };
	buttonLitRightTexture1 = { 0 };
	buttonLitLeftTexture1 = { 0 };

	playButton = {100,100,100,100 };
	levelsButton = { 100,100,100,100 };
	optionButton = { 100,100,100,100 };
	exitButton = { 100,100,100,100 };
	volumeButton1 = { 100,100,100,100 };
	volumeButton2 = { 100,100,100,100 };

	menuMusic = { 0 };
	gameMusic = { 0 };

	levelLoaded = false;
	optionsReturnTo = 1;
	fxPreviewCooldown = 0;
}
//aca solo inicializar entidades, pantalla y demas va en game
void scene::Inits() {
	//el arte del castillo tambien se usa en los menus (fondo animado), asi que se carga desde el arranque
	Art::Load();
	buttonTexture = LoadTexture("Assets/Sprites/Others/StoneButton.png");
	buttonLitRightTexture1 = LoadTexture("Assets/Sprites/Others/StoneButtonLRight.png");
	buttonLitLeftTexture1 = LoadTexture("Assets/Sprites/Others/StoneButtonLLeft.png");

	//stack vertical del menu: Play -> Niveles -> Options -> Exit, cada uno 100px por debajo del anterior
	playButton = { (float)GetScreenWidth() / 2 - buttonTexture.width / 2,(float)GetScreenHeight() / 2 - buttonTexture.height, (float)buttonTexture.width, (float)buttonTexture.height };
	levelsButton = { playButton.x, playButton.y + 100, (float)buttonTexture.width, (float)buttonTexture.height };
	optionButton = { playButton.x, playButton.y + 200, (float)buttonTexture.width, (float)buttonTexture.height };
	exitButton = { playButton.x, playButton.y + 300, (float)buttonTexture.width, (float)buttonTexture.height };
	volumeButton1 = { (float)playButton.x - buttonLitLeftTexture1.width - 50,playButton.y,(float)buttonLitLeftTexture1.width,(float)buttonLitLeftTexture1.height };
	volumeButton2 = { (float)playButton.x + playButton.width + 50,playButton.y,(float)buttonLitLeftTexture1.width,(float)buttonLitLeftTexture1.height };

	menuMusic = LoadMusicStream("Assets/Music/menu.mp3");
	gameMusic = LoadMusicStream("Assets/Music/game.mp3");
	SetMusicVolume(menuMusic, musicVolume / 100.0f);
	PlayMusicStream(menuMusic);

	selector.Init();
	saveData.Load();

	previewSounds.Init();
	previewSounds.SetVolume(fxVolume / 100.0f);
}
void scene::Load() {
	switch (sceneN)
	{

	case 1:
		Menu();
		break;
	case 2:
		Game();
		break;
	case 3:
		LevelSelectScreen();
		break;
	case 4:
		Options();
		break;
	default:
		sceneN = 1;
		break;
	}
}
void scene::Inputs() {
	mousePos = GetMousePosition();
	//menu
	if (sceneN == 1) {
		//playbutton act: lleva al selector de niveles (ya no arranca un nivel fijo a mano)
		if (mousePos.x >= playButton.x && mousePos.y >= playButton.y && mousePos.x <= playButton.x + playButton.width && mousePos.y <= playButton.y + playButton.height && IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
		{
			sceneN = 3;
		}
		//levelsbutton act
		if (mousePos.x >= levelsButton.x && mousePos.y >= levelsButton.y && mousePos.x <= levelsButton.x + levelsButton.width && mousePos.y <= levelsButton.y + levelsButton.height && IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
		{
			sceneN = 3;
		}
		//optionbutton act
		if (mousePos.x >= optionButton.x && mousePos.y >= optionButton.y && mousePos.x <= optionButton.x + optionButton.width && mousePos.y <= optionButton.y + optionButton.height && IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
		{
			optionsReturnTo = 1; //desde el menu principal, options vuelve al menu principal
			sceneN = 4;
		}
		//exitbutton act (todavia no se si va aca pero de ultima dsp lo cambio)
		if (mousePos.x >= exitButton.x && mousePos.y >= exitButton.y && mousePos.x <= exitButton.x + exitButton.width && mousePos.y <= exitButton.y + exitButton.height && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
		{
			exitGame = true;
		}
	}
	//options
	if (sceneN == 4) {
		//el segundo renglon (fx) esta siempre a la misma distancia del primero que optionButton de playButton
		float rowGap = optionButton.y - playButton.y;

		//botones de volumen general
		//bajar
		if (canClick && mousePos.x >= volumeButton1.x && mousePos.y >= volumeButton1.y && mousePos.x <= volumeButton1.x + volumeButton1.width && mousePos.y <= volumeButton1.y + volumeButton1.height && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
			musicVolume -= 5;
			SetVolume();
			canClick = false;
			//printf("music -5\n"); //debug por que sube y baja de a 10

		}
		//subir
		if (canClick&&mousePos.x >= volumeButton2.x && mousePos.y >= volumeButton2.y && mousePos.x <= volumeButton2.x + volumeButton2.width && mousePos.y <= volumeButton2.y + volumeButton2.height && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
			musicVolume += 5;
			SetVolume();
			canClick = false;
		}
		//botonos de volumen fx
		//bajar
		if (canClick&&mousePos.x >= volumeButton1.x && mousePos.y >= volumeButton1.y + rowGap && mousePos.x <= volumeButton1.x + volumeButton1.width && mousePos.y <= volumeButton1.y + rowGap + volumeButton1.height && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
			fxVolume -= 5;
			SetVolume();
			PreviewFxVolume();
			canClick = false;
		}
		//subir
		if (canClick&&mousePos.x >= volumeButton2.x && mousePos.y >= volumeButton2.y + rowGap && mousePos.x <= volumeButton2.x + volumeButton2.width && mousePos.y <= volumeButton2.y + rowGap + volumeButton2.height && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
			fxVolume += 5;
			SetVolume();
			PreviewFxVolume();
			canClick = false;
		}

		//botton de ir hacia atras: vuelve a donde estaba antes de entrar a opciones (menu o pausa)
		if (canClick&&mousePos.x >= exitButton.x && mousePos.y >= exitButton.y && mousePos.x <= exitButton.x + exitButton.width && mousePos.y <= exitButton.y + exitButton.height && IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
		{
			canClick = false;
			sceneN = optionsReturnTo;
		}
	}
	if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
	{
		canClick = true;
	}
}
bool scene::ShouldExit()
{
	return exitGame;
}
void scene::Menu() {
	//menu inicial
		//color del texto del boton
	Color tempColor1 = RAYWHITE;
	//musica
	UpdateMusicStream(menuMusic);
	//fondo: el salon del castillo deslizandose de izquierda a derecha
	Art::DrawMenuBackground();
	//boton de play
	DrawTextureEx(buttonTexture, { playButton.x,playButton.y }, 0, 1, RAYWHITE);
	if (mousePos.x >= playButton.x && mousePos.y >= playButton.y && mousePos.x <= playButton.x + playButton.width && mousePos.y <= playButton.y + playButton.height) {
		tempColor1 = BLACK;
	}
	else {
		tempColor1 = RAYWHITE;
	}
	DrawText("Play", GetScreenWidth() / 2 - buttonTexture.width / 2 + fontSize / 2, (int)(playButton.y + playButton.height / 2 - fontSize / 2), fontSize, tempColor1);
	//boton de niveles
	DrawTextureEx(buttonTexture, { levelsButton.x,levelsButton.y }, 0, 1, RAYWHITE);
	if (mousePos.x >= levelsButton.x && mousePos.y >= levelsButton.y && mousePos.x <= levelsButton.x + levelsButton.width && mousePos.y <= levelsButton.y + levelsButton.height) {
		tempColor1 = BLACK;
	}
	else {
		tempColor1 = RAYWHITE;
	}
	DrawText("Niveles", GetScreenWidth() / 2 - buttonTexture.width / 2 + fontSize / 2, (int)(levelsButton.y + levelsButton.height / 2 - fontSize / 2), fontSize, tempColor1);
	//boton de options
	DrawTextureEx(buttonTexture, { optionButton.x,optionButton.y }, 0, 1, RAYWHITE);
	if (mousePos.x >= optionButton.x && mousePos.y >= optionButton.y && mousePos.x <= optionButton.x + optionButton.width && mousePos.y <= optionButton.y + optionButton.height) {
		tempColor1 = BLACK;
	}
	else {
		tempColor1 = RAYWHITE;
	}
	DrawText("Options", GetScreenWidth() / 2 - buttonTexture.width / 2 + fontSize / 2, (int)(optionButton.y + optionButton.height / 2 - fontSize / 2), fontSize, tempColor1);
	//boton de exit
	DrawTextureEx(buttonTexture, { exitButton.x,exitButton.y }, 0, 1, RAYWHITE);
	if (mousePos.x >= exitButton.x && mousePos.y >= exitButton.y && mousePos.x <= exitButton.x + exitButton.width && mousePos.y <= exitButton.y + exitButton.height) {
		tempColor1 = BLACK;
	}
	else {
		tempColor1 = RAYWHITE;
	}
	DrawText("Exit", GetScreenWidth() / 2 - buttonTexture.width / 2 + fontSize / 2, (int)(exitButton.y + exitButton.height / 2 - fontSize / 2), fontSize, tempColor1);
}
void scene::EnterLevel(int levelIndex)
{
	sceneN = 2;
	StopMusicStream(menuMusic);
	PlayMusicStream(gameMusic);

	if (!levelLoaded)
	{
		gameLevel.Init(fxVolume / 100.0f);
		levelLoaded = true;
	}

	gameLevel.StartLevel(levelIndex, saveData.GetSavedWeapon());
}
void scene::Game() {
	//Game
	UpdateMusicStream(gameMusic);
	gameLevel.Update();

	//si se completo un nivel de la progresion (no el de prueba), se guarda el progreso
	int completedLevel;
	weaponType finalWeapon;
	if (gameLevel.ConsumePendingCompletion(completedLevel, finalWeapon))
	{
		if (completedLevel >= 1 && completedLevel <= TOTAL_LEVELS)
		{
			saveData.SetSavedWeapon(finalWeapon);
			saveData.CompleteLevel(completedLevel); //tambien persiste en disco
		}
	}

	//niveles 1-9: tras el breve festejo, se carga directamente el siguiente (sin pasar por el selector)
	int justFinishedLevel;
	if (gameLevel.ConsumeAdvanceRequest(justFinishedLevel))
	{
		EnterLevel(justFinishedLevel + 1);
	}

	//pedido de abrir Opciones desde el menu de pausa: al volver, debe regresar a la pausa (no al gameplay)
	if (gameLevel.WantsOptions())
	{
		optionsReturnTo = 2;
		sceneN = 4;
	}

	if (gameLevel.WantsExitToMenu())
	{
		//"Volver al menu principal" desde la pausa vuelve siempre al menu principal
		sceneN = 1;
		StopMusicStream(gameMusic);
		PlayMusicStream(menuMusic);
	}
}
void scene::LevelSelectScreen()
{
	UpdateMusicStream(menuMusic);
	selector.Update(saveData);

	int chosen = selector.ConsumeSelectedLevel();
	if (chosen >= 0)
	{
		EnterLevel(chosen);
	}

	if (selector.WantsExitToMenu())
	{
		sceneN = 1;
	}
}
void scene::Options() {
	//options
	Color tempColor2 = RAYWHITE;
	//musica: si se entro desde la pausa (optionsReturnTo==2) hay que seguir alimentando la
	//musica del nivel (gameMusic), no la del menu, para que no se corte mientras se navega aca
	if (optionsReturnTo == 2)
	{
		UpdateMusicStream(gameMusic);
	}
	else
	{
		UpdateMusicStream(menuMusic);
	}
	//fondo
	Art::DrawMenuBackground();
	//musicvolume
	DrawTextureEx(buttonTexture, { playButton.x,playButton.y }, 0, 1, RAYWHITE);
	DrawText(to_string(musicVolume).c_str(), GetScreenWidth() / 2 - buttonTexture.width / 2 + fontSize / 2, (int)(playButton.y + playButton.height / 2 - fontSize / 2), fontSize, tempColor2);
	DrawTextureEx(buttonLitLeftTexture1, { volumeButton1.x,volumeButton1.y }, 0, 1, RAYWHITE);
	DrawTextureEx(buttonLitRightTexture1, { volumeButton2.x,volumeButton2.y }, 0, 1, RAYWHITE);
	//fxvolume (el segundo renglon queda a la misma distancia del primero que optionButton de playButton)
	float rowGap = optionButton.y - playButton.y;
	DrawTextureEx(buttonTexture, { optionButton.x,optionButton.y }, 0, 1, RAYWHITE);
	DrawText(to_string(fxVolume).c_str(), GetScreenWidth() / 2 - buttonTexture.width / 2 + fontSize / 2, (int)(optionButton.y + optionButton.height / 2 - fontSize / 2), fontSize, tempColor2);//esto no se si lo centre bien x2
	DrawTextureEx(buttonLitLeftTexture1, { volumeButton1.x,volumeButton1.y + rowGap }, 0, 1, RAYWHITE);
	DrawTextureEx(buttonLitRightTexture1, { volumeButton2.x,volumeButton2.y + rowGap }, 0, 1, RAYWHITE);
	// boton de volver atras (es el mismo que el de exit peeero le cambie el texto)
	DrawTextureEx(buttonTexture, { exitButton.x,exitButton.y }, 0, 1, RAYWHITE);
	if (mousePos.x >= exitButton.x && mousePos.y >= exitButton.y && mousePos.x <= exitButton.x + exitButton.width && mousePos.y <= exitButton.y + exitButton.height) {
		tempColor2 = BLACK;
	}
	else {
		tempColor2 = RAYWHITE;
	}
	DrawText("Back", GetScreenWidth() / 2 - buttonTexture.width / 2 + fontSize / 2, (int)(exitButton.y + exitButton.height / 2 - fontSize / 2), fontSize, tempColor2);
}
void scene::SetVolume(){
	if (musicVolume > 100)
	{
		musicVolume = 100;
	}
	if (fxVolume > 100) {
		fxVolume = 100;
	}
	if (musicVolume < 0)
	{
		musicVolume = 0;
	}
	if (fxVolume < 0) {
		fxVolume = 0;
	}

	float mVol = musicVolume / 100.0f;
	float fxVol = fxVolume / 100.0f;


	SetMusicVolume(menuMusic, mVol);
	SetMusicVolume(gameMusic, mVol);

	//el preview de FX en Opciones usa exactamente el mismo volumen que los sonidos del gameplay
	previewSounds.SetVolume(fxVol);

	if (levelLoaded)
	{
		gameLevel.SetFxVolume(fxVol);
	}
}
void scene::PreviewFxVolume()
{
	//evita que se acumulen disparos superpuestos si se clickea +/- muy rapido
	if (fxPreviewCooldown > 0) return;

	fxPreviewCooldown = 0.15f;
	previewSounds.PlayShoot();
}
void scene::Update() {
	if (fxPreviewCooldown > 0)
	{
		fxPreviewCooldown -= GetFrameTime();
	}

	Inputs();
	Load();
}
scene::~scene()
{
	if (levelLoaded)
	{
		gameLevel.Unload();
	}
	selector.Unload();
	previewSounds.Unload();
}
