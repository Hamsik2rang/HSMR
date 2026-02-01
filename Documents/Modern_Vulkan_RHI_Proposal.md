# Modern Vulkan RHI 개선 제안서

**작성일**: 2026-01-30
**대상 버전**: Vulkan 1.4
**브랜치**: `enginelet`

---

## 개요

이 문서는 HSMR 엔진의 RHI(Rendering Hardware Interface)를 Vulkan 1.4의 modern extension들을 활용하여 단순화하고 성능을 개선하기 위한 제안서입니다.

### 목표
- RHI 코드 복잡도 감소 (~500줄 절감 예상)
- Bindless rendering 구현 기반 마련
- GPU-driven rendering 지원
- 런타임 유연성 향상

---

## 현재 RHI 구조 분석

### 문제점

| 영역 | 현재 구현 | 문제점 |
|------|----------|--------|
| **RenderPass** | 매 attachment 설정마다 별도 객체 생성 | 조합 폭발, 파이프라인 결합도 ↑ |
| **Framebuffer** | RenderPass에 종속, 크기별 재생성 필요 | 불필요한 객체 관리 |
| **DescriptorSet** | 고정 배열 크기, 매 draw마다 rebind | Bindless 불가, 오버헤드 |
| **PipelineLayout** | 단일 descriptor set만 지원 | 유연성 부족 |
| **Image Layout** | `VK_IMAGE_LAYOUT_GENERAL` 사용 | 성능 저하 10-15% |

### 현재 아키텍처

```
RHIContext (Abstract)
├── VulkanContext (Windows)
└── MetalContext (macOS)

Resource Types:
├── RHIRenderPass → RenderPassVulkan (VkRenderPass)
├── RHIFramebuffer → FramebufferVulkan (VkFramebuffer + attachments)
├── RHIGraphicsPipeline → GraphicsPipelineVulkan (VkPipeline + VkPipelineLayout)
├── RHIResourceLayout → ResourceLayoutVulkan (VkDescriptorSetLayout)
├── RHIResourceSet → ResourceSetVulkan (VkDescriptorSet)
└── RHIResourceSetPool → ResourceSetPoolVulkan (VkDescriptorPool)
```

---

## Vulkan 1.4 주요 Extension 활용 방안

### 1. VK_KHR_dynamic_rendering

**용도**: RenderPass/Framebuffer 객체 제거

**현재 코드:**
```cpp
RHIRenderPass* renderPass = context->CreateRenderPass(info);
RHIFramebuffer* framebuffer = context->CreateFramebuffer(fbInfo, renderPass);
cmdBuffer->BeginRenderPass(renderPass, framebuffer, area);
```

**개선 후:**
```cpp
VkRenderingAttachmentInfo colorAttachment{};
colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
colorAttachment.imageView = targetView;
colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
colorAttachment.clearValue = clearColor;

VkRenderingInfo renderingInfo{};
renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
renderingInfo.renderArea = area;
renderingInfo.layerCount = 1;
renderingInfo.colorAttachmentCount = 1;
renderingInfo.pColorAttachments = &colorAttachment;

vkCmdBeginRendering(cmd, &renderingInfo);  // RenderPass/Framebuffer 불필요
```

**제거 가능 코드:**
- `VulkanContext::createRenderPass()` ~120줄
- `VulkanContext::createFramebuffer()` ~60줄
- `RenderPassVulkan`, `FramebufferVulkan` 클래스 (또는 대폭 단순화)

**주의사항:**
- Swapchain용 RenderPass는 ImGui 호환성을 위해 유지 권장
- Pipeline 생성 시 `VK_PIPELINE_CREATE_RENDERING_INFO_KHR` 사용

---

### 2. VK_KHR_buffer_device_address

**용도**: Descriptor 없이 버퍼를 GPU 포인터로 직접 접근

**기존 방식:**
```cpp
// Descriptor를 통한 간접 접근
layout(binding = 0) uniform VertexBuffer {
    Vertex vertices[];
};
```

