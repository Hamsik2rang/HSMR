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

class HS_EDITOR_API GUIRenderer final : public Renderer
{
public:
	GUIRenderer(RHIContext* rhiContext)
		: Renderer(rhiContext)
	{
	}

	~GUIRenderer() final = default;

	virtual void AddPass(RenderPass* pass) final
	{
		return;
	}

	// Inherited via Renderer
	RenderTargetInfo GetBareboneRenderTargetInfo() final;
};


HS_NS_EDITOR_END

#endif
