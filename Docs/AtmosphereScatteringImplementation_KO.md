# 대기 산란 구현 로그

## 개요

이 문서는 Bruneton & Neyret 논문 기반의 사전 계산 대기 산란(Precomputed Atmospheric Scattering) 시스템의 구현 및 디버깅 과정을 요약합니다.

---

## 단계별 진행 상황

| 단계 | 설명 | 상태 |
|------|------|------|
| Phase 1 | RHI 인프라 (UAV, 메모리 배리어) | 완료 |
| Phase 2 | 사전 계산 셰이더 & LUT 생성 | 완료 |
| Phase 3 | 런타임 렌더링 | 완료 |

---

## 발생한 문제 및 해결 방법

### 1. 부동소수점 픽셀 포맷 지원

**문제**: `VulkanUtility.cpp`에서 LUT 텍스처에 필요한 부동소수점 픽셀 포맷(R16F, RG16F, RGBA16F, R32F, RG32F, RGBA32F)을 지원하지 않음.

**원인**: 포맷 매핑이 구현되지 않음.

**해결**: `Source/RHI/Vulkan/Private/VulkanUtility.cpp`에 포맷 매핑 추가:
```cpp
case EPixelFormat::R16F:    return VK_FORMAT_R16_SFLOAT;
case EPixelFormat::RG16F:   return VK_FORMAT_R16G16_SFLOAT;
case EPixelFormat::RGBA16F: return VK_FORMAT_R16G16B16A16_SFLOAT;
case EPixelFormat::R32F:    return VK_FORMAT_R32_SFLOAT;
case EPixelFormat::RG32F:   return VK_FORMAT_R32G32_SFLOAT;
case EPixelFormat::RGBA32F: return VK_FORMAT_R32G32B32A32_SFLOAT;
```

---

### 2. 셰이더 경로 해석 오류

**문제**: 존재하지 않는 `Atmosphere/` 접두사가 포함된 경로로 셰이더 로딩 실패.

**원인**: 셰이더 경로 문자열이 잘못 설정됨.

**해결**: `AtmospherePrecompute.cpp`에서 전체 에셋 디렉토리 경로 사용:
```cpp
#ifdef __WINDOWS__
    std::string shaderPath = SystemContext::Get()->assetDirectory + "Shaders\\";
#endif
```

---

### 3. 3D 텍스처 깊이 처리

**문제**: `VulkanContext::CreateTexture`가 모든 텍스처에 대해 `depth=1`을 하드코딩하여 3D 텍스처 생성 실패.

**원인**: 텍스처 타입에 관계없이 depth를 1로 고정.

**해결**: TEX_3D 타입에 대해 실제 depth 값 사용:
```cpp
bufferCopyRegion.imageExtent.depth = (info.type == ETextureType::TEX_3D) ? info.extent.depth : 1;
imageCreateInfo.extent.depth = (info.type == ETextureType::TEX_3D) ? info.extent.depth : 1;
```

---

### 4. Push Constants 오류

**문제**: Vulkan 검증 오류:
```
vkCreateComputePipelines(): Push constant are used, but VkPipelineLayout doesn't set VK_SHADER_STAGE_COMPUTE_BIT
```

**원인**: 셰이더에서 `[[vk::push_constant]]`를 사용했지만 파이프라인 레이아웃에서 push constants를 선언하지 않음.

**해결**: 셰이더 로직을 단순화하여 push constants 완전 제거:

- **변경 전**: `scatteringOrder` 매개변수로 조건부 MultipleScatteringTexture 샘플링
- **변경 후**: 항상 MultipleScatteringTexture 샘플링 (order 2일 때 제로 텍스처 바인딩)

**수정된 파일**:
- `ScatteringDensity.comp.slang` - 조건문 제거, 항상 multipleScattering 더하기
- `IndirectIrradiance.comp.slang` - 동일한 변경

---

### 5. 명시적 바인딩 어노테이션

**문제**: 모든 셰이더에 `[[vk::binding(N, 0)]]` 어노테이션 추가함.

**결정**: RHI가 이름 기반 바인딩을 사용하므로 모든 명시적 바인딩 제거. 선택적 어노테이션은 성능 이점 없이 유지보수 오버헤드만 추가.