**Buffer Device Address:**
```cpp
// 포인터 직접 사용
layout(buffer_reference, scalar) buffer VertexBuffer {
    Vertex vertices[];
};

layout(push_constant) uniform PushConstants {
    uint64_t vertexBufferAddress;
    uint64_t indexBufferAddress;
};

void main() {
    VertexBuffer vb = VertexBuffer(vertexBufferAddress);
    Vertex v = vb.vertices[gl_VertexIndex];  // 포인터 연산 가능
}
```

**구현 요구사항:**
```cpp
// 1. Feature 활성화
VkPhysicalDeviceBufferDeviceAddressFeatures bdaFeatures{};
bdaFeatures.bufferDeviceAddress = VK_TRUE;

// 2. Buffer 생성 시 플래그 추가
VkBufferCreateInfo bufferInfo{};
bufferInfo.usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

// 3. Memory 할당 시 플래그 추가
VkMemoryAllocateFlagsInfo allocFlagsInfo{};
allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

// 4. 주소 획득
VkBufferDeviceAddressInfo addressInfo{};
addressInfo.buffer = buffer;
uint64_t address = vkGetBufferDeviceAddress(device, &addressInfo);
```

**장점:**
- Vertex/Index buffer descriptor 제거
- GPU-driven rendering 핵심 기능
- Meshlet 스타일 렌더링 지원
- Vulkan 1.3부터 **필수 기능**

---

### 3. VK_EXT_descriptor_indexing (Bindless)

**용도**: 동적 인덱스로 리소스 배열 접근

**현재 방식:**
```cpp
// 텍스처마다 descriptor set 업데이트
for (each material) {
    RHIResourceSet* set = context->CreateResourceSet(layout, bindings);
    cmdBuffer->BindResourceSet(set);  // 매번 rebind
    Draw();
}
```

**Bindless 방식:**
```glsl
// 셰이더
layout(binding = 0) uniform sampler2D textures[];  // unbounded array

layout(push_constant) uniform PushConstants {
    uint albedoIndex;
    uint normalIndex;
    uint materialIndex;
};

void main() {
    vec4 albedo = texture(textures[albedoIndex], uv);
    vec4 normal = texture(textures[normalIndex], uv);
}
```

**구현 요구사항:**
```cpp
// Feature 활성화
VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures{};
indexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
indexingFeatures.descriptorBindingVariableDescriptorCount = VK_TRUE;
indexingFeatures.runtimeDescriptorArray = VK_TRUE;
indexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;

// Layout 생성 시 플래그
VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlags{};
VkDescriptorBindingFlags flags =
    VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
    VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
    VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
```

**장점:**
- Descriptor set rebind 제거
- 텍스처 수 제한 없음 (동적 확장)
- Draw call 오버헤드 대폭 감소

---

### 4. VK_KHR_synchronization2

**용도**: 동기화 API 단순화

**현재 방식:**
```cpp
VkMemoryBarrier barrier{};
barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

vkCmdPipelineBarrier(cmd,
    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
    0, 1, &barrier, 0, nullptr, 0, nullptr);
```

**Synchronization2:**
```cpp
VkMemoryBarrier2 barrier2{};
barrier2.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
barrier2.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
barrier2.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
barrier2.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
barrier2.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;

VkDependencyInfo depInfo{};
depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
depInfo.memoryBarrierCount = 1;
depInfo.pMemoryBarriers = &barrier2;

vkCmdPipelineBarrier2(cmd, &depInfo);
```

**장점:**
- Stage와 Access가 하나의 구조체에 통합
- 더 명확한 의미론
- `VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT` 등 편의 플래그
- Vulkan 1.3부터 **필수 기능**

---

### 5. VK_KHR_push_descriptor (Vulkan 1.4 Core)

**용도**: Descriptor set 할당 없이 즉시 바인딩

**현재 방식:**
```cpp
VkDescriptorSet set = AllocateFromPool(pool);
vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
vkCmdBindDescriptorSets(cmd, ..., 1, &set, ...);
```

