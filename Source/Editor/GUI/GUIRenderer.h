//
//  GUIRenderer.h
//  HSMR
//
//  Created by Yongsik Im on 8/4/25.
//
#ifndef __HS_GUI_RENDERER_H__
#define __HS_GUI_RENDERER_H__

#include "Precompile.h"

#include "Core/Renderer/Renderer.h"


HS_NS_EDITOR_BEGIN

class HS_EDITOR_API GUIRenderer final : public RenderPath
{
public:
	GUIRenderer(RHIContext* rhiContext)
		: RenderPath(rhiContext)
	{
	}

	~GUIRenderer() final = default;

	virtual void AddPass(RHIRenderPass* pass) final
	{
		return;
	}

	// Inherited via RenderPath
	RenderTargetInfo GetBareboneRenderTargetInfo() final;
};


HS_NS_EDITOR_END

#endif
