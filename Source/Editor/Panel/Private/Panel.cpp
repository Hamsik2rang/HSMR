#include "Editor/Panel/Panel.h"

HS_NS_EDITOR_BEGIN

void Panel::InsertPanel(Panel* panel)
{
    if (!panel)
    {
        return;
    }

    for (Panel* child : _childs)
    {
        if (child == panel)
        {
            return;
        }
    }

    panel->_parent = this;
    _childs.push_back(panel);
}

void Panel::RemovePanel(Panel* panel)
{

    int index = -1;
    for (size_t i = 0; i < _childs.size(); i++)
    {
        if (_childs[i] == panel)
        {
            index = static_cast<decltype(index)>(i);
            break;
        }
    }

    if (index >= 0)
    {
        _childs[index]->_parent = nullptr;
        std::swap(_childs[index], _childs.back());
        _childs.pop_back();
    }
}

HS_NS_EDITOR_END
