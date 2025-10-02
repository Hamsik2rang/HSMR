#include "Engine/EngineContext.h"

#include "HAL/SystemContext.h"

#include "RHI/RHIContext.h"

HS_NS_BEGIN

EngineContext::EngineContext(const char* name, ERHIPlatform platform)
	: name(name)
	, rhiContext(RHIContext::Create(platform))
{
	SystemContext::Get();
}

EngineContext::~EngineContext()
{



	SystemContext::Get()->Finalize();
}




HS_NS_END