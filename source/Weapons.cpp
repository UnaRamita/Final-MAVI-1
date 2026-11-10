#include "Weapons.h"
#include "Collisions.h"
#include <math.h>

namespace
{
	//---------------- espada: combo de 3 golpes ----------------
	const float SWORD_FW = 320, SWORD_FH = 320;     //tamaño de cada frame de las hojas "Great sword attack N"
	const float SWORD_AX = 120, SWORD_AY = 150;     //ancla dentro del frame (empuñadura en reposo)
	const int   SWORD_FRAMES[3] = { 6, 6, 7 };
	const int   SWORD_SWING_FRAME[3] = { 2, 2, 3 }; //frame donde arranca el tajo (suena SliceSwordN)
	const int   SWORD_HIT_FRAME[3] = { 4, 3, 5 };   //frame donde la hoja conecta (se aplica el daño)
	const float SWORD_DAMAGE_MUL[3] = { 1.0f, 1.0f, 2.0f };

	//duracion (s) de cada frame. El frame 0 de cada golpe es la pose final del anterior (sin estela):
	//por eso todos los golpes arrancan en el frame 1 y encadenan sin saltos visuales.
	const float SWORD_DUR[3][7] = {
		{ 0.00f, 0.07f, 0.05f, 0.05f, 0.05f, 0.10f, 0.00f },   //golpe 1: base
		{ 0.00f, 0.07f, 0.05f, 0.05f, 0.05f, 0.10f, 0.00f },   //golpe 2: de abajo hacia arriba
		{ 0.00f, 0.10f, 0.10f, 0.045f, 0.045f, 0.06f, 0.16f }  //golpe 3: carga + tajo largo de arriba hacia abajo
	};
	const float COMBO_WINDOW = 0.40f;        //tiempo para encadenar el siguiente golpe despues de terminar uno
	const float COMBO_END_COOLDOWN = 0.25f;  //pausa tras el tercer golpe antes de poder empezar otro combo

	//---------------- baston ----------------
	const float STAFF_FW = 160, STAFF_FH = 160;     //frames de "magic staff attack"
	const float STAFF_AX = 80, STAFF_AY = 80;
	const int   STAFF_FRAMES = 7;
	const int   STAFF_SHOT_FRAME = 4;               //el disparo sale al empezar el tiron hacia adelante
	const float STAFF_DUR[7] = { 0.00f, 0.05f, 0.06f, 0.08f, 0.04f, 0.07f, 0.09f };
	const float STAFF_COOLDOWN = 0.6f;
	//posicion de la gema (boca del baston) en cada frame, relativa al ancla, mirando a la derecha
	const float STAFF_MUZZLE[7][2] = {
		{ 23, -18 }, { 15, -20 }, { 7, -22 }, { 2, -22 }, { 38, -16 }, { 47, -14 }, { 33, -16 }
	};

	//zona de daño de cada golpe (mirando a la derecha); el 3ro es mas alto y de mucho mas alcance
	Rectangle SwordHitRect(int step, Vector2 p, Vector2 s, bool left)
	{
		float w = 96, top = 0, h = s.y;
		if (step == 2) { w = 108; top = -30; h = s.y + 30; }
		else if (step == 3) { w = 190; top = -60; h = s.y + 80; }

		float x = left ? p.x + 6 - w : p.x + s.x - 6;
		return { x, p.y + top, w, h };
	}
}

