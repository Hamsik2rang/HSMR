#include "PlatformApplicationWindows.h"

#include "Engine/Core/Application.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

HINSTANCE g_hInstance = NULL;

void hs_platform_set_hinsatnce(void* hInstance)
{
	g_hInstance = (HINSTANCE)hInstance;
}

void* hs_platform_get_hinstance()
{
	if (!g_hInstance)
	{
		g_hInstance = GetModuleHandleW(nullptr);
	}

	return static_cast<void*>(g_hInstance);
}

HS_NS_BEGIN


void hs_platform_init(Application* hsApp)
{
	//...
}

void hs_platform_shutdown(Application* hsApp)
{
	//...
}

HS_NS_END
