#include "Art.h"
#include <math.h>

namespace Art
{
	//---------------- estado interno ----------------
	static bool loaded = false;
	static int theme = 0; //0 salon, 1 mazmorra, 2 torre, 3 trono

	static Texture2D bg[4], floorTile[4], platL[4], platM[4], platR[4];
	static Texture2D movL, movR, movM;
	static Texture2D torchSheet, torchHalo;
	static Texture2D bannerRed, bannerBlue, bannerPurple;

	Texture2D doorSheet, buttonSheet, spikeSheet, orbPending, orbCleared, coinSheet;
	animSheet slimeIdle, slimeHop, slimeDeath, beeFly, dogIdle, dogRun, orcIdle, orcWalk, orcCast, puff;
	animSheet boss1Idle, boss1Slam, boss2Idle, boss2Slam;

	static const char* THEME_DIRS[4] = { "Salon", "Mazmorra", "Torre", "Trono" };

	static animSheet LoadAnim(const char* path, int frames, float frameTime)
	{
		animSheet a;
		a.tex = LoadTexture(path);
		a.frames = frames;
		a.fw = a.tex.width / frames;
		a.fh = a.tex.height;
		a.frameTime = frameTime;
		return a;
	}

	void Load()
	{
		if (loaded) return;
		loaded = true;

		//ambientaciones: fondo, piso (tile) y plataformas (tapa izquierda, medio, tapa derecha)
		for (int i = 0; i < 4; i++)
		{
			bg[i] = LoadTexture(TextFormat("Assets/Sprites/Map/%s/fondo.png", THEME_DIRS[i]));
			floorTile[i] = LoadTexture(TextFormat("Assets/Sprites/Map/%s/piso.png", THEME_DIRS[i]));
			platL[i] = LoadTexture(TextFormat("Assets/Sprites/Map/%s/plat_izq.png", THEME_DIRS[i]));
			platM[i] = LoadTexture(TextFormat("Assets/Sprites/Map/%s/plat_medio.png", THEME_DIRS[i]));
			platR[i] = LoadTexture(TextFormat("Assets/Sprites/Map/%s/plat_der.png", THEME_DIRS[i]));
		}

		doorSheet = LoadTexture("Assets/Sprites/Others/Castle/puerta.png");
		buttonSheet = LoadTexture("Assets/Sprites/Others/Castle/boton.png");
		spikeSheet = LoadTexture("Assets/Sprites/Others/Castle/pinchos.png");
		movL = LoadTexture("Assets/Sprites/Others/Castle/mov_izq.png");
		movR = LoadTexture("Assets/Sprites/Others/Castle/mov_der.png");
		movM = LoadTexture("Assets/Sprites/Others/Castle/mov_medio.png");
		orbPending = LoadTexture("Assets/Sprites/Others/Castle/orbe_pendiente.png");
		orbCleared = LoadTexture("Assets/Sprites/Others/Castle/orbe_completado.png");
		coinSheet = LoadTexture("Assets/Sprites/Others/Castle/moneda.png");
		torchSheet = LoadTexture("Assets/Sprites/Others/Castle/antorcha.png");
		torchHalo = LoadTexture("Assets/Sprites/Others/Castle/antorcha_halo.png");
		bannerRed = LoadTexture("Assets/Sprites/Others/Castle/estandarte_rojo.png");
		bannerBlue = LoadTexture("Assets/Sprites/Others/Castle/estandarte_azul.png");
		bannerPurple = LoadTexture("Assets/Sprites/Others/Castle/estandarte_violeta.png");

		//enemigos (cuadros por hoja y duracion de cada cuadro)
		slimeIdle = LoadAnim("Assets/Sprites/Enemy/Anim/slime_idle.png", 4, 0.14f);
		slimeHop = LoadAnim("Assets/Sprites/Enemy/Anim/slime_hop.png", 8, 0.07f);
		slimeDeath = LoadAnim("Assets/Sprites/Enemy/Anim/slime_death.png", 5, 0.08f);
		beeFly = LoadAnim("Assets/Sprites/Enemy/Anim/bee_fly.png", 4, 0.06f);
		dogIdle = LoadAnim("Assets/Sprites/Enemy/Anim/dog_idle.png", 4, 0.12f);
		dogRun = LoadAnim("Assets/Sprites/Enemy/Anim/dog_run.png", 6, 0.06f);
		orcIdle = LoadAnim("Assets/Sprites/Enemy/Anim/orc_idle.png", 4, 0.14f);
		orcWalk = LoadAnim("Assets/Sprites/Enemy/Anim/orc_walk.png", 6, 0.12f);
		orcCast = LoadAnim("Assets/Sprites/Enemy/Anim/orc_cast.png", 6, 0.09f);
		puff = LoadAnim("Assets/Sprites/Enemy/Anim/puff.png", 6, 0.07f);
		boss1Idle = LoadAnim("Assets/Sprites/Enemy/Anim/boss1_idle.png", 6, 0.11f);
		boss1Slam = LoadAnim("Assets/Sprites/Enemy/Anim/boss1_slam.png", 8, 0.06f);
		boss2Idle = LoadAnim("Assets/Sprites/Enemy/Anim/boss2_idle.png", 6, 0.11f);
		boss2Slam = LoadAnim("Assets/Sprites/Enemy/Anim/boss2_slam.png", 8, 0.06f);
	}

