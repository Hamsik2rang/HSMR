# Atmospheric Scattering Implementation Log

## Overview

This document summarizes the implementation and debugging process for the Precomputed Atmospheric Scattering system based on Bruneton & Neyret's paper.

---

## Phase Summary

| Phase | Description | Status |
|-------|-------------|--------|
| Phase 1 | RHI Infrastructure (UAV, memory barriers) | Complete |
| Phase 2 | Precomputation Shaders & LUT Generation | Complete |
| Phase 3 | Runtime Rendering | Complete |

---

## Issues Encountered and Solutions

### 1. Floating-Point Pixel Format Support

**Problem**: `VulkanUtility.cpp` didn't support floating-point pixel formats (R16F, RG16F, RGBA16F, R32F, RG32F, RGBA32F) required for LUT textures.

**Solution**: Added format mappings in `Source/RHI/Vulkan/Private/VulkanUtility.cpp`:
```cpp
case EPixelFormat::R16F:    return VK_FORMAT_R16_SFLOAT;
case EPixelFormat::RG16F:   return VK_FORMAT_R16G16_SFLOAT;
case EPixelFormat::RGBA16F: return VK_FORMAT_R16G16B16A16_SFLOAT;
case EPixelFormat::R32F:    return VK_FORMAT_R32_SFLOAT;
case EPixelFormat::RG32F:   return VK_FORMAT_R32G32_SFLOAT;
case EPixelFormat::RGBA32F: return VK_FORMAT_R32G32B32A32_SFLOAT;
```

---

### 2. Shader Path Resolution

**Problem**: Shader loading failed because paths included non-existent `Atmosphere/` prefix.

**Solution**: Updated `AtmospherePrecompute.cpp` to use full asset directory path:
```cpp
#ifdef __WINDOWS__
    std::string shaderPath = SystemContext::Get()->assetDirectory + "Shaders\\";
#endif
```

---

### 3. 3D Texture Depth Handling

**Problem**: `VulkanContext::CreateTexture` hardcoded `depth=1` for all textures, breaking 3D texture creation.

**Solution**: Modified to use actual depth for TEX_3D types:
```cpp
bufferCopyRegion.imageExtent.depth = (info.type == ETextureType::TEX_3D) ? info.extent.depth : 1;
imageCreateInfo.extent.depth = (info.type == ETextureType::TEX_3D) ? info.extent.depth : 1;
```

---

### 4. Push Constants Error

**Problem**: Vulkan validation error:
```
vkCreateComputePipelines(): Push constant are used, but VkPipelineLayout doesn't set VK_SHADER_STAGE_COMPUTE_BIT
```

**Root Cause**: Shaders used `[[vk::push_constant]]` but pipeline layout didn't declare push constants.

**Solution**: Removed push constants entirely by simplifying the shader logic:

- **Before**: Used `scatteringOrder` parameter to conditionally sample MultipleScatteringTexture
- **After**: Always sample MultipleScatteringTexture (bind zero texture for order 2)

**Files Modified**:
- `ScatteringDensity.comp.slang` - Removed condition, always add multipleScattering
- `IndirectIrradiance.comp.slang` - Same change

---

### 5. Explicit Binding Annotations

**Problem**: Initially added `[[vk::binding(N, 0)]]` annotations to all shaders.

**Decision**: Removed all explicit bindings as they're optional and add maintenance overhead without performance benefit. The RHI uses name-based binding.

**Files Modified** (annotations removed):
- `Transmittance.comp.slang`
- `DirectIrradiance.comp.slang`
- `SingleScattering.comp.slang`
- `ScatteringDensity.comp.slang`
- `IndirectIrradiance.comp.slang`
- `MultipleScattering.comp.slang`

---

### 6. Descriptor Pool Missing Types

**Problem**:
```
vkAllocateDescriptorSets(): VkDescriptorPool was not created with VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
```

