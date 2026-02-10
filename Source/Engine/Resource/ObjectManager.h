//
//  ObjectManager.h
//  Object
//
//  Created by Yongsik Im on 2/7/25.
//
#ifndef __HS_OBJECT_MANAGER_H__
#define __HS_OBJECT_MANAGER_H__

#include "Precompile.h"

#include "Core/SystemContext.h"
#include "Core/HAL/FileSystem.h"

#include "RHI/RHIDefinition.h"

#include "Engine/Resource/Image.h"
#include "Engine/Resource/Mesh.h"
#include "Engine/Resource/Shader.h"
#include "Engine/Resource/Material.h"
#include "Engine/Resource/Model.h"

HS_NS_BEGIN

class HS_API ObjectManager
{
public:
	static bool Initialize();
	static void Finalize();

	static Scoped<Image> LoadImageFromFile(const std::string& path, bool isAbsolutePath = false);
	static void FreeImage(Image* image);

	static Scoped<Mesh> LoadMeshFromFile(const std::string& path, bool isAbsolutePath = false);
	static void FreeMesh(Mesh* mesh);

	static Scoped<Shader> LoadShaderFromFile(const std::string& path, EShaderStage stage, const char* entryPointName, bool isAbsolutePath = false);
	static Scoped<Shader> LoadShaderFromSource(const std::string& shaderName, const std::string& sourceCode,
	                                           EShaderStage requestedStages = EShaderStage::VERTEX | EShaderStage::FRAGMENT,
	                                           const std::vector<std::string>& includePaths = {});
	static void FreeShader(Shader* shader);

	static Scoped<Material> LoadMaterialFromFile(const std::string& path, bool isAbsolutePath = false);
	static void FreeMaterial(Material* material);

	// Load model with meshes and materials
	static bool LoadModel(const std::string& path,
	                      std::vector<Scoped<Mesh>>& outMeshes,
	                      std::vector<Scoped<Material>>& outMaterials,
	                      bool isAbsolutePath = false);
    static bool LoadModel(const std::string& path, Scoped<Model>& outModel, bool isAbsolutePath = false);

	static const Image* GetFallbackImage2DWhite();
	static const Image* GetFallbackImage2DBlack();
	static const Image* GetFallbackImage2DRed();
	static const Image* GetFallbackImage2DGreen();
	static const Image* GetFallbackImage2DBlue();

	// TODO: PTR로변경
	static const Mesh* GetFallbackMeshPlane();
	static const Mesh* GetFallbackMeshCube();
	static const Mesh* GetFallbackMeshSphere();

private:
	static bool s_isInitialize;
	static std::string s_resourcePath;
    

	// Load GLTF/GLB model with PBR materials
	static bool loadGLTF(const std::string& path,
	                     std::vector<Scoped<Mesh>>& outMeshes,
	                     std::vector<Scoped<Material>>& outMaterials,
	                     bool isAbsolutePath = false);
	static bool loadGLTF(const std::string& path, Scoped<Model>& outModel, bool isAbsolutePath = false);

	static void calculatePlane();
	static void calculateCube();
	static void calculateSphere();

	static Scoped<Image> s_fallbackImage2DWhite;
	static Scoped<Image> s_fallbackImage2DBlack;
	static Scoped<Image> s_fallbackImage2DRed;
	static Scoped<Image> s_fallbackImage2DGreen;
	static Scoped<Image> s_fallbackImage2DBlue;

	static Scoped<Mesh> s_fallbackMeshPlane;
	static Scoped<Mesh> s_fallbackMeshCube;
	static Scoped<Mesh> s_fallbackMeshSphere;

};



HS_NS_END

#endif /* GLTFLoader_h */
