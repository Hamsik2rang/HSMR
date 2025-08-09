//
//  Geometry.h
//  Engine
//
//  Created by Yongsik Im on 2/8/25.
//
#ifndef __HS_GEOMETRY_H__
#define __HS_GEOMETRY_H__

struct Resolution
{
    uint32 width  = 1;
    uint32 height = 1;

    Resolution()
        : width(1)
        , height(1)
    {}

    Resolution(uint32 width, uint32 height)
        : width(width)
        , height(height)
    {}

    bool operator==(const Resolution& rhs) const
    {
        return width == rhs.width && height == rhs.height;
    }

    bool operator!=(const Resolution& rhs) const
    {
        return (*this == rhs);
    }
};

struct Rect
{
    float x;
    float y;
    float width;
    float height;

    Rect(float x, float y, float width, float height) noexcept
        : x(x)
        , y(y)
        , width(width)
        , height(height)
    {}

    bool operator==(const Rect& rhs) const
    {
        return ((x == rhs.x) && (y == rhs.y) && (width == rhs.width) && (height == rhs.height));
    }
    
    bool operator!=(const Rect& rhs) const
    {
        return (*this == rhs);
    }
};

#endif /* __HS_GEOMETRY_H__ */
