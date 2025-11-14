#include "Engine/Application.h"


HS_NS_BEGIN

Application::Application(const char* appName) noexcept
	: _name(appName)
	, _window(nullptr)
{
}


HS_NS_END