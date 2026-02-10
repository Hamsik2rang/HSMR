# ImageResource 구현 및 텍스처 렌더링 파이프라인 완성 계획

## 목표
GLTF에서 로드된 텍스처를 실제로 GPU에서 렌더링할 수 있도록 텍스처 바인딩 파이프라인을 완성합니다.

## 완료된 작업

### Proxy 레거시 시스템 제거 (완료)
다음 파일들이 삭제되었습니다:
- `Source/Engine/Resource/Proxy/ObjectProxy.h`
- `Source/Engine/Resource/Proxy/ImageProxy.h`
- `Source/Engine/Resource/Proxy/MeshProxy.h`
- `Source/Engine/Resource/Proxy/MaterialProxy.h`
- `Source/Engine/Resource/Proxy/ShaderProxy.h`
- `Source/Engine/Resource/Proxy/Private/ImageProxy.cpp`
- `Source/Engine/Resource/Proxy/Private/MeshProxy.cpp`
- `Source/Engine/Resource/Proxy/Private/MaterialProxy.cpp`
- `Source/Engine/Resource/Proxy/Private/ShaderProxy.cpp`

> 참고: 이 파일들은 CMakeLists.txt에 포함되어 있지 않아 빌드에 영향 없음

## 현황 분석

### 현재 상태
- **정상 작동**: GLTF 로드 → Image 객체 생성 → Material에 저장
- **정상 작동**: ShaderCompiler가 `textureBindings`, `samplerBindings` 리플렉션 정보 생성
- **정상 작동**: ResourceBinding 구조체가 textures, samplers 필드 지원
- **누락**: Image → RHITexture 변환 없음
- **누락**: createResourceLayoutFromReflection()에서 텍스처 바인딩 처리 없음
- **삭제 완료**: 미사용 Proxy 시스템 (ImageProxy, MeshProxy 등)

### 기존 패턴
```
Mesh     → MeshResource     { RHIBuffer* VB/IB }
Material → MaterialResource { RHIShader*, RHIResourceLayout*, RHIResourceSet* }
Camera   → CameraResource   { RHIBuffer* perView }
Model    → ModelResource    { RHIBuffer* perDraw }
Image    → ??? (없음)       → ImageResource 추가 필요
```

---

## 구현 단계

### 1단계: ImageResource 구조체 정의
**파일**: `Source/Engine/Renderer/RenderResourceManager.h`

```cpp
// Image에 대한 GPU 리소스 캐시
struct HS_API ImageResource
{
    RHITexture* texture = nullptr;
    RHISampler* sampler = nullptr;
    uint32 width = 0;
    uint32 height = 0;
    EPixelFormat format = EPixelFormat::R8G8B8A8_UNORM;
    bool isValid = false;
};
```

---

### 2단계: RenderResourceManager 확장
**파일**: `Source/Engine/Renderer/RenderResourceManager.h`, `.cpp`

**헤더 추가**:
```cpp
class Image;  // forward declaration

// public 메서드
ImageResource* GetOrCreateImageResource(Image* image);

// private 멤버
std::unordered_map<Image*, ImageResource> _imageResources;

// private 헬퍼
ImageResource createImageResource(Image* image);
```

