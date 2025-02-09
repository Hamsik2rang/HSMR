//
//  DockspacePanel.h
//  Editor
//
//  Created by Yongsik Im on 2/8/25.
//
#ifndef __HS_DOCKSPACE_PANEL_H__
#define __HS_DOCKSPACE_PANEL_H__

#include "Precompile.h"

#include "Editor/Panel/Panel.h"

HS_NS_EDITOR_BEGIN

class HS_EDITOR_API DockspacePanel : public Panel
{
public:
    DockspacePanel(Window* window)
        : Panel(window)
    {}
    ~DockspacePanel() override;

    bool Setup() override;
    void Cleanup() override;

    void Draw() override;

private:
    bool _hasPadding = false;
};

HS_NS_EDITOR_END

#endif /* DockSpacePanel_h */
