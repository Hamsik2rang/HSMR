//
//  MenuPanel.h
//  Editor
//
//  Created by Yongsik Im on 2/9/25.
//

#ifndef __HS_MENU_PANEL_H__
#define __HS_MENU_PANEL_H__

#include "Precompile.h"

#include "Editor/Panel/Panel.h"

HS_NS_EDITOR_BEGIN

class HS_EDITOR_API MenuPanel : public Panel
{
public:
    MenuPanel(Window* window) : Panel(window) {}
    ~MenuPanel() override;
    
    bool Setup() override;
    void Cleanup() override;
    
    void Draw() override;
private:
    //...
};


HS_NS_EDITOR_END


#endif /* __HS_MENU_PANEL_H__ */