**Push Descriptor:**
```cpp
vkCmdPushDescriptorSetKHR(cmd,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    pipelineLayout,
    setIndex,
    1, &write);  // Pool 할당 불필요
```

**장점:**
- Descriptor pool 관리 제거
- Per-draw 리소스 바인딩에 최적
- Bindless와 조합 가능

---

### 6. VK_KHR_dynamic_rendering_local_read (Vulkan 1.4 Core)

**용도**: Dynamic rendering 내에서 이전 attachment 읽기

**사용 예시:**
```cpp
// Deferred rendering의 G-Buffer 읽기
vkCmdSetRenderingInputAttachmentIndicesKHR(cmd, &inputIndices);

// 셰이더에서 이전 color attachment 읽기
layout(input_attachment_index = 0) uniform subpassInput gbufferAlbedo;
```

**장점:**
- Deferred rendering을 dynamic rendering으로 구현 가능
- Tile-based GPU에서 on-chip 메모리 활용
- RenderPass subpass 완전 대체

---

## 추가 권장 Extension

| Extension | 용도 | 우선순위 | Vulkan 버전 |
|-----------|------|---------|-------------|
| **VK_EXT_mesh_shader** | Meshlet 기반 렌더링 | 높음 | Extension |
| **VK_KHR_maintenance5** | API 편의 기능 | 높음 | 1.4 Core |
| **VK_KHR_maintenance6** | API 편의 기능 | 높음 | 1.4 Core |
| **VK_EXT_shader_object** | Pipeline 없이 셰이더 바인딩 | 중간 | Extension |
| **VK_EXT_graphics_pipeline_library** | Pipeline 컴파일 분리 | 중간 | Extension |
| **VK_KHR_ray_tracing_pipeline** | 레이트레이싱 | 선택 | Extension |
| **VK_KHR_index_type_uint8** | 8비트 인덱스 버퍼 | 낮음 | 1.4 Core |

---

## 권장 아키텍처

### Bindless + Modern Vulkan 통합 구조

```
┌─────────────────────────────────────────────────────┐
│                    Per-Frame                         │
├─────────────────────────────────────────────────────┤
│  Set 0: Bindless Arrays (descriptor indexing)       │
│  ├── sampler2D textures[]     (unbounded)           │
│  ├── samplerCube cubemaps[]   (unbounded)           │
│  └── buffer materials[]       (unbounded)           │
├─────────────────────────────────────────────────────┤
│  Push Constants: Per-Draw (128 bytes)               │
│  ├── mat4 modelMatrix                               │
│  ├── uint materialIndex                             │
│  ├── uint textureIndices[4]                         │
│  └── uint64 vertexBufferAddress                     │
├─────────────────────────────────────────────────────┤
│  Push Descriptor: Dynamic Resources (optional)      │
│  └── Camera UBO (per-view 변경 시)                   │
├─────────────────────────────────────────────────────┤
│  Dynamic Rendering: No RenderPass/Framebuffer      │
│  └── vkCmdBeginRendering() 직접 호출                │
├─────────────────────────────────────────────────────┤
│  Synchronization2: 명확한 배리어                     │
│  └── vkCmdPipelineBarrier2()                        │
└─────────────────────────────────────────────────────┘
```

### 단순화된 RHI 클래스 구조

```
RHIContext (Abstract)
├── CreateBuffer()      → VkBuffer + device address
├── CreateTexture()     → VkImage + bindless index
├── CreatePipeline()    → VkPipeline (dynamic rendering compatible)
└── CreateSampler()     → VkSampler

RHICommandBuffer
├── BeginRendering()    → vkCmdBeginRendering (dynamic)
├── EndRendering()      → vkCmdEndRendering
├── PushConstants()     → vkCmdPushConstants
├── BindPipeline()      → vkCmdBindPipeline
├── BindBindlessSet()   → vkCmdBindDescriptorSets (1회/프레임)
├── Barrier()           → vkCmdPipelineBarrier2
└── Draw/Dispatch()

BindlessResourceManager (New)
├── RegisterTexture()   → bindless index 반환
├── UnregisterTexture()
├── RegisterBuffer()    → device address 반환
└── GetDescriptorSet()  → global bindless set
```