	void Unload()
	{
		if (!loaded) return;
		loaded = false;

		for (int i = 0; i < 4; i++)
		{
			UnloadTexture(bg[i]); UnloadTexture(floorTile[i]);
			UnloadTexture(platL[i]); UnloadTexture(platM[i]); UnloadTexture(platR[i]);
		}
		UnloadTexture(doorSheet); UnloadTexture(buttonSheet); UnloadTexture(spikeSheet);
		UnloadTexture(movL); UnloadTexture(movR); UnloadTexture(movM);
		UnloadTexture(orbPending); UnloadTexture(orbCleared); UnloadTexture(coinSheet);
		UnloadTexture(torchSheet); UnloadTexture(torchHalo);
		UnloadTexture(bannerRed); UnloadTexture(bannerBlue); UnloadTexture(bannerPurple);

		animSheet* all[] = { &slimeIdle, &slimeHop, &slimeDeath, &beeFly, &dogIdle, &dogRun, &orcIdle, &orcWalk, &orcCast, &puff,
			&boss1Idle, &boss1Slam, &boss2Idle, &boss2Slam };
		for (int i = 0; i < 14; i++) UnloadTexture(all[i]->tex);
	}

	void SetTheme(int levelIndex)
	{
		if (levelIndex <= 3) theme = 0;
		else if (levelIndex <= 6) theme = 1;
		else if (levelIndex <= 10) theme = 2;
		else theme = 3;
	}

	//---------------- decoracion de pared ----------------
	//posiciones fijas por ambientacion (son fondo: las plataformas y objetos se dibujan encima)
	static const Vector2 TORCHES[3] = { { 380, 470 }, { 1130, 330 }, { 1750, 560 } };

	//dibuja una pantalla completa de fondo (1920x1080) con su decoracion, con la esquina izquierda en x0.
	//mirror = true la dibuja espejada (se usa para empalmar sin costura el fondo del menu que se desliza)
	static void DrawBackgroundLayer(int th, float x0, bool mirror)
	{
		float t = (float)GetTime();

		//posicion en pantalla de algo que mide w y esta en x dentro del fondo (se refleja si va espejado)
		#define BG_X(x, w) (mirror ? x0 + 1920.0f - (x) - (w) : x0 + (x))
		#define BG_SRC_W(w) (mirror ? -(float)(w) : (float)(w))

		DrawTexturePro(bg[th], { 0, 0, BG_SRC_W(1920), 1080 }, { x0, 0, 1920, 1080 }, { 0,0 }, 0, WHITE);

		//halos de luz detras de las antorchas
		for (int i = 0; i < 3; i++)
		{
			float hw = (float)torchHalo.width, hh = (float)torchHalo.height;
			DrawTextureV(torchHalo, { BG_X(TORCHES[i].x + 16 - hw / 2, hw), TORCHES[i].y + 16 - hh / 2 }, WHITE);
		}

		//estandartes
		Texture2D banner = (th == 2) ? bannerPurple : ((th == 3) ? bannerRed : bannerBlue);
		float bannerXs[2] = { 700, 1130 };
		if (th == 3) { bannerXs[0] = 470; bannerXs[1] = 1400; }
		for (int i = 0; i < 2; i++)
		{
			float bw = (float)banner.width, bh = (float)banner.height;
			DrawTexturePro(banner, { 0, 0, BG_SRC_W(bw), bh }, { BG_X(bannerXs[i], bw), 40, bw, bh }, { 0,0 }, 0, WHITE);
		}

		//antorchas: cada una con su propia fase para que no parpadeen todas a la vez
		for (int i = 0; i < 3; i++)
		{
			int frame = ((int)(t / 0.1f) + i) % 4;
			DrawTexturePro(torchSheet, { (float)(frame * 32), 0, BG_SRC_W(32), 64 }, { BG_X(TORCHES[i].x, 32), TORCHES[i].y, 32, 64 }, { 0,0 }, 0, WHITE);
		}

		#undef BG_X
		#undef BG_SRC_W
	}

