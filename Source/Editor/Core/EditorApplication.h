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

HS_NS_EDITOR_BEGIN

class HS_EDITOR_API EditorApplication : public Application
{
public:
	EditorApplication(const char* appName) noexcept;
	~EditorApplication() override;

	void Run() override;
	void Shutdown() override;

	GUIContext* GetGUIContext();

private:
	GUIContext* _guiContext;

	float _deltaTime = 0.0f;
};

HS_NS_EDITOR_END

#endif
