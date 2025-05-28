#include "Editor/EntryPoint/EditorMain.h"

#include "Editor/Core/EditorApplication.h"
#include "Engine/Core/EngineContext.h"
#include "Engine/Platform/PlatformWindow.h"

#include "Engine/Core/Log.h"

using namespace HS;

int hs_editor_main(int argc, char* argv[])
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	HS::Application* app = new HS::Editor::EditorApplication("HSMR");

	hs_platform_init(app);

	EngineContext* context = hs_engine_create_context("HSMR", ERHIPlatform::VULKAN);

	app->Initialize(context);

	app->Run();

#ifdef _DEBUG
	HS_ASSERT(_CrtCheckMemory(), "Memory Leak tracked.");
#endif

	return 0;
}