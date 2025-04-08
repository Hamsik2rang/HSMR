//
//  Image.h
//  Engine
//
//  Created by Yongsik Im on 3/22/25.
//

#ifndef __HS_IMAGE_H__
#define __HS_IMAGE_H__

#include "Precompile.h"

HS_NS_BEGIN

class Image
{
public:
    Image(void* data, size_t dataSize, uint32 width, uint32 height, uint8 channel = 4);
    Image(const Image& img);
    Image(Image&& img);

    
    uint8* GetData() { return _rawData; }
    size_t GetDataSize() const { return _rawDataSize; }
    uint32 GetWidth() const { return _width; }
    uint32 GetHeight() const { return _height;}
    uint8 GetChannel() const { return _channel; }
private:
    uint8* _rawData;
    size_t _rawDataSize;

    uint32 _width;
    uint32 _height;
    uint8  _channel;
};

HS_NS_END

#endif /* __HS_IMAGE_H__ */
