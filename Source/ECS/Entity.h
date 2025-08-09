//
//  Entity.h
//  Engine
//
//  Created by Yongsik Im on 3/22/25.
//

#ifndef __HS_ENTITY_H__
#define __HS_ENTITY_H__

#include "Precompile.h"

class Entity
{
public:
    
    
    HS_FORCEINLINE uint32 ID() { return _id; }
    
    
private:
    uint32 _id;
};



#endif /* __HS_ENTITY_H__ */