//---------------- weapon ----------------
weapon::weapon()
{
	type = weaponType::NONE;
	for (int i = 0; i < 3; i++) swordAnim[i] = { 0 };
	staffAnim = { 0 };

	pPos = { 0,0 };
	pSize = { 64,128 };
	pLeft = false;

	attacking = false;
	frame = 0;
	frameTimer = 0;
	idleTime = 0;

	comboStep = 0;
	queuedNext = false;
	comboWindow = 0;
	endCooldown = 0;

	cooldownTimer = 0;
}
weapon::~weapon()
{
}
void weapon::Init()
{
	swordAnim[0] = LoadTexture("Assets/Sprites/Weapons/Great sword attack 1.png");
	swordAnim[1] = LoadTexture("Assets/Sprites/Weapons/Great sword attack 2.png");
	swordAnim[2] = LoadTexture("Assets/Sprites/Weapons/Great sword attack 3.png");
	staffAnim = LoadTexture("Assets/Sprites/Weapons/magic staff attack.png");
}
void weapon::Unload()
{
	for (int i = 0; i < 3; i++) UnloadTexture(swordAnim[i]);
	UnloadTexture(staffAnim);
}
void weapon::Equip(weaponType newType)
{
	//al equipar un arma nueva se reemplaza la anterior: solo se porta una a la vez
	type = newType;

	//cualquier ataque / combo en curso se cancela
	attacking = false;
	frame = 0;
	frameTimer = 0;
	comboStep = 0;
	queuedNext = false;
	comboWindow = 0;
	endCooldown = 0;
	cooldownTimer = 0;
	events.clear();
}
weaponType weapon::GetType()
{
	return type;
}

Vector2 weapon::AnchorWorld(Vector2 playerPos, Vector2 playerSize, bool left, float bob)
{
	//el arma flota delante del jugador, a la altura del pecho. La separacion (18 px) deja que la punta de la
	//espada en guardia no toque la cabeza (antes eran 10 px)
	const float gap = 18.0f;
	return { left ? playerPos.x - gap : playerPos.x + playerSize.x + gap, playerPos.y + 62 + bob };
}

// ---------------------------------------------------------------------------
// Logica
// ---------------------------------------------------------------------------

