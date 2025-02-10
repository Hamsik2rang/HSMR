//
//  ScenePanel.hpp
//  Editor
//
//  Created by Yongsik Im on 2/8/25.
//

#ifndef __HS_SCENE_PANEL_H__
#define __HS_SCENE_PANEL_H__

#include "Precompile.h"

#include "Editor/Panel/Panel.h"

namespace HS
{
class Framebuffer;
}
HS_NS_EDITOR_BEGIN

class HS_EDITOR_API ScenePanel : public Panel
{
public:
    ScenePanel(Window* window)
        : Panel(window)
    {}
    ~ScenePanel() override;

    bool Setup() override;
    void Cleanup() override;

    void Draw() override;

    void SetSceneRenderFramebuffer(Framebuffer* framebuffer) { _currentFramebuffer = framebuffer; }

private:
    Framebuffer* _currentFramebuffer;
};

HS_NS_EDITOR_END

#endif /* ScenePanel_hpp */
