//
//  Scene.h
//  HSMR
//
//  Created by Yongsik Im on 2/2/25.
//
#ifndef __HS_SCENE_H__
#define __HS_SCENE_H__

#include "Precompile.h"

#include <vector>

HS_NS_BEGIN

class Scene
{
public:
    
    void Update();
    
private:
    std::vector<Scene*> _next;
    
};


HS_NS_END


#endif
