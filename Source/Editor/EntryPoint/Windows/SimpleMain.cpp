#include "Editor/EntryPoint/SimpleMain.h"

#include "Editor/Core/SimpleApplication.h"
#include "Engine/Window.h"

#include "Core/Log.h"

int hs_simple_main(int argc, char* argv[])
{
#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    hs::Application* app = new hs::editor::SimpleApplication("HSMR Simple");

    app->Run();

#ifdef _DEBUG
    HS_ASSERT(_CrtCheckMemory(), "Memory Leak tracked.");
#endif

    app->Shutdown();

    delete app;

    return 0;
}
