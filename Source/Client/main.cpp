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

#else

#ifndef UNICODE
#define UNICODE
#endif 

//#include "HAL/Windows/Win.h"


#endif

#else
// #include ...

// #define hs_main(argc, argv) hs_play_main((argc), (argv))
#endif


int main(int argc, char* argv[])
{
	return hs_main(argc, argv);
}