**Solution**: Added missing descriptor types in `VulkanContext.cpp:103-111`:
```cpp
std::vector<DescriptorPoolAllocatorVulkan::PoolSizeRatio> ratios = {
    {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3},
    {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3},
    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3},
    {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4},
    {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 3},      // Added
    {VK_DESCRIPTOR_TYPE_SAMPLER, 3},            // Added
};
```

---

### 7. Descriptor Set Never Bound

**Problem**:
```
vkCmdDispatch(): descriptor set 0 was never bound
```

**Root Cause**: `BindComputeResourceSet` was not implemented (empty stub).

**Solution**:

1. Added `layout` field to `PipelineVulkanBase` in `VulkanRenderHandle.h`:
```cpp
struct HS_API PipelineVulkanBase {
    VkPipeline handle = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;  // Added
};
```

2. Modified `createComputePipeline` to return layout via out parameter

3. Updated `BindComputePipeline` to store layout:
```cpp
curComputePipelineLayout = pipelineVK->layout;
```

4. Implemented `BindComputeResourceSet`:
```cpp
void CommandBufferVulkan::BindComputeResourceSet(RHIResourceSet* rSet) {
    ResourceSetVulkan* rSetVK = static_cast<ResourceSetVulkan*>(rSet);
    vkCmdBindDescriptorSets(handle, VK_PIPELINE_BIND_POINT_COMPUTE,
        curComputePipelineLayout, 0, 1, &rSetVK->handle, 0, nullptr);
}
```

---

### 8. Descriptors Never Updated

**Problem**:
```
descriptor has never been updated via vkUpdateDescriptorSets()
```

**Root Cause**: `CreateResourceSet` allocated descriptor set but never wrote resources to it.

**Solution**: Implemented full descriptor writing in `CreateResourceSet` (~100 lines added):

```cpp
RHIResourceSet* VulkanContext::CreateResourceSet(...) {
    // ... allocate descriptor set ...

    std::vector<VkWriteDescriptorSet> descriptorWrites;
    std::vector<VkDescriptorBufferInfo> bufferInfos;
    std::vector<VkDescriptorImageInfo> imageInfos;

    for (const auto& binding : resourceLayouts->bindings) {
        switch (binding.type) {
            case EResourceType::UNIFORM_BUFFER:
                // Write buffer descriptor
                break;
            case EResourceType::SAMPLED_IMAGE:
                // Write sampled image descriptor
                break;
            case EResourceType::STORAGE_IMAGE:
                // Write storage image descriptor
                break;
            case EResourceType::SAMPLER:
                // Write sampler descriptor
                break;
            // ... etc
        }
    }

    vkUpdateDescriptorSets(_device, descriptorWrites.size(),
        descriptorWrites.data(), 0, nullptr);

    return resourceSetVK;
}
```

---

### 9. Image Layout Mismatch

**Problem**:
```
Cannot use VkImage with layout VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
that doesn't match previous layout VK_IMAGE_LAYOUT_GENERAL
```

**Root Cause**: SAMPLED_IMAGE used `SHADER_READ_ONLY_OPTIMAL` but textures were in `GENERAL` layout from STORAGE_IMAGE usage.

**Solution**: Changed SAMPLED_IMAGE and COMBINED_IMAGE_SAMPLER to use `VK_IMAGE_LAYOUT_GENERAL` for compute shader compatibility:
```cpp
case EResourceType::SAMPLED_IMAGE:
    imgInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;  // Changed from SHADER_READ_ONLY_OPTIMAL
    break;
```

---

## Final Result

After all fixes, the atmosphere precomputation runs successfully:
```
[INFO] AtmospherePrecompute initialized successfully
[INFO] Starting atmosphere precomputation...
[INFO] Atmosphere precomputation complete
```

---

## Files Modified Summary

