#ifndef __HS_RESOURCE_PANEL_H__
#define __HS_RESOURCE_PANEL_H__

#include "Precompile.h"

#include "Editor/Panel/Panel.h"

#include "Resource/ObjectManager.h"


HS_NS_EDITOR_BEGIN

class ResourcePanel : public Panel
{
public:
	ResourcePanel(Window* window);
	~ResourcePanel() override;

	bool Setup() override;
	void Cleanup() override;

	void Draw() override;

private:


};

HS_NS_EDITOR_END

#endif