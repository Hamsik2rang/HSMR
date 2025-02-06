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
    Image() : Object(Object::EType::IMAGE) {}
    Image(const char* path);
    Image(void* data, uint32 width, uint32 height, uint32 channel);
    Image(Image& image);
    Image(Image&& image);
    
    ~Image() override;
    
private:
    void* _rawData;
    uint32 _rawDataSize;
    uint32 _width;
    uint32 _height;
    uint32 _channel;
    
    
};



HS_NS_END


#endif

