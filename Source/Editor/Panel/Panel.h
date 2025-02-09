//
//  Panel.h
//  HSMR
//
//  Created by Yongsik Im on 2/8/25.
//
#ifndef __HS_PANEL_H__
#define __HS_PANEL_H__

#include "Precompile.h"

#include "Engine/Core/Window.h"

#include <vector>

HS_NS_EDITOR_BEGIN

class HS_EDITOR_API Panel
{
public:
    Panel(Window* window) : _window(window) {}
    virtual ~Panel() {}

    virtual bool Setup() = 0;

    virtual void Cleanup() = 0;

    void InsertPanel(Panel* panel);
    void RemovePanel(Panel* panel);

    virtual void Draw() = 0;

protected:
    std::vector<Panel*> _childs;

    Window* _window;
};

HS_NS_EDITOR_END

#endif
