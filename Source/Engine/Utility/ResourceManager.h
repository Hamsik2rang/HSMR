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

#include "Engine/Object/Image.h"
#include "Engine/Object/Mesh.h"

HS_NS_BEGIN

class ResourceManager
{
public:
    static bool Initialize();
    static void Finalize();
    
    static Scoped<Image> LoadImageFromFile(const std::string& path, bool isAbsolutePath = false);
    static void FreeImage(Image* image);
    static Scoped<Mesh> LoadMeshFromFile(const std::string& path, bool isAbsolutePath = false);
    static void FreeMesh(Mesh* mesh);
    
    
private:
    static std::string _resourcePath;
    
    
};



HS_NS_END

#endif /* GLTFLoader_h */
