#include "Editor/Panel/Panel.h"

#include "ImGui/imgui.h"

HS_NS_EDITOR_BEGIN

void Panel::InsertPanel(Panel* panel)
{
    _childs.push_back(panel);
}

void Panel::RemovePanel(Panel* panel)
{

    int index = -1;
    for (size_t i = 0; i < _childs.size(); i++)
    {
        if (_childs[i] == panel)
        {
            index = i;
            break;
        }
    }

    if (index >= 0)
    {
        std::swap(_childs[index], _childs.back());
        _childs.pop_back();
    }
}

HS_NS_EDITOR_END