void weapon::Update(float dt, Vector2 playerPos, Vector2 playerSize, bool facingLeft)
{
	pPos = playerPos;
	pSize = playerSize;
	pLeft = facingLeft;

	idleTime += dt;
	if (cooldownTimer > 0) cooldownTimer -= dt;
	if (endCooldown > 0) endCooldown -= dt;

	if (type == weaponType::SWORD) UpdateSword(dt);
	else if (type == weaponType::STAFF) UpdateStaff(dt);
}
bool weapon::TryAttack()
{
	if (type == weaponType::NONE) return false;

	if (type == weaponType::SWORD)
	{
		if (endCooldown > 0) return false;

		if (!attacking)
		{
			//si todavia esta abierta la ventana del combo se sigue con el golpe que corresponde; si no, se empieza de cero
			int next = (comboWindow > 0 && comboStep >= 1 && comboStep < 3) ? comboStep + 1 : 1;
			StartSwordStep(next);
			return true;
		}

		//apretar durante un golpe lo deja "en cola" para encadenar apenas termine (hasta el golpe 3)
		if (comboStep < 3) queuedNext = true;
		return true;
	}

	//baston: un hechizo a la vez y con un tiempo minimo entre disparos
	if (attacking || cooldownTimer > 0) return false;
	attacking = true;
	frame = 1;
	frameTimer = 0;
	cooldownTimer = STAFF_COOLDOWN;
	OnStaffFrameEnter(frame);
	return true;
}
bool weapon::TryHeavyAttack()
{
	if (type != weaponType::SWORD) return false;
	if (endCooldown > 0) return false;                //todavia en la pausa del combo anterior
	if (attacking && comboStep == 3) return false;    //ya esta haciendo el golpe fuerte

	StartSwordStep(3);
	return true;
}
void weapon::StartSwordStep(int step)
{
	comboStep = step;
	attacking = true;
	frame = 1;           //el frame 0 es la pose final del golpe anterior, no hace falta mostrarlo
	frameTimer = 0;
	queuedNext = false;
	comboWindow = 0;
	OnSwordFrameEnter(step, frame);
}
void weapon::UpdateSword(float dt)
{
	if (attacking)
	{
		frameTimer += dt;
		while (attacking)
		{
			int s = comboStep - 1;
			float dur = SWORD_DUR[s][frame];
			if (frameTimer < dur) break;

			frameTimer -= dur;
			frame++;
			if (frame >= SWORD_FRAMES[s])
			{
				attacking = false;
				frame = SWORD_FRAMES[s] - 1;

				if (queuedNext && comboStep < 3)
				{
					StartSwordStep(comboStep + 1); //se encadena sin pausa con el siguiente golpe
				}
				else if (comboStep < 3)
				{
					comboWindow = COMBO_WINDOW;    //queda abierta la ventana para encadenar apretando de nuevo
				}
				else
				{
					comboStep = 0;                 //combo completo
					endCooldown = COMBO_END_COOLDOWN;
				}
			}
			else
			{
				OnSwordFrameEnter(comboStep, frame);
			}
		}
	}
	else if (comboWindow > 0)
	{
		comboWindow -= dt;
		if (comboWindow <= 0) comboStep = 0; //se perdio el combo: el proximo ataque vuelve al golpe 1
	}
}
void weapon::OnSwordFrameEnter(int step, int f)
{
	int s = step - 1;

	if (f == SWORD_SWING_FRAME[s])
	{
		weaponEvent ev = {};
		ev.kind = weaponEventType::SWING;
		ev.comboStep = step;
		ev.facingLeft = pLeft;
		events.push_back(ev);
	}
	if (f == SWORD_HIT_FRAME[s])
	{
		weaponEvent ev = {};
		ev.kind = weaponEventType::SWORD_HIT;
		ev.comboStep = step;
		ev.hitRect = SwordHitRect(step, pPos, pSize, pLeft);
		ev.damageMul = SWORD_DAMAGE_MUL[s];
		ev.facingLeft = pLeft;
		events.push_back(ev);
	}
}
void weapon::UpdateStaff(float dt)
{
	if (!attacking) return;

	frameTimer += dt;
	while (attacking && frameTimer >= STAFF_DUR[frame])
	{
		frameTimer -= STAFF_DUR[frame];
		frame++;
		if (frame >= STAFF_FRAMES)
		{
			attacking = false;
			frame = 0;
		}
		else
		{
			OnStaffFrameEnter(frame);
		}
	}
}
void weapon::OnStaffFrameEnter(int f)
{
	if (f != STAFF_SHOT_FRAME) return;

	Vector2 a = AnchorWorld(pPos, pSize, pLeft, 0);
	weaponEvent ev = {};
	ev.kind = weaponEventType::STAFF_SHOT;
	ev.muzzle = { a.x + (pLeft ? -STAFF_MUZZLE[f][0] : STAFF_MUZZLE[f][0]), a.y + STAFF_MUZZLE[f][1] };
	ev.facingLeft = pLeft;
	events.push_back(ev);
}
bool weapon::ConsumeEvent(weaponEvent& out)
{
	if (events.empty()) return false;

	out = events.front();
	events.erase(events.begin());
	return true;
}

// ---------------------------------------------------------------------------
// Dibujado
// ---------------------------------------------------------------------------

