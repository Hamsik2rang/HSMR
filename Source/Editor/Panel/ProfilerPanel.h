//
//  ProfilerPanel.h
//  Editor
//
//  Dockable profiler panel with CPU zone tree view.
//

#ifndef __HS_EDITOR_PROFILER_PANEL_H__
#define __HS_EDITOR_PROFILER_PANEL_H__

#include "Precompile.h"
#include "Editor/Panel/Panel.h"

HS_NS_EDITOR_BEGIN

class HS_EDITOR_API ProfilerPanel : public Panel
{
public:
    ProfilerPanel(Window* window);
    ~ProfilerPanel() override;

    bool Setup() override;
    void Cleanup() override;
    void Draw() override;

private:
    void _drawCPUTab();
    void _drawGPUTab();
    void _drawMemoryTab();
    void _drawZoneBar(float fraction, uint32 color, float width, float height);
};

HS_NS_EDITOR_END

#endif /* __HS_EDITOR_PROFILER_PANEL_H__ */
