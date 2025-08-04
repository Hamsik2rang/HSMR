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
    
    static const Image* GetFallbackImage2DWhite();
    static const Image* GetFallbackImage2DBlack();
    static const Image* GetFallbackImage2DRed();
    static const Image* GetFallbackImage2DGreen();
    static const Image* GetFallbackImage2DBlue();
    
    // TODO: PTR로변경
    static const Mesh& GetFallbackMeshPlane();
    static const Mesh& GetFallbackMeshCube();
    static const Mesh& GetFallbackMeshSphere();
    
private:
    static bool s_isInitialize;
    static std::string s_resourcePath;
    
    static void calculatePlane();
    static void calculateCube();
    static void calculateSphere();
    
    static Image* s_fallbackImage2DWhite;
    static Image* s_fallbackImage2DBlack;
    static Image* s_fallbackImage2DRed;
    static Image* s_fallbackImage2DGreen;
    static Image* s_fallbackImage2DBlue;
    
    static Mesh s_fallbackMeshPlane;
    static Mesh s_fallbackMeshCube;
    static Mesh s_fallbackMeshSphere;
    
};



HS_NS_END

#endif /* GLTFLoader_h */
