//
//  main.cpp
//  HSMR
//
//  Created by Yongsik Im on 1/29/25.
//

#ifdef HS_EDITOR_MODE
#include "Editor/EntryPoint/EditorMain.h"

#define hs_main(argc, argv) hs_editor_main((argc), (argv))

#include <cstdlib>   // For malloc/free
#include <cstdio>    // For printf/logging

#if defined (__APPLE__)
#include <execinfo.h> // For backtrace (optional)
#include <unistd.h>

int main(int argc, char* argv[])
{
    return hs_main(argc, argv);
}

#else

#ifndef UNICODE
#define UNICODE
#endif 

#include "Engine/Platform/Windows/PlatformApplicationWindows.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	hs_platform_set_hinsatnce(hInstance);

	return hs_main(__argc, __argv);
}


#endif

#else
// #include ...

// #define hs_main(argc, argv) hs_play_main((argc), (argv))
#endif