**수정된 파일** (어노테이션 제거):
- `Transmittance.comp.slang`
- `DirectIrradiance.comp.slang`
- `SingleScattering.comp.slang`
- `ScatteringDensity.comp.slang`
- `IndirectIrradiance.comp.slang`
- `MultipleScattering.comp.slang`

---

### 6. 디스크립터 풀 타입 누락

**문제**:
```
vkAllocateDescriptorSets(): VkDescriptorPool was not created with VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
```

**원인**: 디스크립터 풀에 SAMPLED_IMAGE 및 SAMPLER 타입이 포함되지 않음.

**해결**: `VulkanContext.cpp:103-111`에 누락된 디스크립터 타입 추가:
```cpp
std::vector<DescriptorPoolAllocatorVulkan::PoolSizeRatio> ratios = {
    {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3},
    {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3},
    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3},
    {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4},
    {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 3},      // 추가됨
    {VK_DESCRIPTOR_TYPE_SAMPLER, 3},            // 추가됨
};
```

---

### 7. 디스크립터 셋 바인딩 안됨

**문제**:
```
vkCmdDispatch(): descriptor set 0 was never bound
```

**원인**: `BindComputeResourceSet`이 구현되지 않음 (빈 스텁).

**해결**:

1. `VulkanRenderHandle.h`의 `PipelineVulkanBase`에 `layout` 필드 추가:
```cpp
struct HS_API PipelineVulkanBase {
    VkPipeline handle = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;  // 추가됨
};
```

2. `createComputePipeline`이 out 매개변수로 layout 반환하도록 수정

3. `BindComputePipeline`에서 layout 저장:
```cpp
curComputePipelineLayout = pipelineVK->layout;
```

4. `BindComputeResourceSet` 구현:
```cpp
void CommandBufferVulkan::BindComputeResourceSet(RHIResourceSet* rSet) {
    ResourceSetVulkan* rSetVK = static_cast<ResourceSetVulkan*>(rSet);
    vkCmdBindDescriptorSets(handle, VK_PIPELINE_BIND_POINT_COMPUTE,
        curComputePipelineLayout, 0, 1, &rSetVK->handle, 0, nullptr);
}
```

---

### 8. 디스크립터 업데이트 안됨

**문제**:
```
descriptor has never been updated via vkUpdateDescriptorSets()
```

**원인**: `CreateResourceSet`이 디스크립터 셋을 할당만 하고 리소스를 쓰지 않음.

**해결**: `CreateResourceSet`에 전체 디스크립터 쓰기 구현 (~100줄 추가):

```cpp
RHIResourceSet* VulkanContext::CreateResourceSet(...) {
    // ... 디스크립터 셋 할당 ...

    std::vector<VkWriteDescriptorSet> descriptorWrites;
    std::vector<VkDescriptorBufferInfo> bufferInfos;
    std::vector<VkDescriptorImageInfo> imageInfos;

    for (const auto& binding : resourceLayouts->bindings) {
        switch (binding.type) {
            case EResourceType::UNIFORM_BUFFER:
                // 버퍼 디스크립터 쓰기
                break;
            case EResourceType::SAMPLED_IMAGE:
                // 샘플 이미지 디스크립터 쓰기
                break;
            case EResourceType::STORAGE_IMAGE:
                // 스토리지 이미지 디스크립터 쓰기
                break;
            case EResourceType::SAMPLER:
                // 샘플러 디스크립터 쓰기
                break;
            // ... 등
        }
    }

    vkUpdateDescriptorSets(_device, descriptorWrites.size(),
        descriptorWrites.data(), 0, nullptr);

    return resourceSetVK;
}
```

---

### 9. 이미지 레이아웃 불일치

**문제**:
```
Cannot use VkImage with layout VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
that doesn't match previous layout VK_IMAGE_LAYOUT_GENERAL
```

**원인**: SAMPLED_IMAGE가 `SHADER_READ_ONLY_OPTIMAL`을 사용했지만 텍스처는 STORAGE_IMAGE 사용으로 인해 `GENERAL` 레이아웃 상태.

**해결**: 컴퓨트 셰이더 호환성을 위해 SAMPLED_IMAGE 및 COMBINED_IMAGE_SAMPLER가 `VK_IMAGE_LAYOUT_GENERAL` 사용:
```cpp
case EResourceType::SAMPLED_IMAGE:
    imgInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;  // SHADER_READ_ONLY_OPTIMAL에서 변경
    break;
```

