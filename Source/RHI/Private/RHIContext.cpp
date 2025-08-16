#include "RHI/RHIContext.h"

#ifdef __WINDOWS__
#include "RHI/Vulkan/VulkanContext.h"
#else
#include "RHI/Metal/MetalContext.h"
#endif

HS_NS_BEGIN

RHIContext* g_pRHIContext = nullptr;

RHIContext* RHIContext::Create(ERHIPlatform platform)
{
	if (g_pRHIContext)
	{
		return g_pRHIContext;
	}

	switch (platform)
	{
	case ERHIPlatform::VULKAN:
	{
		g_pRHIContext = new VulkanContext();
	}
	break;
#ifdef __APPLE__
	case ERHIPlatform::METAL:
	{
		g_pRHIContext = new MetalContext();
	}
	break;
#endif
	default:
		HS_LOG(crash, "Unsupported RHI");
	}

	g_pRHIContext->Initialize();

	return g_pRHIContext;
}

RHIContext* RHIContext::Get()
{
	if (!g_pRHIContext)
	{
		HS_LOG(error, "RHIContext is not created");
	}

	return g_pRHIContext;
}

HS_NS_END
