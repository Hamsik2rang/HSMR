//
//  Image.h
//  HSMR
//
//  Created by Yongsik Im on 2/5/25.
//
#ifndef __HS_IMAGE_H__
#define __HS_IMAGE_H__

#include "Precompile.h"

#include "Core/Object.h"

HS_NS_BEGIN

class HS_RESOURCE_API Image : public Object
{
public:
    enum ImageType
    {
        Default,
        Buffer,
    };

    explicit Image() noexcept
        : Object(Object::EType::Image)
    {}
    Image(const char* path) noexcept;
    Image(void* data, uint32 width, uint32 height, uint32 channel) noexcept;
    Image(std::vector<uint8>&& rawData, uint16 width, uint16 height, uint8 channel) noexcept;
    Image(const Image& o) noexcept;
    Image(Image&& o) noexcept;

    Image& operator=(const Image& o);
    Image& operator=(Image&& o);

    ~Image() override;

    HS_FORCEINLINE uint8* GetRawData() const { return const_cast<uint8*>(_rawData.data()); }
    HS_FORCEINLINE const std::vector<uint8>& GetRawDataVector() const { return _rawData; }
    HS_FORCEINLINE size_t GetRawDataSize() const { return _rawData.size(); }

    // Drop CPU-side pixel data after the GPU texture is uploaded. The width/
    // height/channel metadata is kept so the image is still self-describing.
    void ReleaseRawData()
    {
        std::vector<uint8>().swap(_rawData);
    }
    bool HasRawData() const { return !_rawData.empty(); }
    HS_FORCEINLINE uint16 GetWidth() const { return _width; }
    HS_FORCEINLINE uint16 GetHeight() const { return _height; }
    HS_FORCEINLINE uint8  GetChannel() const { return _channel; }
    HS_FORCEINLINE ImageType GetType() const { return _type; }
    HS_FORCEINLINE void SetType(ImageType type) { _type = type; }
    HS_FORCEINLINE bool IsSrgb() const { return _isSrgb; }
    HS_FORCEINLINE void SetSrgb(bool isSrgb) { _isSrgb = isSrgb; }
    void SetDisplayName(const std::string& name)
    {
        _nameStorage = name;
        this->name = _nameStorage.c_str();
    }
    const std::string& GetDisplayName() const { return _nameStorage; }
    void SetSourceAssetPath(const std::string& path) { _sourceAssetPath = path; }
    const std::string& GetSourceAssetPath() const { return _sourceAssetPath; }
    bool HasSourceAssetPath() const { return !_sourceAssetPath.empty(); }

private:
    std::string _nameStorage;
    std::string _sourceAssetPath;
    std::vector<uint8> _rawData;

    ImageType _type = ImageType::Default;
    uint16 _width   = 0;
    uint16 _height  = 0;
    uint8  _channel = 0;
    bool   _isSrgb  = false;
};

HS_NS_END

#endif
