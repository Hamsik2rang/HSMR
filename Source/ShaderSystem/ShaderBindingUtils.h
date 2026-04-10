#ifndef __HS_SHADER_BINDING_UTILS_H__
#define __HS_SHADER_BINDING_UTILS_H__

#include "Precompile.h"

#include <string>

HS_NS_BEGIN

inline bool IsReservedGlobalBufferName(const std::string& name)
{
    return name == "perView" || name == "PerView" ||
           name == "perDraw" || name == "PerDraw" ||
           name == "perFrame" || name == "PerFrame" ||
           name == "lightUBO" || name == "LightUBO";
}

HS_NS_END

#endif
