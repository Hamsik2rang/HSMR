//
//  Object.cpp
//  HSMR
//
//  Created by Yongsik Im on 2/4/25.
//
#include "Object/Object.h"
#include <atomic>

HS_NS_BEGIN

uint64 Object::GenerateObjectId()
{
    static std::atomic<uint64> s_nextObjectId{1};
    return s_nextObjectId.fetch_add(1, std::memory_order_relaxed);
}

HS_NS_END