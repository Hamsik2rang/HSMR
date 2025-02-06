//
//  GUIContext.h
//  HSMR
//
//  Created by Yongsik Im on 2/6/25.
//
#ifndef __HS_GUI_CONTEXT_H__
#define __HS_GUI_CONTEXT_H__

#include "Precompile.h"

HS_NS_EDITOR_BEGIN

class HS_EDITOR_API GUIContext
{
public:
    GUIContext() = default;
    ~GUIContext();
    
    void Initialize();
    void Finalize();
    
    void SetColorTheme(bool useWhite);
    
private:
    ImGuiContext _context;
};

GUIContext* hs_editor_get_gui_context();

HS_NS_EDITOR_END

#endif
