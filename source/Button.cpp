#include "Button.h"
#include "Collisions.h"
#include "Art.h"

button::button()
{
	bounds = { 0,0,0,0 };
	pressed = false;
	linkedDoorIndex = -1;
}
button::~button()
{
}
void button::Init(Rectangle boundsVal, int doorIndex)
{
	bounds = boundsVal;
	linkedDoorIndex = doorIndex;
	pressed = false;

}
bool button::CheckPressed(Rectangle playerHitbox)
{
	if (pressed) return false;

	if (Collisions::CheckAABB(bounds, playerHitbox))
	{
		pressed = true;
		return true;
	}
	return false;
}
void button::Reset()
{
	pressed = false;
}
int button::GetLinkedDoorIndex()
{
	return linkedDoorIndex;
}
bool button::IsPressed()
{
	return pressed;
}
void button::Draw()
{
	//hoja de 2 cuadros de 60x20: 0 = suelto (gema roja), 1 = pisado (gema verde)
	Rectangle src = { pressed ? 60.0f : 0.0f, 0, 60, 20 };
	DrawTexturePro(Art::buttonSheet, src, bounds, { 0,0 }, 0, WHITE);
}
