#include "Editor/EntryPoint/EditorMain.h"

#include "Editor/Core/EditorApplication.h"
#include "Engine/Window.h"

#include "Core/Log.h"
#include "Core/HAL/CommandLine.h"
#include "Core/HAL/Timer.h"
#include "Core/Native/NativeWindow.h"
#include "Core/Profiler/ProfileDataCollector.h"
#include "Core/SystemContext.h"
#include "RHI/RHIContext.h"

using namespace hs;

int hs_editor_main(int argc, char* argv[])
{
#ifdef _DEBUG
	_CrtMemState crtStartState;
	int crtFlags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	crtFlags |= _CRTDBG_ALLOC_MEM_DF;
	crtFlags &= ~_CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag(crtFlags);
	_CrtMemCheckpoint(&crtStartState);
#endif

	hs::Application* app = new hs::editor::EditorApplication("HSMR");

	app->Run();

	app->Shutdown();
	delete app;

	FinalizeNativeWindowSystem();
	RHIContext::Destroy();
	SystemContext::Destroy();
	CommandLine::Finalize();
	Timer::Finalize();

#ifdef _DEBUG
	_CrtMemState crtEndState;
	_CrtMemState crtDiffState;
	_CrtMemCheckpoint(&crtEndState);
	if (_CrtMemDifference(&crtDiffState, &crtStartState, &crtEndState))
	{
		_CrtMemDumpStatistics(&crtDiffState);
		_CrtMemDumpAllObjectsSince(&crtStartState);
		HS_ASSERT(false, "Memory leaks detected during application lifetime.");
	}
	HS_ASSERT(_CrtCheckMemory(), "CRT heap corruption detected.");
#endif

	return 0;
}