### Shaders (Simplified)
| File | Changes |
|------|---------|
| `Transmittance.comp.slang` | Removed `[[vk::binding]]` |
| `DirectIrradiance.comp.slang` | Removed `[[vk::binding]]` |
| `SingleScattering.comp.slang` | Removed `[[vk::binding]]`, removed push_constant |
| `ScatteringDensity.comp.slang` | Removed `[[vk::binding]]`, removed scatteringOrder condition |
| `IndirectIrradiance.comp.slang` | Removed `[[vk::binding]]`, removed scatteringOrder condition |
| `MultipleScattering.comp.slang` | Removed `[[vk::binding]]` |

### RHI Layer
| File | Changes |
|------|---------|
| `VulkanUtility.cpp` | Added floating-point format support |
| `VulkanContext.cpp` | Descriptor pool types, CreateResourceSet implementation, pipeline layout handling |
| `VulkanContext.h` | createComputePipeline signature change |
| `VulkanRenderHandle.h` | Added layout field to PipelineVulkanBase |
| `VulkanCommandHandle.cpp` | Implemented BindComputeResourceSet |

### Engine Layer
| File | Changes |
|------|---------|
| `AtmospherePrecompute.cpp` | Shader path fix, resource binding in CreateResourceLayouts |
| `AtmosphereSkyPass.cpp` | Null checks, disabled depth testing |

---

## Lessons Learned

1. **Keep shaders simple**: Optional annotations like `[[vk::binding]]` add maintenance overhead without benefits when using name-based binding
2. **Push constants need pipeline layout support**: If not using push constants, don't declare them in shaders
3. **Descriptor sets require explicit updates**: Allocating is not enough; `vkUpdateDescriptorSets` must be called
4. **Image layouts must be consistent**: For compute shaders using textures as both read and write, use `VK_IMAGE_LAYOUT_GENERAL`
5. **Descriptor pools need all types**: Pre-allocate all descriptor types that will be used

---

## Additional Issues (Runtime Rendering Phase)

### 10. STATIC Buffer vkMapMemory Error

**Problem**:
```
vkMapMemory(): Mapping memory without VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT set.
Memory has type 1 which has properties VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT.
```

**Root Cause**: `CreateBuffer` tried to map DEVICE_LOCAL memory when initial data was provided for STATIC buffers.

**Solution**: Use staging buffer for STATIC buffers with initial data:
```cpp
if (needsStaging) {
    // Create staging buffer with HOST_VISIBLE memory
    // Copy data to staging buffer
    // Use vkCmdCopyBuffer to copy from staging to device-local
    // Cleanup staging resources
}
```

---

### 11. Graphics Pipeline Layout Missing Descriptors

**Problem**:
```
vkCreateGraphicsPipelines(): SPIR-V uses descriptor [Set 0, Binding 0]
but was not declared in VkPipelineLayoutCreateInfo::pSetLayouts[0]
```

**Root Cause**: `createGraphicsPipeline` had a TODO comment and didn't use the resource layout.

**Solution**: Use resource layout's descriptor set layout when creating pipeline layout:
```cpp
if (info.resourceLayout != nullptr) {
    ResourceLayoutVulkan* resourceLayoutVK = static_cast<ResourceLayoutVulkan*>(info.resourceLayout);
    descriptorSetLayout = resourceLayoutVK->handle;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts    = &descriptorSetLayout;
}
```

Also implemented `BindResourceSet` for graphics pipelines.

---

### 12. Storage Texture Layout UNDEFINED

**Problem**:
```
VkImage expects layout VK_IMAGE_LAYOUT_GENERAL--instead, current layout is VK_IMAGE_LAYOUT_UNDEFINED
```

**Root Cause**: Storage textures created without initial data never had layout transition.

**Solution**: Transition storage textures to GENERAL layout on creation:
```cpp
else if (isStorageTexture) {
    VkCommandBuffer transitionCmd = beginSingleTimeCommands();
    // Transition UNDEFINED -> GENERAL
    vkCmdPipelineBarrier(transitionCmd, ...);
    endSingleTimeCommands(transitionCmd);
    initialLayout = VK_IMAGE_LAYOUT_GENERAL;
}
```

