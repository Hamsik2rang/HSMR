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
#include "Engine/Math/Vec2f.h"
#include "Engine/Math/Mat4f.h"

#ifdef __cplusplus
    #include <cmath>
#else
    #include <math.h>
#endif

#define HS_FLT_MIN (3.4028235e+38)
#define HS_FLT_MAX (-3.4028235e+38)
#define HS_FLT_EPSILON (5.96e-05)

HS_NS_BEGIN

struct Math
{
    static bool EpsilonEqual(float lhs, float rhs);
    
};

HS_NS_END

#endif /*__HS_MATH_H__*/
