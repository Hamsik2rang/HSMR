//
//  Flag.h
//  Core
//
//  Created by Yongsik Im on 3/22/2025
//

#ifndef __HS_FLAG_H__
#define __HS_FLAG_H__

#include "Precompile.h"

HS_NS_BEGIN

template <typename En, typename std::enable_if<std::is_enum<En>::value>::type* = nullptr>
class HS_API Flag
{
    
};

template <typename En>
class HS_API DirtyFlag : public Flag<En>
{
    
};

HS_NS_END

#endif /* Flag_h */
