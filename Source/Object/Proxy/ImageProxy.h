#ifndef __HS_IMAGE_PROXY_H__
#define __HS_IMAGE_PROXY_H__

#include "Precompile.h"
#include "Object/ObjectProxy.h"
#include "RHI/RHIDefinition.h"

HS_NS_BEGIN

class Image;
class RHITexture;
class RHISampler;

class HS_OBJECT_API ImageProxy : public ObjectProxy
{
public:
    explicit ImageProxy(uint64 gameObjectId);
    ~ImageProxy() override;

    void UpdateFromGameObject(const Object* gameObject) override;
    void ReleaseRenderResources() override;

    RHITexture* GetRHITexture() const { return _rhiTexture; }
    RHISampler* GetRHISampler() const { return _rhiSampler; }

    uint32 GetWidth() const { return _width; }
    uint32 GetHeight() const { return _height; }
    uint32 GetChannels() const { return _channels; }
    EPixelFormat GetFormat() const { return _format; }

    bool HasValidTexture() const { return _rhiTexture != nullptr; }

private:
    void CreateRHIResources(const Image* image);
    void UpdateTextureData(const Image* image);

    RHITexture* _rhiTexture;
    RHISampler* _rhiSampler;
    
    uint32 _width;
    uint32 _height;
    uint32 _channels;
    EPixelFormat _format;
    
    uint64 _lastUpdateHash;
};

HS_NS_END

#endif