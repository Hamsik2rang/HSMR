//
//  main.cpp
//  HSMR
//
//  Created by Yongsik Im on 1/29/25.
//

#ifdef HS_EDITOR_MODE
#include "Editor/EntryPoint/EditorMain.h"

#define hs_main(argc, argv) hs_editor_main((argc), (argv))
#else
// #include ...

// #define hs_main(argc, argv) hs_play_main((argc), (argv))
#endif

int main(int argc, char* argv[])
{
    return hs_main(argc, argv);
}