void weapon::Draw(Vector2 playerPos, Vector2 playerSize, bool facingLeft)
{
	if (type == weaponType::NONE) return;

	bool inComboWindow = (type == weaponType::SWORD && !attacking && comboWindow > 0 && comboStep >= 1 && comboStep < 3);
	bool idle = !attacking && !inComboWindow;
	float bob = idle ? sinf(idleTime * 2.2f) * 4.0f : 0.0f;   //flotado suave en reposo
	Vector2 a = AnchorWorld(playerPos, playerSize, facingLeft, bob);

	Texture2D sheet;
	int fr = 0;
	float fw, fh, ax, ay;
	if (type == weaponType::SWORD)
	{
		fw = SWORD_FW; fh = SWORD_FH; ax = SWORD_AX; ay = SWORD_AY;
		if (attacking)
		{
			sheet = swordAnim[comboStep - 1];
			fr = frame;
		}
		else if (inComboWindow)
		{
			//mientras espera el siguiente golpe se muestra su frame 0 = pose final del golpe anterior, sin estela
			sheet = swordAnim[comboStep];
			fr = 0;
		}
		else
		{
			sheet = swordAnim[0];  //en reposo: pose de guardia (frame 0 del golpe 1)
			fr = 0;
		}
	}
	else
	{
		fw = STAFF_FW; fh = STAFF_FH; ax = STAFF_AX; ay = STAFF_AY;
		sheet = staffAnim;
		fr = attacking ? frame : 0;
	}

	//las hojas estan dibujadas mirando a la derecha; para la izquierda se espeja (ancla incluida)
	Rectangle src = { fr * fw, 0, facingLeft ? -fw : fw, fh };
	float dstX = facingLeft ? a.x - (fw - ax) : a.x - ax;
	Rectangle dst = { dstX, a.y - ay, fw, fh };
	DrawTexturePro(sheet, src, dst, { 0,0 }, 0, WHITE);
}

//---------------- weaponPickup ----------------
weaponPickup::weaponPickup()
{
	pos = { 0,0 };
	size = { 48,48 };
	type = weaponType::NONE;
	texture = { 0 };
	collected = false;
}
weaponPickup::~weaponPickup()
{
}
void weaponPickup::Init(Vector2 position, weaponType pickupType, Texture2D tex)
{
	pos = position;
	type = pickupType;
	texture = tex;
	collected = false;
}
bool weaponPickup::IsPlayerNear(Rectangle playerHitbox)
{
	if (collected) return false;

	Rectangle hitbox = { pos.x, pos.y, size.x, size.y };
	return Collisions::CheckAABB(hitbox, playerHitbox);
}
bool weaponPickup::CheckPickup(Rectangle playerHitbox)
{
	//ya no se recoge solo al pisarla: hay que estar encima y apretar E
	if (!IsPlayerNear(playerHitbox)) return false;
	if (!IsKeyPressed(KEY_E)) return false;

	collected = true;
	return true;
}
void weaponPickup::DrawPrompt(Rectangle playerHitbox)
{
	if (!IsPlayerNear(playerHitbox)) return;

	//tecla "E" con un leve vaiven, encima de la cabeza del jugador (el arma queda tapada por el), y el texto de la accion arriba
	float bob = sinf((float)GetTime() * 5.0f) * 3.0f;
	float cx = playerHitbox.x + playerHitbox.width / 2;
	Rectangle key = { cx - 16, playerHitbox.y - 52 + bob, 32, 32 };
	DrawRectangleRec({ key.x + 2, key.y + 3, key.width, key.height }, Fade(BLACK, 0.45f)); //sombra
	DrawRectangleRec(key, RAYWHITE);
	DrawRectangleLinesEx(key, 3, DARKGRAY);
	DrawText("E", (int)(cx - MeasureText("E", 24) / 2.0f), (int)(key.y + 4), 24, BLACK);

	const char* label = "Recoger";
	DrawText(label, (int)(cx - MeasureText(label, 20) / 2.0f) + 1, (int)(key.y - 24) + 1, 20, BLACK);
	DrawText(label, (int)(cx - MeasureText(label, 20) / 2.0f), (int)(key.y - 24), 20, RAYWHITE);
}
void weaponPickup::Draw()
{
	if (collected) return;
	DrawTextureEx(texture, pos, 0, size.x / (float)texture.width, WHITE);
}
bool weaponPickup::IsCollected()
{
	return collected;
}
void weaponPickup::Reset()
{
	collected = false;
}
weaponType weaponPickup::GetType()
{
	return type;
}
void weaponPickup::ReplaceWith(weaponType newType, Texture2D newTexture)
{
	//el arma que el jugador tenia en la mano queda tirada en el mismo lugar, lista para recogerse de nuevo
	type = newType;
	texture = newTexture;
	collected = false;
}
