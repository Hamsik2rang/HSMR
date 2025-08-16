//
//  Image.cpp
//  HSMR
//
//  Created by Yongsik Im on 2/5/25.
//
#include "Object/Image.h"

#include "Object/ObjectManager.h"

HS_NS_BEGIN

Image::Image(const char* path) noexcept
    : Object(EType::IMAGE)
{
    Scoped<Image> image = ObjectManager::LoadImageFromFile(path);
    Image* pImage       = image.get();
    _rawData            = std::move(pImage->_rawData);
    _width              = pImage->_width;
    _height             = pImage->_height;
    _channel            = pImage->_channel;
}

Image::Image(void* data, uint32 width, uint32 height, uint32 channel) noexcept
    : Object(EType::IMAGE)
    , _rawData(nullptr)
    , _width(width)
    , _height(height)
    , _channel(channel)
    , _rawDataSize(width * height * channel)
{
    _rawData = new uint8[_rawDataSize];
    ::memcpy(_rawData, data, _rawDataSize);
}

Image::Image(const Image& o) noexcept
    : Object(EType::IMAGE)
    , _rawData(nullptr)
    , _rawDataSize(o._rawDataSize)
    , _width(o._width)
    , _height(o._height)
    , _channel(o._channel)
    , _type(o._type)

{
    _rawData = new uint8[o._rawDataSize];
    ::memcpy(_rawData, o._rawData, o._rawDataSize);
}

Image::Image(Image&& o) noexcept
    : Object(EType::IMAGE)
    , _rawData(std::move(o._rawData))
    , _rawDataSize(o._rawDataSize)
    , _width(o._width)
    , _height(o._height)
    , _channel(o._channel)
    , _type(o._type)
{
    o._rawData = nullptr;
    o._rawDataSize = 0;
    o._width = 0;
    o._height = 0;
    o._channel = 0;
}

Image::~Image()
{
    if (_rawData)
    {
        delete[] _rawData;
        _rawData = nullptr;
    }
}

Image& Image::operator=(const Image& o)
{
    if (this != &o)
    {
        if (_rawData != nullptr)
        {
            delete[] _rawData;
        }

        _rawData = new uint8[o._rawDataSize];
        ::memcpy(_rawData, o._rawData, o._rawDataSize);
        _rawDataSize = o._rawDataSize;
        _type        = o._type;
        _width       = o._width;
        _height      = o._height;
        _channel     = o._channel;
    }

    return *this;
}

Image& Image::operator=(Image&& o)
{
    if (this != &o)
    {
        _rawData     = std::move(o._rawData);
        _rawDataSize = o._rawDataSize;
        _type        = o._type;
        _width       = o._width;
        _height      = o._height;
        _channel     = o._channel;
    }

    return *this;
}

HS_NS_END
