//
//  Flag.h
//  Engine
//
//  Created by Yongsik Im on 2025.3.22
//

#ifndef __HS_FLAG_H__
#define __HS_FLAG_H__

#include "Precompile.h"

HS_NS_BEGIN

template <typename En, typename std::enable_if<std::is_enum<En>::value>::type* = nullptr>
class Flag
{
    
};

template <typename En>
class DirtyFlag : public Flag<En>
{
    
};

HS_NS_END

#endif /* Flag_h */
