#include "Object/ImageProxy.h"
#include "Object/Image.h"
#include "Core/Log.h"
#include "RHI/RHIContext.h"

HS_NS_BEGIN

ImageProxy::ImageProxy(uint64 gameObjectId)
    : ObjectProxy(EType::IMAGE, gameObjectId)
    , _rhiTexture(nullptr)
    , _rhiSampler(nullptr)
    , _width(0)
    , _height(0)
    , _channels(0)
    , _format(EPixelFormat::R8G8B8A8_UNORM)
    , _lastUpdateHash(0)
{
}

ImageProxy::~ImageProxy()
{
    ReleaseRenderResources();
}

void ImageProxy::UpdateFromGameObject(const Object* gameObject)
{
    const Image* image = static_cast<const Image*>(gameObject);
    if (!image || !image->IsValid())
    {
        return;
    }

    uint64 currentHash = reinterpret_cast<uint64>(image->GetRawData()) + 
                        static_cast<uint64>(image->GetWidth()) + 
                        static_cast<uint64>(image->GetHeight()) + 
                        static_cast<uint64>(image->GetChannel());

    if (currentHash != _lastUpdateHash || _isDirty)
    {
        CreateRHIResources(image);
        _lastUpdateHash = currentHash;
        MarkClean();
    }
}

void ImageProxy::ReleaseRenderResources()
{
    if (_rhiTexture)
    {
        // TODO: Release RHI texture when RHI is implemented
        _rhiTexture = nullptr;
    }

    if (_rhiSampler)
    {
        // TODO: Release RHI sampler when RHI is implemented  
        _rhiSampler = nullptr;
    }
}

void ImageProxy::CreateRHIResources(const Image* image)
{
    if (!image || !image->GetRawData())
    {
        HS_LOG(warning, "ImageProxy: Cannot create RHI resources from invalid image");
        return;
    }

    ReleaseRenderResources();

    _width = image->GetWidth();
    _height = image->GetHeight();
    _channels = image->GetChannel();

    switch (_channels)
    {
    case 1:
        _format = EPixelFormat::R8_UNORM;
        break;
    case 2:
        _format = EPixelFormat::RG8_UNORM;
        break;
    case 3:
        _format = EPixelFormat::R8G8B8A8_UNORM;
        break;
    case 4:
    default:
        _format = EPixelFormat::R8G8B8A8_UNORM;
        break;
    }

    // TODO: Create RHI texture and sampler when RHI system is available
    // RHITextureDesc textureDesc;
    // textureDesc.width = _width;
    // textureDesc.height = _height;
    // textureDesc.format = _format;
    // textureDesc.usage = ERHITextureUsage::ShaderResource;
    // textureDesc.initialData = image->GetRawData();
    // 
    // _rhiTexture = RHIContext::CreateTexture(textureDesc);
    // 
    // RHISamplerDesc samplerDesc;
    // samplerDesc.filter = ERHISamplerFilter::Linear;
    // samplerDesc.addressU = ERHISamplerAddressMode::Wrap;
    // samplerDesc.addressV = ERHISamplerAddressMode::Wrap;
    // 
    // _rhiSampler = RHIContext::CreateSampler(samplerDesc);

    HS_LOG(info, "ImageProxy: Created RHI resources for %ux%u image with %u channels", 
           _width, _height, _channels);
}

void ImageProxy::UpdateTextureData(const Image* image)
{
    if (!_rhiTexture || !image || !image->GetRawData())
    {
        return;
    }

    // TODO: Update texture data when RHI system is available
    // RHIContext::UpdateTextureData(_rhiTexture, image->GetRawData(), image->GetRawDataSize());
}

HS_NS_END