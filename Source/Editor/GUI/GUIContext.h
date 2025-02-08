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

class ImGuiContext;

namespace HS
{
class EngineContext;
}

HS_NS_EDITOR_BEGIN

class HS_EDITOR_API GUIContext
{
public:
    GUIContext() = default;
    ~GUIContext();

    void Initialize();
    void Finalize();

    void SetColorTheme(bool useWhite);
    void SetFont(std::string& fontPath, float fontSize = 18.0f);

private:
    ImGuiContext* _context;
    ImFont* _font;
    std::string iniFileName = "imgui.ini";
};

GUIContext* hs_editor_get_gui_context();

HS_NS_EDITOR_END

#endif
