//
//  ProfilerPanel.h
//  Editor
//
//  Created by Yongsik Im on 10/06/25.
//
#ifndef __HS_PROFILER_PANEL_H__
#define __HS_PROFILER_PANEL_H_H

#include "Precompile.h"

#include "Editor/Panel/Panel.h"

#include "Core/HAL/Timer.h"

HS_NS_EDITOR_BEGIN

class ProfilerPanel : public Panel
{
public:
	ProfilerPanel(Window* window);
	~ProfilerPanel() override;

	bool Setup() override;
	void Cleanup() override;

	void Draw() override;


private:
	double _lastFrameTime = 0.0;

};

HS_NS_EDITOR_END

#endif