---

## 사전 계산 완료 결과

모든 수정 후 대기 사전 계산이 성공적으로 실행:
```
[INFO] AtmospherePrecompute initialized successfully
[INFO] Starting atmosphere precomputation...
[INFO] Atmosphere precomputation complete
```

---

## 수정된 파일 요약

### 셰이더 (단순화)
| 파일 | 변경 내용 |
|------|-----------|
| `Transmittance.comp.slang` | `[[vk::binding]]` 제거 |
| `DirectIrradiance.comp.slang` | `[[vk::binding]]` 제거 |
| `SingleScattering.comp.slang` | `[[vk::binding]]` 제거, push_constant 제거 |
| `ScatteringDensity.comp.slang` | `[[vk::binding]]` 제거, scatteringOrder 조건 제거 |
| `IndirectIrradiance.comp.slang` | `[[vk::binding]]` 제거, scatteringOrder 조건 제거 |
| `MultipleScattering.comp.slang` | `[[vk::binding]]` 제거 |

### RHI 레이어
| 파일 | 변경 내용 |
|------|-----------|
| `VulkanUtility.cpp` | 부동소수점 포맷 지원 추가 |
| `VulkanContext.cpp` | 디스크립터 풀 타입, CreateResourceSet 구현, 파이프라인 레이아웃 처리 |
| `VulkanContext.h` | createComputePipeline 시그니처 변경 |
| `VulkanRenderHandle.h` | PipelineVulkanBase에 layout 필드 추가 |
| `VulkanCommandHandle.cpp` | BindComputeResourceSet 구현 |

### 엔진 레이어
| 파일 | 변경 내용 |
|------|-----------|
| `AtmospherePrecompute.cpp` | 셰이더 경로 수정, CreateResourceLayouts에서 리소스 바인딩 |
| `AtmosphereSkyPass.cpp` | null 체크, depth testing 비활성화 |

---

## 배운 점

1. **셰이더는 단순하게**: `[[vk::binding]]`과 같은 선택적 어노테이션은 이름 기반 바인딩 사용 시 이점 없이 유지보수 오버헤드만 추가
2. **Push constants는 파이프라인 레이아웃 지원 필요**: push constants를 사용하지 않으면 셰이더에서 선언하지 말 것
3. **디스크립터 셋은 명시적 업데이트 필요**: 할당만으로는 부족하고 `vkUpdateDescriptorSets` 호출 필수
4. **이미지 레이아웃은 일관성 있게**: 컴퓨트 셰이더에서 읽기/쓰기 모두 사용하는 텍스처는 `VK_IMAGE_LAYOUT_GENERAL` 사용
5. **디스크립터 풀에 모든 타입 포함**: 사용할 모든 디스크립터 타입을 사전 할당

---

## 추가 이슈 (런타임 렌더링 단계)

### 10. STATIC 버퍼 vkMapMemory 오류

**문제**:
```
vkMapMemory(): Mapping memory without VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT set.
Memory has type 1 which has properties VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT.
```

**원인**: `CreateBuffer`가 STATIC 버퍼(DEVICE_LOCAL 메모리)에 초기 데이터가 제공될 때 직접 매핑 시도.

**해결**: STATIC 버퍼에 초기 데이터가 있을 때 스테이징 버퍼 사용:
```cpp
bool needsStaging = (data != nullptr) && (info.memoryOption == EBufferMemoryOption::STATIC);

if (needsStaging) {
    // HOST_VISIBLE 메모리로 스테이징 버퍼 생성
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;
    // ... 스테이징 버퍼 생성 및 할당 ...

    // 스테이징 버퍼에 데이터 복사
    void* mapped;
    vkMapMemory(_device, stagingMemory, 0, dataSize, 0, &mapped);
    memcpy(mapped, data, dataSize);
    vkUnmapMemory(_device, stagingMemory);

    // 스테이징에서 디바이스 로컬 버퍼로 복사
    VkCommandBuffer cmdBuffer = beginSingleTimeCommands();
    VkBufferCopy copyRegion{};
    copyRegion.size = dataSize;
    vkCmdCopyBuffer(cmdBuffer, stagingBuffer, bufferVk, 1, &copyRegion);
    endSingleTimeCommands(cmdBuffer);

    // 스테이징 리소스 정리
    vkDestroyBuffer(_device, stagingBuffer, nullptr);
    vkFreeMemory(_device, stagingMemory, nullptr);
}
```

