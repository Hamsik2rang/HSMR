//
//  main.cpp
//  HSMR
//
//  Created by Yongsik Im on 1/29/25.
//

#ifdef HS_EDITOR_MODE
#include "Editor/EntryPoint/EditorMain.h"

//extern HS_EDITOR_API int hs_editor_main(int argc, char* argv[]);

#else
// extern int hs_play_main(int argc, char* argv[]);
#endif

int main(int argc, char* argv[])
{
    return hs_editor_main(argc, argv);
}


