//
//  Input.cpp
//  HSMR
//
//  Created on 10/03/25.
//
#include "HAL/Input.h"


HS_NS_BEGIN

Input::ButtonInfo Input::s_button[static_cast<uint8>(Button::COUNT)];
Input::MoveInfo Input::s_move;
Input::ScrollInfo Input::s_scroll;

HS_NS_END