**수정된 파일**: `VulkanContext.cpp` - CreateBuffer 함수

---

### 11. 그래픽스 파이프라인 레이아웃 디스크립터 누락

**문제**:
```
vkCreateGraphicsPipelines(): SPIR-V uses descriptor [Set 0, Binding 0]
but was not declared in VkPipelineLayoutCreateInfo::pSetLayouts[0]
```

**원인**: `createGraphicsPipeline`에 TODO 주석만 있고 리소스 레이아웃을 사용하지 않음.

**해결**: 파이프라인 레이아웃 생성 시 리소스 레이아웃의 디스크립터 셋 레이아웃 사용:
```cpp
VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
if (info.resourceLayout != nullptr) {
    ResourceLayoutVulkan* resourceLayoutVK = static_cast<ResourceLayoutVulkan*>(info.resourceLayout);
    descriptorSetLayout = resourceLayoutVK->handle;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts    = &descriptorSetLayout;
}
```

또한 그래픽스 파이프라인용 `BindResourceSet` 구현:
```cpp
void CommandBufferVulkan::BindResourceSet(RHIResourceSet* rSet) {
    ResourceSetVulkan* rSetVK = static_cast<ResourceSetVulkan*>(rSet);
    vkCmdBindDescriptorSets(handle, VK_PIPELINE_BIND_POINT_GRAPHICS,
        curGraphicsPipelineLayout, 0, 1, &rSetVK->handle, 0, nullptr);
}
```

**수정된 파일**:
- `VulkanContext.cpp` - createGraphicsPipeline 함수
- `VulkanContext.h` - 함수 시그니처 변경
- `VulkanCommandHandle.cpp` - BindResourceSet 및 BindPipeline
- `VulkanCommandHandle.h` - curGraphicsPipelineLayout 필드 추가

---

### 12. 스토리지 텍스처 레이아웃 UNDEFINED

**문제**:
```
VkImage expects layout VK_IMAGE_LAYOUT_GENERAL--instead, current layout is VK_IMAGE_LAYOUT_UNDEFINED
```

**원인**: 초기 데이터 없이 생성된 스토리지 텍스처가 레이아웃 전환 없이 `VK_IMAGE_LAYOUT_UNDEFINED` 상태로 유지.

**해결**: 텍스처 생성 시 스토리지 텍스처를 GENERAL 레이아웃으로 전환:
```cpp
else if (isStorageTexture) {
    // 초기 데이터 없는 스토리지 텍스처는 GENERAL 레이아웃으로 전환 필요
    VkCommandBuffer transitionCmd = beginSingleTimeCommands();

    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel   = 0;
    subresourceRange.levelCount     = imageCreateInfo.mipLevels;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.layerCount     = imageCreateInfo.arrayLayers;

    VkImageMemoryBarrier imageMemoryBarrier{};
    imageMemoryBarrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.image               = imageVk;
    imageMemoryBarrier.subresourceRange    = subresourceRange;
    imageMemoryBarrier.srcAccessMask       = 0;
    imageMemoryBarrier.dstAccessMask       = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
    imageMemoryBarrier.oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
    imageMemoryBarrier.newLayout           = VK_IMAGE_LAYOUT_GENERAL;

    vkCmdPipelineBarrier(
        transitionCmd,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &imageMemoryBarrier
    );

    endSingleTimeCommands(transitionCmd);

    initialLayout = VK_IMAGE_LAYOUT_GENERAL;
}
```

**수정된 파일**: `VulkanContext.cpp` - CreateTexture 함수

---

### 13. DYNAMIC 버퍼 메모리 일관성 문제 (검은 화면 / 흰색 Transmittance)

**문제**:
RenderDoc에서 확인한 결과:
- TransmittanceTexture: 전체 (1.0, 1.0, 1.0, 1.0) - 순수 흰색
- ScatteringTexture: 전체 (0.0, 0.0, 0.0, 1.0) - 순수 검은색
- 결과: 완전히 검은 하늘

