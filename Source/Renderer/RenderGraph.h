#ifndef __HS_RENDER_GRAPH_H__
#define __HS_RENDER_GRAPH_H__

#include "Precompile.h"

HS_NS_BEGIN

class RenderGraphBuilder
{
public:
	enum class EBufferAccess
	{

	};

	enum class ETextureAccess
	{
        ShaderRead = 0,
        ColorAttachmentWrite,
        ReadWrite, // ← General 레이아웃, UAV에 해당
        DepthAttachmentRead,
        DepthAttachmentWrite,
        DepthStencilAttachmentRead,
        DepthStencilAttachmentWrite,
        TransferRead,
        TransferWrite,
        ComputeShaderRead,
        ComputeShaderWrite, // ← Compute UAV Write에 해당
        FragmentShaderReadSampledImageOrUniformTexelBuffer,
        Present
	};


private:

};

HS_NS_END

#endif