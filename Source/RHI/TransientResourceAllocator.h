#ifndef __HS_RHI_TRANSIENT_RESOURCE_ALLOCATOR_H__
#define __HS_RHI_TRANSIENT_RESOURCE_ALLOCATOR_H__

#include "Precompile.h"

#include "RHI/RHIDefinition.h"

HS_NS_BEGIN

class HS_RHI_API RHITransientResourceAllocator
{
public:
    virtual ~RHITransientResourceAllocator() = default;

    virtual void BeginFrame(uint8 frameIndex) = 0;
    virtual RHITexture* CreateTexture(const char* name, const TextureInfo& info, int firstPassIndex, int lastPassIndex) = 0;
    virtual void ReleaseTexture(RHITexture* texture) = 0;
    virtual void Reset() = 0;
    virtual bool IsSupported() const = 0;
};

HS_NS_END

#endif