---

## 구현 단계

### Phase 1: 기반 작업 (빠른 승리)

1. **Image Layout 수정**
   - `VK_IMAGE_LAYOUT_GENERAL` → `VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL`
   - 예상 성능 향상: 10-15%

2. **Push Constants 지원 추가**
   - `PipelineLayout`에 push constant range 추가
   - 셰이더 per-draw 데이터 전달

3. **Synchronization2 적용**
   - 기존 barrier 코드를 `vkCmdPipelineBarrier2`로 교체

### Phase 2: Dynamic Rendering

1. **Dynamic rendering 경로 추가**
   - 기존 RenderPass 경로와 병행
   - Swapchain은 기존 방식 유지 (ImGui 호환)

2. **Pipeline 생성 수정**
   - `VkPipelineRenderingCreateInfo` 사용
   - RenderPass 의존성 제거

### Phase 3: Bindless 구현

1. **BindlessResourceManager 구현**
   - Global texture array 관리
   - Texture 등록/해제 API

2. **Buffer Device Address 적용**
   - Vertex/Index buffer에 device address 사용
   - Draw indirect 지원

3. **Descriptor Indexing 활성화**
   - Unbounded array 지원
   - Partial binding 활성화

### Phase 4: 최적화 및 정리

1. **레거시 코드 제거**
   - 미사용 RenderPass/Framebuffer 코드 정리
   - Descriptor pool 로직 단순화

2. **GPU-driven Rendering 준비**
   - Indirect draw buffer 지원
   - Compute culling 기반 마련

---

## 예상 효과

### 코드 감소

| 영역 | 제거/단순화 | 예상 감소량 |
|------|------------|------------|
| RenderPass 관리 | `createRenderPass()`, `createFramebuffer()` | ~200줄 |
| Descriptor Pool | `DescriptorPoolAllocator` 대폭 축소 | ~150줄 |
| Resource Binding | `CreateResourceSet()` 단순화 | ~100줄 |
| 동기화 코드 | Pipeline barrier 로직 | ~50줄 |
| **총계** | | **~500줄** |

### 성능 향상

| 영역 | 현재 | 개선 후 | 효과 |
|------|------|--------|------|
| Descriptor rebind | 수백회/프레임 | 1-2회/프레임 | CPU 오버헤드 감소 |
| Image layout | GENERAL | SHADER_READ_ONLY | GPU 성능 10-15% ↑ |
| RenderPass 생성 | N개 객체 | 0개 | 메모리 절약 |
| Draw call 준비 | 복잡 | 단순 | 배칭 용이 |

### 유연성 향상

- 런타임 attachment 결정
- 동적 텍스처 추가/제거
- GPU-driven rendering 지원
- 셰이더 내 포인터 연산

---

## 참고 자료

- [Vulkan Buffer Device Address Sample](https://docs.vulkan.org/samples/latest/samples/extensions/buffer_device_address/README.html)
- [Khronos Synchronization Examples](https://github.com/khronosgroup/vulkan-docs/wiki/synchronization-examples)
- [Vulkan 1.4 Announcement](https://www.khronos.org/news/press/khronos-streamlines-development-and-deployment-of-gpu-accelerated-applications-with-vulkan-1.4)
- [Vulkan 1.4 Documentation](https://docs.vulkan.org/features/latest/features/proposals/VK_VERSION_1_4.html)
- [How to Vulkan](https://www.howtovulkan.com/)
- [VMA Buffer Device Address](https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/enabling_buffer_device_address.html)

---

## 결론

Vulkan 1.4를 타겟으로 하면 위 기능들이 모두 **필수 지원**되므로 분기 처리 없이 사용 가능합니다. Dynamic rendering + descriptor indexing + buffer device address 조합은 현대적인 렌더러의 표준이 되어가고 있으며, bindless rendering 구현을 계획 중이라면 이 방향으로의 개선이 필수적입니다.
