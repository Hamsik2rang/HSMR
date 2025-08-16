//
//  EngineContext.h
//  Engine
//
//  Created by Yongsik Im on 8/13/25.
//
#ifndef __HS_ENGINE_CONTEXT_H__
#define __HS_ENGINE_CONTEXT_H__

#include "Precompile.h"

#include "RHI/RHIDefinition.h"
//#include "RHI/RHIContext.h" 
namespace HS { class RHIContext; }



HS_NS_BEGIN

class HS_ENGINE_API EngineContext
{
public:
	EngineContext() = delete;
	EngineContext(const char* name, RHI::ERHIPlatform rhiPlatform);



private:
	
};

HS_NS_END

#endif