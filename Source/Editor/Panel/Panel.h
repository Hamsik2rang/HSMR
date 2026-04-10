//
//  Panel.h
//  Editor
//
//  Created by Yongsik Im on 2/8/25.
//
#ifndef __HS_PANEL_H__
#define __HS_PANEL_H__

#include "Precompile.h"

#include "Engine/Window.h"
#include "Editor/GUI/GUIContext.h"

#include <vector>

HS_NS_EDITOR_BEGIN

class HS_EDITOR_API Panel
{
public:
    Panel(Window* window, const char* panelId = nullptr, bool* visibilityBinding = nullptr)
        : _window(window)
        , _panelId(panelId ? panelId : "")
        , _visibilityBinding(visibilityBinding)
    {}
    virtual ~Panel() {}

    virtual bool Setup() = 0;

    virtual void Cleanup() = 0;
    
    virtual void Update(float deltaTime) {}

    void BindVisibility(bool* visibilityBinding) { _visibilityBinding = visibilityBinding; }
    bool* GetVisibilityBinding() const { return _visibilityBinding; }
    bool IsVisible() const { return _visibilityBinding == nullptr || *_visibilityBinding; }

    void SetPanelId(const char* panelId) { _panelId = panelId ? panelId : ""; }
    const std::string& GetPanelId() const { return _panelId; }

    void InsertPanel(Panel* panel);
    void RemovePanel(Panel* panel);

    virtual void Draw() = 0;

protected:
    std::vector<Panel*> _childs;

    Window* _window;
    std::string _panelId;
    bool* _visibilityBinding = nullptr;
    Panel* _parent = nullptr;
};

HS_NS_EDITOR_END

#endif
