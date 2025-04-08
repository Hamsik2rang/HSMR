//
//  GUIContext.h
//  HSMR
//
//  Created by Yongsik Im on 2/6/25.
//
#ifndef __HS_GUI_CONTEXT_H__
#define __HS_GUI_CONTEXT_H__

#include "Precompile.h"

#include <string>

class ImGuiContext;
class ImFont;

namespace HS
{
class EngineContext;
}

HS_NS_EDITOR_BEGIN

class HS_EDITOR_API GUIContext
{
public:
    GUIContext();
    ~GUIContext();

    void Initialize();
    void Finalize();

    void NextFrame();

    void SetColorTheme(bool useWhite);
    void SetFont(const std::string& fontPath, float fontSize = 18.0f);
    
    void LoadLayout(const std::string& layoutPath);
    void SaveLayout(const std::string& layoutPath);

private:
    ImGuiContext* _context;
    ImFont*       _font;
    std::string   _defaultLayoutPath;
};

GUIContext* hs_editor_get_gui_context();

HS_NS_EDITOR_END

#endif
