#include "Boss.h"
#include "Art.h"
#include "Collisions.h"
#include <math.h>

namespace
{
	//--- onda horizontal: una pared de segmentos que cruza la arena ---
	//Se reparte el alto en 8 segmentos de 95 px. Cualquier golpe del arma mide mas que un segmento
	//(la espada cubre la altura del jugador, 128 px, o mas), asi que siempre rompe al menos 2 seguidos y deja
	//un hueco de 190 px o mas: sobra para que pase el personaje (128 px) pero no se puede abrir "gratis"
	//sin estar a la altura de la zona que se quiere liberar.
	const float WAVE_TOP = 240.0f;        //borde superior de la columna
	const float WAVE_BOTTOM = 1000.0f;    //borde inferior (el piso)
	const int   WAVE_SEGMENTS = 8;
	const float WAVE_WIDTH = 60.0f;
	const float WAVE_SPEED = 260.0f;      //px/s, igual que la onda original
	const float WAVE_BREAK_FX = 0.30f;    //duracion del destello al romperse un segmento
	const float FIRE_FRAME = 80.0f;       //hoja de fuego: frames de 80x80 con el sprite de 32x32 al centro
	const float FIRE_SPRITE = 32.0f;
	const int   FIRE_FRAMES = 6;
	const float FIRE_FRAME_TIME = 0.07f;
}

boss::boss()
{
	waveTexture = { 0 };
	pos = { 0,0 };
	size = { 0,0 };

	chargeTime = 0;
	releaseTime = -1;
	wasCharging = false;

	state = bossState::INTRO;
	stateTimer = 0;

	introDuration = 2.0f;
	attackPhaseDuration = 30.0f;
	deathDuration = 3.0f;

	cyclesRequired = 3;
	currentCycle = 1;

	maxHP = 300;
	currentHP = 300;
	hpPerCycle = 100;

	aimedCharging = false;
	aimedChargeTimer = 0;
	aimedCooldown = 1.1f;

	fanCharging = false;
	fanChargeTimer = 0;
	fanCooldown = 4.0f;

	waveTelegraphing = false;
	waveTelegraphTimer = 0;
	waveCooldown = 5.5f;
	waveStartX = 0;
	waveDirX = 1;

	flashTimer = 0;
	bobTimer = 0;
}
boss::~boss()
{
}
void boss::Init(Vector2 spawnPos, Vector2 spriteSize, float attackPhaseDurationSeconds, int cyclesRequiredVal, int maxHPVal)
{
	pos = spawnPos;
	size = spriteSize;

	//--- valores configurables (equivalente al "inspector"): los recibe Init, no estan hardcodeados aca abajo ---
	attackPhaseDuration = attackPhaseDurationSeconds;
	cyclesRequired = (cyclesRequiredVal >= 1) ? cyclesRequiredVal : 1;
	maxHP = (maxHPVal >= cyclesRequired) ? maxHPVal : cyclesRequired;
	hpPerCycle = maxHP / cyclesRequired;

	introDuration = 2.0f;
	deathDuration = 3.0f;

	Reset();
}
void boss::Reset()
{
	state = bossState::INTRO;
	stateTimer = 0;
	currentCycle = 1;
	currentHP = maxHP;
	pendingAttacks.clear();
	waves.clear();

	aimedCharging = false;
	aimedCooldown = 1.1f;
	fanCharging = false;
	fanCooldown = 4.0f;
	waveTelegraphing = false;
	waveCooldown = 5.5f;

	flashTimer = 0;
	bobTimer = 0;
	chargeTime = 0;
	releaseTime = -1;
	wasCharging = false;
}

// ---------------------------------------------------------------------------
// Estados
// ---------------------------------------------------------------------------

