//
//  GUIContext.h
//  HSMR
//
//  Created by Yongsik Im on 2/6/25.
//
#ifndef __HS_GUI_CONTEXT_H__
#define __HS_GUI_CONTEXT_H__

#include "Precompile.h"

#include "ImGui/imgui.h"

#include <string>

namespace HS { struct EngineContext; }
namespace HS { class Window; }

HS_NS_EDITOR_BEGIN

class HS_EDITOR_API GUIContext
{
public:
	GUIContext(struct EngineContext* engineContext);
	~GUIContext();

	void Initialize();
	void Finalize();
	void NextFrame();
	void SetColorTheme(bool useWhite);
	void SetFont(const std::string& fontPath, float fontSize = 18.0f);

	void LoadLayout(const std::string& layoutPath);
	void SaveLayout(const std::string& layoutPath);

private:
	ImGuiContext* _context; // 네임스페이스 없이 사용
	ImFont* _font;
	std::string _defaultLayoutPath;

	EngineContext* _engineContext;
};

HS_NS_EDITOR_END

#endif
