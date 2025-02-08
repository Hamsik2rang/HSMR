//
//  ResourceManager.h
//  Engine
//
//  Created by Yongsik Im on 2/7/25.
//
#ifndef __HS_RESOURCE_MANAGER_H__
#define __HS_RESOURCE_MANAGER_H__

#include "Precompile.h"

#include "Engine/Core/EngineContext.h"
#include "Engine/Core/FileSystem.h"

HS_NS_BEGIN

bool hs_resource_load_image(std::string& fileName, void* data, uint32& width, uint32& height, uint32& channel);
void hs_resource_free_image(void* data);




HS_NS_END

#endif /* GLTFLoader_h */