void boss::UpdateIntro(float dt)
{
	stateTimer += dt;
	if (stateTimer >= introDuration)
	{
		state = bossState::ATTACK_PHASE;
		stateTimer = 0;
	}
}
void boss::UpdateAttackPhase(float dt, Vector2 playerPos)
{
	stateTimer += dt;

	//dificultad progresiva: los ataques se aceleran un poco en cada ciclo
	float cycleSpeedup = (float)(currentCycle - 1) * 0.15f;

	//--- tiro dirigido a la posicion del jugador ---
	if (aimedCharging)
	{
		aimedChargeTimer -= dt;
		if (aimedChargeTimer <= 0)
		{
			PatternAimedShot(playerPos);
			aimedCharging = false;
			float cd = 1.1f - cycleSpeedup;
			aimedCooldown = (cd < 0.5f) ? 0.5f : cd;
		}
	}
	else
	{
		aimedCooldown -= dt;
		if (aimedCooldown <= 0)
		{
			aimedCharging = true;
			aimedChargeTimer = 0.3f; //anticipacion visual antes de disparar
		}
	}

	//--- rafaga en abanico (patron de varios proyectiles) ---
	if (fanCharging)
	{
		fanChargeTimer -= dt;
		if (fanChargeTimer <= 0)
		{
			PatternFanBurst();
			fanCharging = false;
			float cd = 4.0f - cycleSpeedup * 3.0f;
			fanCooldown = (cd < 2.2f) ? 2.2f : cd;
		}
	}
	else
	{
		fanCooldown -= dt;
		if (fanCooldown <= 0)
		{
			fanCharging = true;
			fanChargeTimer = 0.4f; //anticipacion visual antes de la rafaga
		}
	}

	//--- onda expansiva que atraviesa toda la arena ---
	if (waveTelegraphing)
	{
		waveTelegraphTimer -= dt;
		if (waveTelegraphTimer <= 0)
		{
			SpawnWave(waveStartX, waveDirX); //pared de segmentos que se puede romper con el arma
			waveTelegraphing = false;
			float cd = 5.5f - cycleSpeedup * 3.0f;
			waveCooldown = (cd < 3.2f) ? 3.2f : cd;
		}
	}
	else
	{
		waveCooldown -= dt;
		if (waveCooldown <= 0)
		{
			waveTelegraphing = true;
			waveTelegraphTimer = 0.7f; //anticipacion visual: aviso antes de que la onda se dispare

			bool fromLeft = (currentCycle % 2 == 1);
			waveStartX = fromLeft ? 20.0f : (float)GetScreenWidth() - WAVE_WIDTH - 20.0f;
			waveDirX = fromLeft ? 1.0f : -1.0f;
		}
	}

	if (stateTimer >= attackPhaseDuration)
	{
		state = bossState::VULNERABLE_PHASE;
		stateTimer = 0;

		//deja de atacar de inmediato al cambiar de fase
		aimedCharging = false;
		fanCharging = false;
		waveTelegraphing = false;
	}
}
void boss::UpdateDeath(float dt)
{
	stateTimer += dt;
	if (stateTimer >= deathDuration)
	{
		state = bossState::DEFEATED;
	}
}
void boss::Update(float dt, Vector2 playerPos)
{
	if (flashTimer > 0) flashTimer -= dt;
	bobTimer += dt;

	//las ondas que ya salieron siguen su camino aunque el jefe cambie de fase
	UpdateWaves(dt);

	switch (state)
	{
	case bossState::INTRO:
		UpdateIntro(dt);
		break;
	case bossState::ATTACK_PHASE:
		UpdateAttackPhase(dt, playerPos);
		break;
	case bossState::VULNERABLE_PHASE:
		//no ataca: espera a que el jugador le haga suficiente daño (ver TakeDamage)
		break;
	case bossState::DEATH:
		UpdateDeath(dt);
		break;
	case bossState::DEFEATED:
		break;
	}

	//animacion de ataque: mientras carga un ataque se agacha; al soltarlo (o al cambiar de fase) salta y golpea
	bool charging = (state == bossState::ATTACK_PHASE) && (aimedCharging || fanCharging || waveTelegraphing);
	if (charging)
	{
		chargeTime += dt;
		releaseTime = -1;
	}
	else
	{
		if (wasCharging) releaseTime = 0;
		else if (releaseTime >= 0) releaseTime += dt;
		chargeTime = 0;
	}
	wasCharging = charging;
}

// ---------------------------------------------------------------------------
// Daño / ciclos
// ---------------------------------------------------------------------------

void boss::TakeDamage(int amount)
{
	if (!CanBeDamaged()) return; //el jugador no puede dañarlo durante su fase de ataque
	if (amount <= 0) return;

	currentHP -= amount;
	if (currentHP < 0) currentHP = 0;
	flashTimer = 0.15f;

	int cycleFloor = maxHP - currentCycle * hpPerCycle; //HP que hay que bajar para completar este ciclo
	if (currentHP <= cycleFloor)
	{
		if (currentCycle >= cyclesRequired)
		{
			//tercer ciclo completado: muere definitivamente (nunca hay un cuarto ciclo)
			currentHP = 0;
			state = bossState::DEATH;
			stateTimer = 0;
			pendingAttacks.clear();
			waves.clear(); //al morir el jefe desaparecen todos sus ataques en pantalla
		}
		else
		{
			//ciclo completado: vuelve a la fase de ataque, un ciclo mas arriba
			currentCycle++;
			state = bossState::ATTACK_PHASE;
			stateTimer = 0;
		}
	}
}
bool boss::CanBeDamaged()
{
	return state == bossState::VULNERABLE_PHASE;
}
bool boss::IsDefeated()
{
	return state == bossState::DEFEATED;
}
Rectangle boss::GetHitbox()
{
	return { pos.x, pos.y, size.x, size.y };
}
bossState boss::GetState()
{
	return state;
}