**구현** (`RenderResourceManager.cpp`):
```cpp
ImageResource* RenderResourceManager::GetOrCreateImageResource(Image* image)
{
    if (!image) return nullptr;

    auto it = _imageResources.find(image);
    if (it != _imageResources.end() && it->second.isValid)
    {
        return &it->second;
    }

    ImageResource resource = createImageResource(image);
    if (resource.isValid)
    {
        _imageResources[image] = std::move(resource);
        return &_imageResources[image];
    }
    return nullptr;
}

ImageResource RenderResourceManager::createImageResource(Image* image)
{
    ImageResource resource;

    uint32 width = image->GetWidth();
    uint32 height = image->GetHeight();
    uint8 channels = image->GetChannel();

    // 포맷 결정
    EPixelFormat format = EPixelFormat::R8G8B8A8_UNORM;
    if (channels == 1) format = EPixelFormat::R8_UNORM;
    else if (channels == 2) format = EPixelFormat::RG8_UNORM;
    // 3채널도 RGBA로 처리 (RGB8은 Vulkan에서 비효율적)

    // 3채널 이미지는 4채널로 확장 필요
    const void* imageData = image->GetRawData();
    std::vector<uint8> rgbaData;
    if (channels == 3)
    {
        rgbaData.resize(width * height * 4);
        const uint8* src = static_cast<const uint8*>(imageData);
        for (uint32 i = 0; i < width * height; ++i)
        {
            rgbaData[i * 4 + 0] = src[i * 3 + 0];
            rgbaData[i * 4 + 1] = src[i * 3 + 1];
            rgbaData[i * 4 + 2] = src[i * 3 + 2];
            rgbaData[i * 4 + 3] = 255;
        }
        imageData = rgbaData.data();
    }

    // RHITexture 생성
    TextureInfo texInfo{};
    texInfo.format = format;
    texInfo.type = ETextureType::TEX_2D;
    texInfo.usage = ETextureUsage::SAMPLED;
    texInfo.extent.width = width;
    texInfo.extent.height = height;
    texInfo.extent.depth = 1;
    texInfo.mipLevel = 1;
    texInfo.arrayLength = 1;

    resource.texture = _rhiContext->CreateTexture("ImageTex",
        const_cast<void*>(imageData), texInfo);

    // RHISampler 생성
    SamplerInfo sampInfo{};
    sampInfo.type = ETextureType::TEX_2D;
    sampInfo.minFilter = EFilterMode::LINEAR;
    sampInfo.magFilter = EFilterMode::LINEAR;
    sampInfo.mipmapMode = EFilterMode::LINEAR;
    sampInfo.addressU = EAddressMode::REPEAT;
    sampInfo.addressV = EAddressMode::REPEAT;
    sampInfo.addressW = EAddressMode::REPEAT;

    resource.sampler = _rhiContext->CreateSampler("ImageSampler", sampInfo);

    resource.width = width;
    resource.height = height;
    resource.format = format;
    resource.isValid = (resource.texture != nullptr && resource.sampler != nullptr);

    return resource;
}
```

**ReleaseAll() 수정**:
```cpp
// _imageResources 정리 추가
for (auto& [img, res] : _imageResources)
{
    if (res.texture) _rhiContext->DestroyTexture(res.texture);
    if (res.sampler) _rhiContext->DestroySampler(res.sampler);
}
_imageResources.clear();
```

---

### 3단계: MaterialResource에 텍스처 참조 추가
**파일**: `Source/Engine/Renderer/RenderResourceManager.h`

```cpp
struct HS_API MaterialResource
{
    // 기존 필드들...
    RHIShader* vertexShader = nullptr;
    RHIShader* fragmentShader = nullptr;
    RHIResourceLayout* resourceLayout = nullptr;
    RHIResourceSet* resourceSet = nullptr;
    std::vector<RHIBuffer*> materialBuffers;
    std::unordered_map<size_t, RHIGraphicsPipeline*> pipelineCache;

    // 추가: 텍스처 리소스 참조
    std::unordered_map<EMaterialTextureType, ImageResource*> textureResources;

    bool isValid = false;
};
```

---

### 4단계: createResourceLayoutFromReflection 수정
**파일**: `Source/Engine/Renderer/Private/RenderResourceManager.cpp`

**수정 사항**:
- 기존 버퍼 바인딩 로직 유지
- 텍스처 바인딩 로직 추가
- 샘플러 바인딩 로직 추가 (Vulkan의 combined image sampler 고려)

