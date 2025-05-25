//
//  Image.cpp
//  HSMR
//
//  Created by Yongsik Im on 2/5/25.
//
#include "Engine/Object/Image.h"

#include "Engine/Utility/ResourceManager.h"

HS_NS_BEGIN

Image::Image(const char* path) noexcept
    : Object(EType::IMAGE)
{
    Scoped<Image> image  = ResourceManager::LoadImageFromFile(path);
    Image*        pImage = image.get();
    _rawData             = std::move(pImage->_rawData);
    _width               = pImage->_width;
    _height              = pImage->_height;
    _channel             = pImage->_channel;
}

Image::Image(void* data, uint32 width, uint32 height, uint32 channel) noexcept
    : Object(EType::IMAGE)
    , _rawData(static_cast<uint8*>(data))
    , _width(width)
    , _height(height)
    , _channel(channel)
{
}

Image::Image(const Image& image) noexcept
    : Object(EType::IMAGE)
{
}
Image::Image(Image&& image) noexcept
    : Object(EType::IMAGE)
{
}

Image::~Image()
{
}

HS_NS_END
