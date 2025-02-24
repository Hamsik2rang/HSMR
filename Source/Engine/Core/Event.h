//
//  Event.h
//  Engine
//
//  Created by Yongsik Im on 2/8/25.
//

#ifndef __HS_EVENT_H__
#define __HS_EVENT_H__
//...

#include "Precompile.h"

struct Event
{
    union
    {
        uint64 type;
        uint8 padding[128];
    };
};


#endif /* __HS_EVENT_H__ */
