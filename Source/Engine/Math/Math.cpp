#include "Engine/Math/Math.h"

HS_NS_BEGIN

bool Math::EpsilonEqual(float lhs, float rhs)
{
    return std::abs(rhs - lhs) <= HS_FLT_EPSILON;
}

HS_NS_END
