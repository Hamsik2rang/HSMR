//
//  EditorApplication.h
//  HSMR
//
//  Created by Yongsik Im on 2/7/25.
//
#ifndef __HS_EDITOR_APPLICATION_H__
#define __HS_EDITOR_APPLICATION_H__

#include "Precompile.h"

#include "Engine/Application.h"
#include "Engine/Window.h"

namespace hs { namespace editor { class GUIContext; } }
namespace hs { namespace editor { class ProjectLauncherWindow; } }
HS_NS_EDITOR_BEGIN

class HS_EDITOR_API EditorApplication : public Application
{
public:
	EditorApplication(const char* appName) noexcept;
	~EditorApplication() override;

	void Run() override;
	void Shutdown() override;

	GUIContext* GetGUIContext();

	// Command line project path (for double-clicking .hsproj)
	void SetProjectPath(const std::string& path) { _commandLineProjectPath = path; }

private:
	bool runLauncher();           // Phase 1: Project Launcher
	void runEditor();             // Phase 2: Editor Window

	std::string getProjectFromCommandLine();

	GUIContext* _guiContext = nullptr;
	ProjectLauncherWindow* _launcherWindow = nullptr;

	std::string _commandLineProjectPath;
	std::string _selectedProjectPath;

	float _deltaTime = 0.0f;
};

HS_NS_EDITOR_END

#endif
