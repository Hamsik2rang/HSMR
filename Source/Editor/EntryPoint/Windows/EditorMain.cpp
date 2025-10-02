#include "Editor/EntryPoint/EditorMain.h"

#include "Editor/Core/EditorApplication.h"
#include "Engine/EngineContext.h"
#include "Engine/Window.h"

#include "Core/Log.h"

using namespace HS;

int hs_editor_main(int argc, char* argv[])
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	EngineContext* context = new EngineContext("HSMR", ERHIPlatform::VULKAN);

	HS::Application* app = new HS::Editor::EditorApplication("HSMR", context);

	app->Run();

#ifdef _DEBUG
	HS_ASSERT(_CrtCheckMemory(), "Memory Leak tracked.");
#endif

	app->Shutdown();

	return 0;
}