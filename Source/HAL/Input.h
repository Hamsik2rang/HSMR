#ifndef __HS_INPUT_H__
#define __HS_INPUT_H__

#include "Precompile.h"


HS_NS_BEGIN

enum class Key
{
	UNKNOWN,
	SPACE,
	APOSTROPHE,
	COMMA,
	MINUS,
	PERIOD,
	SLASH,
	NUM_0,
	NUM_1,
	NUM_2,
	NUM_3,
	NUM_4,
	NUM_5,
	NUM_6,
	NUM_7,
	NUM_8,
	NUM_9,
	SEMICOLON,
	EQUAL,
	A,
	B,
	C,
	D,
	E,
	F,
	G,
	H,
	I,
	J,
	K,
	L,
	M,
	N,
	O,
	P,
	Q,
	R,
	S,
	T,
	U,
	V,
	W,
	X,
	Y,
	Z,
	LEFT_BRACKET,
	RIGHT_BRACKET,
	//...
};

enum class Mouse
{
	LEFT,
	RIGHT,
	MIDDLE,
	X1,
	X2
};

class Input
{
public:
	static bool IsKeyPressed(Key key);
	static bool IsKeyReleased(Key key) { return !IsKeyPressed(key); }
	static bool IsMousePressed(Mouse button);
	static bool IsMouseReleased(Mouse button) { return !IsMousePressed(button); }
	static void GetMousePosition(double& x, double& y);
	static void SetMousePosition(double x, double y);

	static uint64 s_keyState[8];
	static uint64 s_mouseState[2];
};

HS_NS_END


#endif