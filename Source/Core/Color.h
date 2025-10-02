//
//  Color.h
//  Core
//
//  Created by Yongsik Im on 2/8/2025
//
#ifndef __HS_COLOR_H__
#define __HS_COLOR_H__

#include "Precompile.h"

#include "Core/Log.h"
#include "ThirdParty/glm/glm.hpp"
// #include <simd/simd.h>

class Color
{
public:
    Color() : _data{0.0f} {}
    Color(glm::vec4 v) : _data{v} {}
    
    Color(const Color& o) : _data{o._data} {}
    Color(Color&& o) : _data(o._data) {};
    
    
    float& R() noexcept { return _data.r; }
    float& G() noexcept { return _data.g; }
    float& B() noexcept { return _data.b; }
    float& A() noexcept { return _data.a; }
    
    HS_FORCEINLINE float operator[](int index) { HS_ASSERT(index >= 0 && index < 4, "out of range"); return _data[index];}
    HS_FORCEINLINE operator glm::vec4() const { return _data; }
private:
    glm::vec4 _data;
};

#endif /* __HS_COLOR_H__ */