```cpp
RHIResourceLayout* RenderResourceManager::createResourceLayoutFromReflection(
    const ShaderReflectionDataEx& reflection,
    Material* material)  // 파라미터 추가
{
    std::vector<ResourceBinding> bindings;

    // 1. 기존 버퍼 바인딩 (perView, perDraw)
    for (const auto& buf : reflection.bufferBindings)
    {
        // ... 기존 코드 유지 ...
    }

    // 2. 텍스처 바인딩 추가
    for (const auto& tex : reflection.textureBindings)
    {
        // 텍스처 이름에서 Material 텍스처 타입 매핑
        EMaterialTextureType texType = mapTextureNameToType(tex.name);
        Image* image = material->GetTexture(texType);

        if (!image) continue;

        ImageResource* imgRes = GetOrCreateImageResource(image);
        if (!imgRes || !imgRes->isValid) continue;

        ResourceBinding binding{};
        binding.type = EResourceType::COMBINED_IMAGE_SAMPLER;  // 또는 SAMPLED_IMAGE
        binding.stage = tex.stages;
        binding.binding = static_cast<uint8>(tex.binding);
        binding.arrayCount = 1;
        binding.resource.textures.push_back(imgRes->texture);
        binding.resource.samplers.push_back(imgRes->sampler);
        bindings.push_back(std::move(binding));
    }

    return _rhiContext->CreateResourceLayout(
        "AutoLayout",
        bindings.data(),
        static_cast<uint32>(bindings.size()));
}

// 헬퍼 함수
EMaterialTextureType RenderResourceManager::mapTextureNameToType(const std::string& name)
{
    // 셰이더의 텍스처 이름 → Material 텍스처 타입 매핑
    if (name.find("albedo") != std::string::npos ||
        name.find("diffuse") != std::string::npos ||
        name.find("baseColor") != std::string::npos)
        return EMaterialTextureType::DIFFUSE;

    if (name.find("normal") != std::string::npos)
        return EMaterialTextureType::NORMAL;

    if (name.find("metallic") != std::string::npos ||
        name.find("metalness") != std::string::npos)
        return EMaterialTextureType::METALLIC;

    if (name.find("roughness") != std::string::npos)
        return EMaterialTextureType::ROUGHNESS;

    if (name.find("emission") != std::string::npos ||
        name.find("emissive") != std::string::npos)
        return EMaterialTextureType::EMISSION;

    if (name.find("ao") != std::string::npos ||
        name.find("occlusion") != std::string::npos)
        return EMaterialTextureType::AMBIENT_OCCLUSION;

    return EMaterialTextureType::DIFFUSE;  // 기본값
}
```

---

### 5단계: createMaterialResources 수정
**파일**: `Source/Engine/Renderer/Private/RenderResourceManager.cpp`

Material 포인터를 `createResourceLayoutFromReflection`에 전달하도록 수정:

```cpp
MaterialResource RenderResourceManager::createMaterialResources(Material* material)
{
    // ... 기존 코드 ...

    // 수정: material 파라미터 추가
    resources.resourceLayout = createResourceLayoutFromReflection(reflection, material);

    // ... 기존 코드 ...
}
```

---

### 6단계: 시그니처 변경에 따른 수정
`createResourceLayoutFromReflection` 시그니처 변경:

**헤더**:
```cpp
RHIResourceLayout* createResourceLayoutFromReflection(
    const ShaderReflectionDataEx& reflection,
    Material* material = nullptr);
```

---

## 파일 수정 목록

1. **`Source/Engine/Renderer/RenderResourceManager.h`**
   - `ImageResource` 구조체 추가
   - `MaterialResource`에 `textureResources` 필드 추가
   - `GetOrCreateImageResource()` 메서드 추가
   - `_imageResources` 캐시 맵 추가
   - `createResourceLayoutFromReflection` 시그니처 수정

2. **`Source/Engine/Renderer/Private/RenderResourceManager.cpp`**
   - `GetOrCreateImageResource()` 구현
   - `createImageResource()` 구현
   - `createResourceLayoutFromReflection()` 수정 (텍스처 바인딩 추가)
   - `createMaterialResources()` 수정
   - `mapTextureNameToType()` 헬퍼 추가
   - `ReleaseAll()` 수정 (ImageResource 정리)
   - `#include "Resource/Image.h"` 추가

3. **`Source/Engine/Resource/Material.h`** (필요시)
   - `GetTextures()` 메서드 추가 (전체 텍스처 맵 접근용)

---

## 검증 사항

1. **빌드 테스트**: Windows(Vulkan) 빌드 확인
2. **런타임 테스트**: DamagedHelmet GLTF 로드 및 텍스처 렌더링 확인
3. **로그 확인**: ImageResource 생성 로그 출력

---

## 향후 고려 사항

- Mipmap 생성 지원
- 텍스처 압축 포맷 지원 (DXT, BC)
- 텍스처 스트리밍
- Mac(Metal) 플랫폼 호환성 검증
