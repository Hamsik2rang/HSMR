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
    , _width(width)
    , _height(height)
    , _channel(channel)
{
    size_t size = width * height * channel;
    _rawData.resize(size);
    ::memcpy(_rawData.data(), data, size);
}

Image::Image(const Image& o) noexcept
    : Object(EType::IMAGE)
    , _rawData(o._rawData)  // std::vector copy constructor handles memory safely
    , _width(o._width)
    , _height(o._height)
    , _channel(o._channel)
    , _type(o._type)
{
    // std::vector automatically handles memory allocation and copying
}

Image::Image(Image&& o) noexcept
    : Object(EType::IMAGE)
    , _rawData(std::move(o._rawData))  // std::vector move constructor
    , _width(o._width)
    , _height(o._height)
    , _channel(o._channel)
    , _type(o._type)
{
    // Reset moved-from object
    o._width = 0;
    o._height = 0;
    o._channel = 0;
    // std::vector automatically cleared by move
}

Image::~Image()
{
    // std::vector automatically cleans up memory - no manual delete needed
}

Image& Image::operator=(const Image& o)
{
    if (this != &o)
    {
        _rawData = o._rawData;  // std::vector assignment handles memory automatically
        _type    = o._type;
        _width   = o._width;
        _height  = o._height;
        _channel = o._channel;
    }

    return *this;
}

Image& Image::operator=(Image&& o)
{
    if (this != &o)
    {
        _rawData = std::move(o._rawData);  // std::vector move assignment
        _type    = o._type;
        _width   = o._width;
        _height  = o._height;
        _channel = o._channel;
        
        // Reset moved-from object
        o._width = 0;
        o._height = 0;
        o._channel = 0;
    }

    return *this;
}

HS_NS_END
