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
// #include "RHI/RHIContext.h"
namespace HS
{
class RHIContext;
}

HS_NS_BEGIN

struct HS_ENGINE_API EngineContext
{
public:
    EngineContext() = delete;
    EngineContext(const char* name);
    ~EngineContext();

    const char* const name;

    std::string executablePath      = "";
    std::string executableDirectory = "";
    std::string assetDirectory      = "";

private:
    void intializePLatform(EngineContext* engineContext);
};

HS_NS_END

#endif