	void DrawBackground()
	{
		DrawBackgroundLayer(theme, 0, false);
	}

	void DrawMenuBackground()
	{
		//el salon se desliza de izquierda a derecha sin parar. Para que el recorrido no tenga corte, el fondo
		//se repite alternando su version normal y su version espejada (en la union los pilares coinciden)
		const float period = 3840.0f;   //una vuelta completa = fondo normal + fondo espejado
		const float speed = 45.0f;      //pixeles por segundo
		float offset = fmodf((float)GetTime() * speed, period);

		for (int k = -2; k <= 0; k++)
		{
			DrawBackgroundLayer(0, offset + k * 1920.0f, (k % 2) != 0);
		}

		//velo oscuro suave para que los botones y titulos se lean bien sobre el fondo en movimiento
		DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.3f));
	}

	//---------------- pisos y plataformas ----------------
	void DrawSolid(Rectangle r)
	{
		if (r.height >= 60)
		{
			//piso: tile de 128x80 repetido a lo largo
			Texture2D tile = floorTile[theme];
			for (float x = r.x; x < r.x + r.width; x += 128)
			{
				float w = (r.x + r.width) - x;
				if (w > 128) w = 128;
				float h = r.height > 80 ? 80 : r.height;
				DrawTexturePro(tile, { 0, 0, w, h }, { x, r.y, w, h }, { 0,0 }, 0, WHITE);
			}
			return;
		}

		//plataforma: tapa izquierda + medio repetido + tapa derecha (el alto se ajusta al rectangulo)
		Texture2D l = platL[theme], m = platM[theme], rr = platR[theme];
		float capW = 32;
		if (r.width < capW * 2) capW = r.width / 2;
		DrawTexturePro(l, { 0, 0, capW, 30 }, { r.x, r.y, capW, r.height }, { 0,0 }, 0, WHITE);
		DrawTexturePro(rr, { 32 - capW, 0, capW, 30 }, { r.x + r.width - capW, r.y, capW, r.height }, { 0,0 }, 0, WHITE);
		for (float x = r.x + capW; x < r.x + r.width - capW; x += 64)
		{
			float w = (r.x + r.width - capW) - x;
			if (w > 64) w = 64;
			DrawTexturePro(m, { 0, 0, w, 30 }, { x, r.y, w, r.height }, { 0,0 }, 0, WHITE);
		}
	}

	void DrawMovingPlatform(Rectangle r)
	{
		int frame = ((int)((float)GetTime() / 0.12f)) % 4;
		float capW = 32;
		DrawTexturePro(movL, { 0, 0, capW, 30 }, { r.x, r.y, capW, r.height }, { 0,0 }, 0, WHITE);
		DrawTexturePro(movR, { 0, 0, capW, 30 }, { r.x + r.width - capW, r.y, capW, r.height }, { 0,0 }, 0, WHITE);
		for (float x = r.x + capW; x < r.x + r.width - capW; x += 64)
		{
			float w = (r.x + r.width - capW) - x;
			if (w > 64) w = 64;
			DrawTexturePro(movM, { (float)(frame * 64), 0, w, 30 }, { x, r.y, w, r.height }, { 0,0 }, 0, WHITE);
		}
	}

	//---------------- animaciones ----------------
	int LoopFrame(const animSheet& a, float time)
	{
		if (a.frames <= 0 || a.frameTime <= 0) return 0;
		int f = (int)(time / a.frameTime);
		return ((f % a.frames) + a.frames) % a.frames;
	}

	void DrawFrame(const animSheet& a, int frame, Vector2 anchor, float scale, bool flipX, bool centered, Color tint)
	{
		if (frame < 0) frame = 0;
		if (frame >= a.frames) frame = a.frames - 1;

		float w = a.fw * scale, h = a.fh * scale;
		Rectangle src = { (float)(frame * a.fw), 0, (float)(flipX ? -a.fw : a.fw), (float)a.fh };
		Rectangle dst = { anchor.x - w / 2, centered ? anchor.y - h / 2 : anchor.y - h, w, h };
		DrawTexturePro(a.tex, src, dst, { 0,0 }, 0, tint);
	}
}
