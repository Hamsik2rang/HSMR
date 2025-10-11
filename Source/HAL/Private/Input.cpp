//
//  Input.cpp
//  HSMR
//
//  Created on 10/03/25.
//
#include "HAL/Input.h"


HS_NS_BEGIN

uint64 Input::s_keyState[4] = {};
uint8 Input::s_mouseState = {};


bool Input::IsKeyPressed(Key key)
{
	if (static_cast<uint8>(key) > static_cast<uint8>(Key::COUNT))
	{
		return false;
	}
	uint64 mask = 1ULL << (static_cast<int>(key) % 64);
	return (s_keyState[static_cast<int>(key) / 64] & mask) != 0;
}

bool Input::IsMousePressed(Mouse button)
{
	if (static_cast<uint8>(button) >= static_cast<uint8>(Mouse::COUNT))
	{
		return false;
	}
	uint64 mask = 1ULL << (static_cast<int>(button) % 64);
	return (s_mouseState & mask) != 0;
}

void Input::GetMousePosition(double& x, double& y)
{
	//Get Native Mouse Position
};

void Input::SetMousePosition(double x, double y)
{
	//Set Native Mouse Position
};


HS_NS_END