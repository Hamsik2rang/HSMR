#pragma once
#ifndef __HS_ENGINE_CONTEXT_H__
#define __HS_ENGINE_CONTEXT_H__

#include "Precompile.h"

#include "Core/SystemContext.h"

HS_NS_BEGIN

struct HS_API EngineContext
{
	std::string executablePath = "";
	std::string executableDirectory = "";
	std::string assetDirectory = "";
};

EngineContext* GetEngineContext();
EngineContext* CreateEngineContext();

HS_NS_END


#endif