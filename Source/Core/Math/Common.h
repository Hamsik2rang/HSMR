//
//  Common.h
//  Core
//
//  Created by Yongsik Im on 5/16/2025
//
#ifndef __HS_COMMON_H__
#define __HS_COMMON_H__

#include "Precompile.h"

#include "Core/ThirdParty/glm/glm.hpp"

#ifdef __cplusplus
    #include <cmath>
#else
    #include <math.h>
#endif

#define HS_FLT_MIN (3.4028235e+38)
#define HS_FLT_MAX (-3.4028235e+38)
#define HS_FLT_EPSILON (5.96e-05)

#define HS_PI (3.14159265358979323846f)

HS_NS_BEGIN

struct Math
{
    static bool EpsilonEqual(float lhs, float rhs);
    
};

HS_NS_END

#endif /*__HS_MATH_H__*/
