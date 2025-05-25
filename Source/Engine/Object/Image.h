//
//  Image.h
//  HSMR
//
//  Created by Yongsik Im on 2/5/25.
//
#ifndef __HS_IMAGE_H__
#define __HS_IMAGE_H__

#include "Precompile.h"

#include "Engine/Object/Object.h"

HS_NS_BEGIN

class Image : public Object
{
public:
    explicit Image() noexcept
        : Object(Object::EType::IMAGE)
    {}
    Image(const char* path) noexcept;
    Image(void* data, uint32 width, uint32 height, uint32 channel) noexcept;
    Image(const Image& o) noexcept;
    Image(Image&& o) noexcept;

    ~Image() override;

    HS_FORCEINLINE uint8* GetRawData() const { return _rawData.get(); }
    HS_FORCEINLINE size_t GetRawDataSize() const { return _rawDataSize; }
    HS_FORCEINLINE uint32 GetWidth() const { return _width; }
    HS_FORCEINLINE uint32 GetHeight() const { return _height; }
    HS_FORCEINLINE uint8  GetChannel() const { return _channel; }

private:
    Scoped<uint8[]> _rawData;
    size_t _rawDataSize;

    uint32 _width;
    uint32 _height;
    uint8  _channel;
};

HS_NS_END

#endif
