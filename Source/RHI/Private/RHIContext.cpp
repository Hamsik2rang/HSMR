#include "RHI/RHIContext.h"

#ifdef __WINDOWS__
#include "RHI/Vulkan/VulkanContext.h"
#else
#include "RHI/Metal/MetalContext.h"
#endif

HS_NS_BEGIN

HS_API RHIContext* g_rhiContext = nullptr;

RHIContext* RHIContext::Create(ERHIPlatform platform)
{
	if (g_rhiContext)
	{
		return g_rhiContext;
	}

	switch (platform)
	{
#if defined(__WINDOWS__)
	case ERHIPlatform::VULKAN:
	{
		g_rhiContext = new VulkanContext();
	}
	break;
#elif defined(__APPLE__)
	case ERHIPlatform::METAL:
	{
		g_rhiContext = new MetalContext();
	}
	break;
#endif
	default:
		HS_LOG(crash, "Unsupported RHI");
	}

	g_rhiContext->Initialize();

	return g_rhiContext;
}

RHIContext* RHIContext::Get()
{
	if (!g_rhiContext)
	{
		HS_LOG(error, "RHIContext is not created");
	}

	return g_rhiContext;
}

HS_NS_END
