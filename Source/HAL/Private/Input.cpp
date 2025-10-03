//
//  Input.cpp
//  HSMR
//
//  Created on 10/03/25.
//
#include "HAL/Input.h"


HS_NS_BEGIN

uint64 Input::_keyState[8] = {};
uint64 Input::_mouseState[2] = {};


bool Input::IsKeyPressed(Key key)
{
	if (key < Key::UNKNOWN || key > Key::RIGHT_BRACKET)
		return false;
	uint64 mask = 1ULL << (static_cast<int>(key) % 64);
	return (_keyState[static_cast<int>(key) / 64] & mask) != 0;
}

bool Input::IsMousePressed(Mouse button)
{
	if (button < Mouse::LEFT || button > Mouse::X2)
		return false;
	uint64 mask = 1ULL << (static_cast<int>(button) % 64);
	return (_mouseState[static_cast<int>(button) / 64] & mask) != 0;
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