**증상 분석**:
| 텍스처 | 값 | 수학적 의미 |
|--------|-----|------------|
| Transmittance = (1,1,1,1) | `exp(-optical_depth) = 1` | optical_depth = 0 |
| Scattering = (0,0,0,1) | 산란 계산 안됨 | 모든 계수 = 0 |

이는 GPU가 대기 파라미터를 0으로 읽었다는 것을 의미합니다.

**원인**:
`EBufferMemoryOption::DYNAMIC`이 `HOST_COHERENT_BIT` 없이 `HOST_VISIBLE_BIT | HOST_CACHED_BIT`을 사용:

```cpp
case EBufferMemoryOption::DYNAMIC:
    properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
```

`HOST_COHERENT_BIT`이 없으면 CPU 쓰기가 CPU 캐시에만 남고 GPU에서 보이지 않음:
```
CPU ──memcpy──> CPU 캐시 ──(flush 필요)──> GPU 메모리
                    ↑
              데이터가 여기서 멈춤!
```

**Vulkan 사양 참조**:
> "호스트가 메모리 객체에 쓴 내용은 `vkFlushMappedMemoryRanges` 호출 후에만 호스트 도메인에서 사용 가능해집니다. 호스트가 메모리 객체에서 읽을 때 디바이스 쓰기 내용을 보려면 읽기 전에 `vkInvalidateMappedMemoryRanges`를 호출해야 합니다."
> — [Vulkan Spec 1.3, Section 11.2.1 "Host Access to Device Memory Objects"](https://docs.vulkan.org/spec/latest/chapters/memory.html#memory-device-hostaccess)

> "`VK_MEMORY_PROPERTY_HOST_COHERENT_BIT` 비트는 호스트 캐시 관리 명령인 `vkFlushMappedMemoryRanges`와 `vkInvalidateMappedMemoryRanges`가 각각 호스트 쓰기를 디바이스로 플러시하거나 디바이스 쓰기를 호스트에서 볼 수 있게 하는 데 필요하지 않음을 지정합니다."
> — [Vulkan Spec 1.3, Section 11.2.1](https://docs.vulkan.org/spec/latest/chapters/memory.html#VkMemoryPropertyFlagBits)

**해결**: DYNAMIC 버퍼에 `HOST_COHERENT_BIT` 사용:
```cpp
case EBufferMemoryOption::DYNAMIC:
    properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    break;
```

**대안적 해결책** (성능상 HOST_CACHED_BIT이 필요한 경우):
```cpp
memcpy(mapped, data, dataSize);
// HOST_COHERENT_BIT 없이는 플러시 필요
VkMappedMemoryRange range{};
range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
range.memory = bufferMemory;
range.offset = 0;
range.size = VK_WHOLE_SIZE;
vkFlushMappedMemoryRanges(_device, 1, &range);
vkUnmapMemory(_device, bufferMemory);
```

**수정된 파일**: `VulkanContext.cpp:490-492`

---

### 14. 디스크립터 풀 FREE_DESCRIPTOR_SET_BIT 누락

**문제** (애플리케이션 종료 시):
```
vkFreeDescriptorSets(): descriptorPool was created with VkDescriptorPoolCreateFlags(0)
(missing FREE_DESCRIPTOR_SET_BIT)
```

**원인**: 디스크립터 풀이 `VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT` 플래그 없이 생성되었지만 `vkFreeDescriptorSets`가 호출됨.

**해결**: 디스크립터 풀 생성 시 플래그 추가:
```cpp
poolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
```

**수정된 파일**: `VulkanDescriptorPoolAllocator.cpp:127`

---

## 최종 결과

모든 Vulkan 검증 오류 해결. 대기 사전 계산이 올바른 LUT 텍스처를 생성하고 런타임 스카이 렌더링이 정상 작동:
```
[INFO] AtmospherePrecompute initialized successfully
[INFO] Starting atmosphere precomputation...
[INFO] Atmosphere precomputation complete
```

애플리케이션이 오류 없이 실행됩니다.

---

## 다음 단계

1. 태양 디스크 렌더링 추가
2. 씬 카메라와 통합
3. 노출/톤 매핑 컨트롤 추가
4. 시각적 품질 검증
