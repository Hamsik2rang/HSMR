//
//  HierarchyPanel.h
//  Editor
//
//  Created by Yongsik Im on 10/08/25.
//
#ifndef __HS_HIERARCHY_PANEL_H__
#define __HS_HIERARCHY_PANEL_H__

#include "Precompile.h"

#include "Editor/Panel/Panel.h"

HS_NS_EDITOR_BEGIN

class HierarchyPanel : public Panel
{
public:
	HierarchyPanel(Window* window);
	~HierarchyPanel() override = default;

	bool Setup() override;

	void Cleanup() override;

	void Draw() override;


private:

};




HS_NS_EDITOR_END

#endif