---

### 13. DYNAMIC Buffer Memory Coherency (Black Screen / White Transmittance)

**Problem**:
RenderDoc showed:
- TransmittanceTexture: all (1.0, 1.0, 1.0, 1.0) - pure white
- ScatteringTexture: all (0.0, 0.0, 0.0, 1.0) - pure black
- Result: completely black sky

**Symptom Analysis**:
| Texture | Value | Mathematical Meaning |
|---------|-------|---------------------|
| Transmittance = (1,1,1,1) | `exp(-optical_depth) = 1` | optical_depth = 0 |
| Scattering = (0,0,0,1) | No scattering computed | All coefficients = 0 |

This indicates atmosphere parameters were read as zeros by GPU.

**Root Cause**:
`EBufferMemoryOption::DYNAMIC` used `HOST_VISIBLE_BIT | HOST_CACHED_BIT` without `HOST_COHERENT_BIT`:

```cpp
case EBufferMemoryOption::DYNAMIC:
    properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
```

Without `HOST_COHERENT_BIT`, CPU writes stay in CPU cache and are not visible to GPU:
```
CPU ──memcpy──> CPU Cache ──(flush required)──> GPU Memory
                    ↑
              Data stuck here!
```

**Vulkan Specification Reference**:
> "Host writes to the memory object will be made available to the host domain only after a call to `vkFlushMappedMemoryRanges`. Host reads from the memory object will only see updates from device writes if a call to `vkInvalidateMappedMemoryRanges` is made before reading."
> — [Vulkan Spec 1.3, Section 11.2.1 "Host Access to Device Memory Objects"](https://docs.vulkan.org/spec/latest/chapters/memory.html#memory-device-hostaccess)

> "`VK_MEMORY_PROPERTY_HOST_COHERENT_BIT` bit specifies that the host cache management commands `vkFlushMappedMemoryRanges` and `vkInvalidateMappedMemoryRanges` are not needed to flush host writes to the device or make device writes visible to the host, respectively."
> — [Vulkan Spec 1.3, Section 11.2.1](https://docs.vulkan.org/spec/latest/chapters/memory.html#VkMemoryPropertyFlagBits)

**Solution**: Use `HOST_COHERENT_BIT` for DYNAMIC buffers:
```cpp
case EBufferMemoryOption::DYNAMIC:
    properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    break;
```

**Alternative Solution** (if HOST_CACHED_BIT is required for performance):
```cpp
memcpy(mapped, data, dataSize);
// Flush required without HOST_COHERENT_BIT
VkMappedMemoryRange range{};
range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
range.memory = bufferMemory;
range.offset = 0;
range.size = VK_WHOLE_SIZE;
vkFlushMappedMemoryRanges(_device, 1, &range);
vkUnmapMemory(_device, bufferMemory);
```

**Files Modified**: `VulkanContext.cpp:490-492`

---

### 14. Descriptor Pool FREE_DESCRIPTOR_SET_BIT Missing

**Problem** (on application exit):
```
vkFreeDescriptorSets(): descriptorPool was created with VkDescriptorPoolCreateFlags(0)
(missing FREE_DESCRIPTOR_SET_BIT)
```

**Root Cause**: Descriptor pool created without `VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT` flag, but `vkFreeDescriptorSets` was called.

**Solution**: Add flag when creating descriptor pool:
```cpp
poolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
```

**Files Modified**: `VulkanDescriptorPoolAllocator.cpp:127`

---

## Final Result

All Vulkan validation errors resolved. Atmosphere precomputation produces correct LUT textures and runtime sky rendering is functional:
```
[INFO] AtmospherePrecompute initialized successfully
[INFO] Starting atmosphere precomputation...
[INFO] Atmosphere precomputation complete
```

Application runs without errors.

---

## Next Steps

1. Add sun disc rendering
2. Integrate with scene camera
3. Add exposure/tone mapping controls
4. Visual quality validation
