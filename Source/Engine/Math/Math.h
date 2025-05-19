//
//  Math.h
//  Engine
//
//  Created by Yongsik Im on 2025.5.16
//
#ifndef __HS_MATH_H__
#define __HS_MATH_H__

#include "Precompile.h"

#include "Engine/Math/Vec4f.h"
#include "Engine/Math/Vec3f.h"
#include "Engine/Math/Mat4f.h"

#ifdef __cplusplus
    #include <cmath>
#else
    #include <math.h>
#endif

HS_NS_BEGIN

bool EpsilonEqual(float lhs, float rhs)
{
    FLT_EPSILON;
}

HS_NS_END

#endif /*__HS_MATH_H__*/
