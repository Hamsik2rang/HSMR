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
#include "Renderer/RenderTarget.h"


#include "Geometry/GeometryDefinition.h"

namespace HS
{
class RHIFramebuffer;
}

HS_NS_EDITOR_BEGIN

class HS_EDITOR_API ScenePanel : public Panel
{
public:
    ScenePanel(Window* window)
        : Panel(window)
        , _resolution(800, 600)
    {}
    ~ScenePanel() override;

    bool Setup() override;
    void Cleanup() override;

    void Draw() override;

    HS_FORCEINLINE void SetSceneRenderTarget(RenderTarget* renderTarget) { _currentRenderTarget = renderTarget; }

    HS_FORCEINLINE Resolution GetResolution() const { return _resolution; }
    
private:
    Resolution _resolution;
    RenderTarget* _currentRenderTarget;
};

HS_NS_EDITOR_END

#endif /* ScenePanel_hpp */