// ---------------------------------------------------------------------------
// Patrones de ataque
// ---------------------------------------------------------------------------

void boss::QueueProjectile(Vector2 p, Vector2 v, Vector2 s)
{
	bossProjectileRequest req;
	req.pos = p;
	req.vel = v;
	req.size = s;
	pendingAttacks.push_back(req);
}
void boss::PatternAimedShot(Vector2 playerPos)
{
	Vector2 center = { pos.x + size.x / 2, pos.y + size.y / 2 };
	Vector2 dir = { playerPos.x - center.x, playerPos.y - center.y };
	float dist = sqrtf(dir.x * dir.x + dir.y * dir.y);
	if (dist < 1.0f) dist = 1.0f;

	float speed = 480.0f;
	Vector2 vel = { (dir.x / dist) * speed, (dir.y / dist) * speed };

	QueueProjectile({ center.x - 16, center.y - 16 }, vel, { 32, 32 });
}
void boss::PatternFanBurst()
{
	Vector2 center = { pos.x + size.x / 2, pos.y + size.y / 2 };

	int count = 5;
	float startDeg = 20.0f;   //en pantalla, y crece hacia abajo: esto cubre un abanico hacia abajo
	float spreadDeg = 140.0f;
	float speed = 380.0f;

	for (int i = 0; i < count; i++)
	{
		float t = (count == 1) ? 0.5f : (float)i / (float)(count - 1);
		float angleRad = (startDeg + spreadDeg * t) * (PI / 180.0f);
		Vector2 vel = { cosf(angleRad) * speed, sinf(angleRad) * speed };

		QueueProjectile({ center.x - 16, center.y - 16 }, vel, { 32, 32 });
	}
}
void boss::SpawnWave(float startX, float dirX)
{
	bossWave w;
	w.dirX = dirX;

	float segH = (WAVE_BOTTOM - WAVE_TOP) / WAVE_SEGMENTS;
	for (int i = 0; i < WAVE_SEGMENTS; i++)
	{
		waveSegment s;
		s.rect = { startX, WAVE_TOP + i * segH, WAVE_WIDTH, segH };
		s.alive = true;
		s.breakTimer = 0;
		w.segments.push_back(s);
	}
	waves.push_back(w);
}
void boss::UpdateWaves(float dt)
{
	float screenW = (float)GetScreenWidth();

	for (unsigned int i = 0; i < waves.size(); i++)
	{
		float dx = waves[i].dirX * WAVE_SPEED * dt;

		//todos los segmentos avanzan juntos; los rotos solo cuentan el tiempo del destello
		for (unsigned int j = 0; j < waves[i].segments.size(); j++)
		{
			waves[i].segments[j].rect.x += dx;
			if (!waves[i].segments[j].alive) waves[i].segments[j].breakTimer += dt;
		}

		//se descartan los segmentos rotos cuyo destello ya termino
		std::vector<waveSegment>& segs = waves[i].segments;
		for (int j = (int)segs.size() - 1; j >= 0; j--)
		{
			if (!segs[j].alive && segs[j].breakTimer >= WAVE_BREAK_FX) segs.erase(segs.begin() + j);
		}
	}

	//una onda termina cuando se rompio entera o cuando sale de la pantalla por el lado opuesto
	for (int i = (int)waves.size() - 1; i >= 0; i--)
	{
		bool finished = waves[i].segments.empty();
		if (!finished)
		{
			float x = waves[i].segments[0].rect.x;
			if (waves[i].dirX > 0 && x > screenW + 10.0f) finished = true;
			if (waves[i].dirX < 0 && x + WAVE_WIDTH < -10.0f) finished = true;
		}
		if (finished) waves.erase(waves.begin() + i);
	}
}
void boss::SetWaveTexture(Texture2D fireFlightSheet)
{
	waveTexture = fireFlightSheet;
}
bool boss::WaveTouches(Rectangle area)
{
	for (unsigned int i = 0; i < waves.size(); i++)
	{
		for (unsigned int j = 0; j < waves[i].segments.size(); j++)
		{
			if (waves[i].segments[j].alive && Collisions::CheckAABB(area, waves[i].segments[j].rect)) return true;
		}
	}
	return false;
}
int boss::BreakWave(Rectangle area)
{
	//cada segmento se evalua UNA vez contra el area del golpe: los que toca se rompen (y dejan de colisionar
	//en el mismo instante), los demas siguen intactos. Un segmento ya roto no cuenta de nuevo.
	int broken = 0;
	for (unsigned int i = 0; i < waves.size(); i++)
	{
		for (unsigned int j = 0; j < waves[i].segments.size(); j++)
		{
			waveSegment& s = waves[i].segments[j];
			if (s.alive && Collisions::CheckAABB(area, s.rect))
			{
				s.alive = false;
				s.breakTimer = 0;
				broken++;
			}
		}
	}
	return broken;
}
int boss::CountWaveSegments()
{
	int count = 0;
	for (unsigned int i = 0; i < waves.size(); i++)
	{
		for (unsigned int j = 0; j < waves[i].segments.size(); j++)
		{
			if (waves[i].segments[j].alive) count++;
		}
	}
	return count;
}
int boss::CountWaves()
{
	return (int)waves.size();
}
int boss::CollectAttacks(std::vector<bossProjectileRequest>& outRequests)
{
	if (pendingAttacks.empty()) return 0;

	outRequests = pendingAttacks;
	int count = (int)pendingAttacks.size();
	pendingAttacks.clear();
	return count;
}

