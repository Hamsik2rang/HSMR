//
//  GamePanel.h
//  Editor
//
//  Runtime-style scene preview panel driven by scene cameras.
//

#ifndef __HS_EDITOR_GAME_PANEL_H__
#define __HS_EDITOR_GAME_PANEL_H__

#include "Precompile.h"

#include "Editor/Panel/Panel.h"
#include "Renderer/RenderTarget.h"
#include "Resource/GeometryDefinition.h"
#include "Scene/Entity.h"

#include <string>

HS_NS_EDITOR_BEGIN

class HS_EDITOR_API GamePanel : public Panel
{
public:
    GamePanel(Window* window);
    ~GamePanel() override;

    bool Setup() override;
    void Cleanup() override;
    void Update(float deltaTime) override;
    void Draw() override;

    RenderTarget* GetRenderTarget(uint32 imageIndex);

    Resolution GetResolution() const { return _resolution; }
    Entity ResolveCamera(Scene* scene) const;

private:
    void drawMenuBar(Scene* scene);
    std::string getCameraLabel(Entity camera) const;

    Resolution _resolution;
    std::vector<RenderTarget> _panelRenderTargets;
    bool _useAutoCamera = true;
    Entity _selectedCamera;
};

HS_NS_EDITOR_END

#endif /* __HS_EDITOR_GAME_PANEL_H__ */
