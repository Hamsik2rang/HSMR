//
//  main.cpp
//  HSMR
//
//  Created by Yongsik Im on 1/29/25.
//

#ifdef HS_EDITOR_MODE
#include "Editor/EntryPoint/EditorMain.h"
#include "Editor/EntryPoint/SimpleMain.h"

#else // !HS_EDITOR_MODE
// #include ...

// #define hs_main(argc, argv) hs_play_main((argc), (argv))
#endif // HS_EDITOR_MODE

#include "Core/HAL/CommandLine.h"

int main(int argc, char* argv[])
{
    hs::CommandLine::Initialize(argc, argv);

#ifdef HS_EDITOR_MODE
    if (hs::CommandLine::HasFlag("-advanced"))
    {
        return hs_editor_main(argc, argv);
    }
    return hs_simple_main(argc, argv);
#else
    // return hs_play_main(argc, argv);
    return 0;
#endif
}