// ---------------------------------------------------------------------------
// Dibujado
// ---------------------------------------------------------------------------

void boss::Draw()
{
	if (state == bossState::DEFEATED) return;

	//flotado suave, puramente visual (no afecta la hitbox real)
	float bobOffset = sinf(bobTimer * 2.0f) * 8.0f;

	//durante la fase vulnerable cambia a su version gris con grietas violetas (queda expuesto)
	bool vulnerable = (state == bossState::VULNERABLE_PHASE);
	Color tint = WHITE;
	if (flashTimer > 0) tint = RED;
	if (state == bossState::DEATH)
	{
		float t = stateTimer / deathDuration;
		if (t > 1.0f) t = 1.0f;
		tint = Fade(GRAY, 1.0f - t);
	}

	//hojas de 80x80 (el sprite original de 65x65 va adentro con margen para el salto y el polvo):
	//se dibuja con la base alineada a la base de la hitbox, a la escala que pide el tamaño del jefe
	const animSheet& idleSheet = vulnerable ? Art::boss2Idle : Art::boss1Idle;
	const animSheet& slamSheet = vulnerable ? Art::boss2Slam : Art::boss1Slam;
	const animSheet* sheet = &idleSheet;
	int frame = Art::LoopFrame(idleSheet, bobTimer);
	if (chargeTime > 0)
	{
		//agachandose (cuadros 0..2 de la hoja de golpe) y manteniendo la pose hasta soltar el ataque
		sheet = &slamSheet;
		frame = (int)(chargeTime / 0.1f);
		if (frame > 2) frame = 2;
	}
	else if (releaseTime >= 0 && releaseTime < 5 * slamSheet.frameTime)
	{
		//salto, golpe al piso y recuperacion (cuadros 3..7)
		sheet = &slamSheet;
		frame = 3 + (int)(releaseTime / slamSheet.frameTime);
	}
	float scale = size.x / 65.0f;
	Art::DrawFrame(*sheet, frame, { pos.x + size.x / 2, pos.y + size.y + bobOffset }, scale, false, false, tint);

	//aviso de la onda expansiva antes de que se dispare
	if (waveTelegraphing)
	{
		float alpha = 0.3f + 0.3f * sinf(bobTimer * 12.0f); //parpadeo para llamar la atencion
		DrawRectangle((int)waveStartX, (int)WAVE_TOP, (int)WAVE_WIDTH, (int)(WAVE_BOTTOM - WAVE_TOP), Fade(RED, alpha));
	}

	DrawWaves();

	//aviso simple de "esta por atacar" para el tiro dirigido y la rafaga
	if (aimedCharging || fanCharging)
	{
		DrawCircleLines((int)(pos.x + size.x / 2), (int)(pos.y + size.y / 2 + bobOffset), size.x * 0.75f, Fade(ORANGE, 0.8f));
	}
}
void boss::DrawWaves()
{
	for (unsigned int i = 0; i < waves.size(); i++)
	{
		//el fuego mira hacia donde viaja la onda (el sprite original apunta hacia arriba-derecha: -45 grados)
		float angle = (waves[i].dirX > 0 ? 0.0f : 180.0f) + 45.0f;

		for (unsigned int j = 0; j < waves[i].segments.size(); j++)
		{
			const waveSegment& s = waves[i].segments[j];
			Vector2 c = { s.rect.x + s.rect.width / 2, s.rect.y + s.rect.height / 2 };

			if (s.alive)
			{
				if (waveTexture.id != 0)
				{
					//cada segmento es una bola de fuego del tamaño de su alto; el desfasaje por fila evita que titilen todos juntos
					int frame = ((int)((bobTimer + j * 0.05f) / FIRE_FRAME_TIME)) % FIRE_FRAMES;
					float scale = s.rect.height / FIRE_SPRITE;
					Rectangle src = { frame * FIRE_FRAME, 0, FIRE_FRAME, FIRE_FRAME };
					Rectangle dst = { c.x, c.y, FIRE_FRAME * scale, FIRE_FRAME * scale };
					DrawTexturePro(waveTexture, src, dst, { FIRE_FRAME * scale / 2, FIRE_FRAME * scale / 2 }, angle, WHITE);
				}
				else
				{
					DrawRectangleRec(s.rect, Fade(ORANGE, 0.85f));
				}
			}
			else if (s.breakTimer < WAVE_BREAK_FX)
			{
				//destello al romperse: aro que se expande y se desvanece (no colisiona)
				float t = s.breakTimer / WAVE_BREAK_FX;
				DrawCircleLines((int)c.x, (int)c.y, 20.0f + 55.0f * t, Fade(YELLOW, 1.0f - t));
				DrawCircleLines((int)c.x, (int)c.y, 12.0f + 40.0f * t, Fade(ORANGE, 1.0f - t));
			}
		}
	}
}
void boss::DrawHealthBarInternal()
{
	float barWidth = 700;
	float barHeight = 34;
	float barX = GetScreenWidth() / 2.0f - barWidth / 2.0f;
	float barY = 40;

	float pct = (maxHP > 0) ? (float)currentHP / (float)maxHP : 0.0f;

	DrawRectangle((int)barX - 3, (int)barY - 3, (int)barWidth + 6, (int)barHeight + 6, BLACK);
	DrawRectangle((int)barX, (int)barY, (int)barWidth, (int)barHeight, Fade(MAROON, 0.6f));
	DrawRectangle((int)barX, (int)barY, (int)(barWidth * pct), (int)barHeight, RED);

	const char* label = TextFormat("JEFE FINAL   -   Ciclo %d/%d", currentCycle, cyclesRequired);
	DrawText(label, (int)(barX + barWidth / 2 - MeasureText(label, 22) / 2), (int)(barY - 28), 22, RAYWHITE);

	if (state == bossState::VULNERABLE_PHASE)
	{
		const char* vulnText = "¡VULNERABLE! Atacalo ahora";
		DrawText(vulnText, (int)(barX + barWidth / 2 - MeasureText(vulnText, 20) / 2), (int)(barY + barHeight + 6), 20, YELLOW);
	}
	else if (state == bossState::ATTACK_PHASE)
	{
		float remaining = attackPhaseDuration - stateTimer;
		if (remaining < 0) remaining = 0;
		const char* atkText = TextFormat("Esquiva sus ataques y rompe la pared de fuego: %.0fs", remaining);
		DrawText(atkText, (int)(barX + barWidth / 2 - MeasureText(atkText, 20) / 2), (int)(barY + barHeight + 6), 20, LIGHTGRAY);
	}
	else if (state == bossState::INTRO)
	{
		const char* introText = "¡El jefe final despierta!";
		DrawText(introText, (int)(barX + barWidth / 2 - MeasureText(introText, 20) / 2), (int)(barY + barHeight + 6), 20, LIGHTGRAY);
	}
	else if (state == bossState::DEATH)
	{
		const char* deathText = "¡Jefe derrotado!";
		DrawText(deathText, (int)(barX + barWidth / 2 - MeasureText(deathText, 20) / 2), (int)(barY + barHeight + 6), 20, GREEN);
	}
}
void boss::DrawHUD()
{
	if (state == bossState::DEFEATED) return;
	DrawHealthBarInternal();
}
