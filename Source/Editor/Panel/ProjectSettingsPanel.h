#ifndef __HS_EDITOR_PROJECT_SETTINGS_PANEL_H__
#define __HS_EDITOR_PROJECT_SETTINGS_PANEL_H__

#include "Precompile.h"

#include "Editor/Panel/Panel.h"

HS_NS_EDITOR_BEGIN

class HS_EDITOR_API ProjectSettingsPanel : public Panel
{
public:
    ProjectSettingsPanel(Window* window);
    ~ProjectSettingsPanel() override;

    bool Setup() override;
    void Cleanup() override;
    void Draw() override;

private:
    void drawStartupSceneSection();
    void browseStartupScene();
    void setCurrentSceneAsStartup();
};

HS_NS_EDITOR_END

#endif /* __HS_EDITOR_PROJECT_SETTINGS_PANEL_H__ */
