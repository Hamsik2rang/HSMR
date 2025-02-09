//
//  InspectorPanel.h
//  Editor
//
//  Created by Yongsik Im on 2/8/25.
//
#ifndef __HS_INSPECTOR_PANEL_H__
#define __HS_INSPECTOR_PANEL_H__

#include "Precompile.h"

#include "Editor/Panel/Panel.h"

HS_NS_EDITOR_BEGIN

class HS_EDITOR_API InspectorPanel : public Panel
{
public:
    InspectorPanel(Window* window)
        : Panel(window)
    {}
    ~InspectorPanel() override;

    bool Setup() override;
    void Cleanup() override;

    void Draw() override;

private:
};

HS_NS_EDITOR_END

#endif /* __HS_INSPECTOR_PANEL_H__ */
