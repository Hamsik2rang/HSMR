//
//  Image.h
//  HSMR
//
//  Created by Yongsik Im on 2/5/25.
//
#ifndef __HS_IMAGE_H__
#define __HS_IMAGE_H__

#include "Precompile.h"

#include "Object/Object.h"

HS_NS_BEGIN

class HS_OBJECT_API Image : public Object
{
public:
    enum ImageType
    {
        DEFAULT,
        BUFFER,
    };
    
    explicit Image() noexcept
        : Object(Object::EType::IMAGE)
    {}
    Image(const char* path) noexcept;
    Image(void* data, uint32 width, uint32 height, uint32 channel) noexcept;
    Image(const Image& o) noexcept;
    Image(Image&& o) noexcept;
    
    Image& operator=(const Image& o);
    Image& operator=(Image&& o);

    ~Image() override;

    HS_FORCEINLINE uint8* GetRawData() const { return _rawData; }
    HS_FORCEINLINE size_t GetRawDataSize() const { return _rawDataSize; }
    HS_FORCEINLINE uint16 GetWidth() const { return _width; }
    HS_FORCEINLINE uint16 GetHeight() const { return _height; }
    HS_FORCEINLINE uint8  GetChannel() const { return _channel; }
    HS_FORCEINLINE ImageType GetType() const { return _type; }
    HS_FORCEINLINE void SetType(ImageType type) { _type = type; }

private:
    uint8* _rawData;
    size_t _rawDataSize;

    ImageType _type;
    uint16 _width;
    uint16 _height;
    uint8  _channel;
};

HS_NS_END

#endif
