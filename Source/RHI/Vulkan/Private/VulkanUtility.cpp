#include "RHI/Vulkan/VulkanUtility.h"

#include "RHI/Vulkan/VulkanResourceHandle.h"

HS_NS_BEGIN

void VulkanUtility::SetDebugObjectName(VkInstance instance, VkDevice device, VkObjectType type, uint64 handle, std::string_view name)
{
#ifdef _DEBUG
    static auto vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT");
    HS_ASSERT(vkSetDebugUtilsObjectNameEXT, "function addr is nullptr");

    VkDebugUtilsObjectNameInfoEXT nameInfo{};
    nameInfo.sType        = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    nameInfo.objectType   = type;
    nameInfo.objectHandle = handle;
    nameInfo.pObjectName  = name.data();
    nameInfo.pNext        = nullptr;

    vkSetDebugUtilsObjectNameEXT(device, &nameInfo);

#endif
}

VkFormat VulkanUtility::ToPixelFormat(EPixelFormat format)
{
    switch (format)
    {
    case EPixelFormat::R8G8B8A8Unorm:
        return VK_FORMAT_R8G8B8A8_UNORM;
    case EPixelFormat::R8G8B8A8Srgb:
        return VK_FORMAT_R8G8B8A8_SRGB;
    case EPixelFormat::B8G8A8R8Unorm:
        return VK_FORMAT_B8G8R8A8_UNORM;
    case EPixelFormat::B8G8A8R8Srgb:
        return VK_FORMAT_B8G8R8A8_SRGB;
    // Floating-point formats
    case EPixelFormat::R16f:
        return VK_FORMAT_R16_SFLOAT;
    case EPixelFormat::RG16f:
        return VK_FORMAT_R16G16_SFLOAT;
    case EPixelFormat::Rgba16f:
        return VK_FORMAT_R16G16B16A16_SFLOAT;
    case EPixelFormat::R32f:
        return VK_FORMAT_R32_SFLOAT;
    case EPixelFormat::RG32f:
        return VK_FORMAT_R32G32_SFLOAT;
    case EPixelFormat::Rgba32f:
        return VK_FORMAT_R32G32B32A32_SFLOAT;
    // Depth/stencil formats
    case EPixelFormat::Depth32:
        return VK_FORMAT_D32_SFLOAT;
    case EPixelFormat::Stencil8:
        return VK_FORMAT_S8_UINT;
    case EPixelFormat::Depth24Stencil8:
        return VK_FORMAT_D24_UNORM_S8_UINT;
    case EPixelFormat::Depth32Stencil8:
        return VK_FORMAT_D32_SFLOAT_S8_UINT;
    default:
        HS_LOG(error, "Unsupported pixel format: %d", static_cast<int>(format));
    }
    return VK_FORMAT_UNDEFINED;
}

EPixelFormat VulkanUtility::FromPixelFormat(VkFormat format)
{
    switch (format)
    {
    case VK_FORMAT_R8G8B8A8_UNORM:
        return EPixelFormat::R8G8B8A8Unorm;
    case VK_FORMAT_R8G8B8A8_SRGB:
        return EPixelFormat::R8G8B8A8Srgb;
    case VK_FORMAT_B8G8R8A8_UNORM:
        return EPixelFormat::B8G8A8R8Unorm;
    case VK_FORMAT_B8G8R8A8_SRGB:
        return EPixelFormat::B8G8A8R8Srgb;
    // Floating-point formats
    case VK_FORMAT_R16_SFLOAT:
        return EPixelFormat::R16f;
    case VK_FORMAT_R16G16_SFLOAT:
        return EPixelFormat::RG16f;
    case VK_FORMAT_R16G16B16A16_SFLOAT:
        return EPixelFormat::Rgba16f;
    case VK_FORMAT_R32_SFLOAT:
        return EPixelFormat::R32f;
    case VK_FORMAT_R32G32_SFLOAT:
        return EPixelFormat::RG32f;
    case VK_FORMAT_R32G32B32A32_SFLOAT:
        return EPixelFormat::Rgba32f;
    // Depth/stencil formats
    case VK_FORMAT_D32_SFLOAT:
        return EPixelFormat::Depth32;
    case VK_FORMAT_S8_UINT:
        return EPixelFormat::Stencil8;
    case VK_FORMAT_D24_UNORM_S8_UINT:
        return EPixelFormat::Depth24Stencil8;
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
        return EPixelFormat::Depth32Stencil8;
    default:
        HS_LOG(error, "Unsupported VkFormat: %d", static_cast<int>(format));
        return EPixelFormat::Invalid;
    }
}

VkFormat VulkanUtility::ToVertexFormat(EVertexFormat format)
{
    switch (format)
    {
    case EVertexFormat::Float:  return VK_FORMAT_R32_SFLOAT;
    case EVertexFormat::Float2: return VK_FORMAT_R32G32_SFLOAT;
    case EVertexFormat::Float3: return VK_FORMAT_R32G32B32_SFLOAT;
    case EVertexFormat::Float4: return VK_FORMAT_R32G32B32A32_SFLOAT;
    case EVertexFormat::Half:   return VK_FORMAT_R16_SFLOAT;
    case EVertexFormat::Half2:  return VK_FORMAT_R16G16_SFLOAT;
    case EVertexFormat::Half3:  return VK_FORMAT_R16G16B16_SFLOAT;
    case EVertexFormat::Half4:  return VK_FORMAT_R16G16B16A16_SFLOAT;
    default:
        HS_LOG(error, "Unsupported vertex format: %d", static_cast<int>(format));
    }
    return VK_FORMAT_UNDEFINED;
}

EVertexFormat VulkanUtility::FromVertexFormat(VkFormat format)
{
    switch (format)
    {
    case VK_FORMAT_R32_SFLOAT:          return EVertexFormat::Float;
    case VK_FORMAT_R32G32_SFLOAT:       return EVertexFormat::Float2;
    case VK_FORMAT_R32G32B32_SFLOAT:    return EVertexFormat::Float3;
    case VK_FORMAT_R32G32B32A32_SFLOAT: return EVertexFormat::Float4;
    case VK_FORMAT_R16_SFLOAT:          return EVertexFormat::Half;
    case VK_FORMAT_R16G16_SFLOAT:       return EVertexFormat::Half2;
    case VK_FORMAT_R16G16B16_SFLOAT:    return EVertexFormat::Half3;
    case VK_FORMAT_R16G16B16A16_SFLOAT: return EVertexFormat::Half4;
    default:
        HS_LOG(error, "Unsupported VkFormat for vertex format: %d", static_cast<int>(format));
    }
    return EVertexFormat::Float;
}

VkAttachmentLoadOp VulkanUtility::ToLoadOp(ELoadAction action)
{
    switch (action)
    {
    case ELoadAction::DontCare:
        return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    case ELoadAction::Load:
        return VK_ATTACHMENT_LOAD_OP_LOAD;
    case ELoadAction::Clear:
        return VK_ATTACHMENT_LOAD_OP_CLEAR;
    default:
        HS_LOG(error, "Unsupported load action: %d", static_cast<int>(action));
    }
    return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
}

ELoadAction VulkanUtility::FromLoadOp(VkAttachmentLoadOp action)
{
    switch (action)
    {
    case VK_ATTACHMENT_LOAD_OP_DONT_CARE:
        return ELoadAction::DontCare;
    case VK_ATTACHMENT_LOAD_OP_LOAD:
        return ELoadAction::Load;
    case VK_ATTACHMENT_LOAD_OP_CLEAR:
        return ELoadAction::Clear;
    default:
        HS_LOG(error, "Unsupported VkAttachmentLoadOp: %d", static_cast<int>(action));
    }
    return ELoadAction::Invalid;
}

VkAttachmentStoreOp VulkanUtility::ToStoreOp(EStoreAction action)
{
    switch (action)
    {
    case EStoreAction::DontCare:
        return VK_ATTACHMENT_STORE_OP_DONT_CARE;
    case EStoreAction::Store:
        return VK_ATTACHMENT_STORE_OP_STORE;
    default:
        HS_LOG(error, "Unsupported store action: %d", static_cast<int>(action));
    }
    return VK_ATTACHMENT_STORE_OP_DONT_CARE;
}

EStoreAction VulkanUtility::FromStoreOp(VkAttachmentStoreOp action)
{
    switch (action)
    {
    case VK_ATTACHMENT_STORE_OP_DONT_CARE:
        return EStoreAction::DontCare;
    case VK_ATTACHMENT_STORE_OP_STORE:
        return EStoreAction::Store;
    default:
        HS_LOG(error, "Unsupported VkAttachmentStoreOp: %d", static_cast<int>(action));
    }
    return EStoreAction::Invalid;
}

VkViewport VulkanUtility::ToViewport(Viewport vp)
{
    VkViewport viewport{};
    viewport.x        = vp.x;
    viewport.y        = vp.y;
    viewport.width    = vp.width;
    viewport.height   = vp.height;
    viewport.minDepth = vp.zNear;
    viewport.maxDepth = vp.zFar;
    return viewport;
}

Viewport VulkanUtility::FromViewport(VkViewport vp)
{
    Viewport viewport{};
    viewport.x      = vp.x;
    viewport.y      = vp.y;
    viewport.width  = vp.width;
    viewport.height = vp.height;
    viewport.zNear  = vp.minDepth;
    viewport.zFar   = vp.maxDepth;
    return viewport;
}

VkImageUsageFlags VulkanUtility::ToTextureUsage(ETextureUsage usage)
{
    VkImageUsageFlags flags = 0;
    if ((usage & ETextureUsage::TransferDestination) != 0)
        flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    if ((usage & ETextureUsage::TransferSource) != 0)
        flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    if ((usage & ETextureUsage::Sampled) != 0)
        flags |= VK_IMAGE_USAGE_SAMPLED_BIT;

    if ((usage & ETextureUsage::Storage) != 0)
        flags |= VK_IMAGE_USAGE_STORAGE_BIT;

    if ((usage & ETextureUsage::ColorAttachment) != 0)
        flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if ((usage & ETextureUsage::DepthStencilAttachment) != 0)
        flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    if ((usage & ETextureUsage::TransientAttachment) != 0)
        flags |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;

    if ((usage & ETextureUsage::InputAttachment) != 0)
        flags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;

    return flags;
}

ETextureUsage VulkanUtility::FromTextureUsage(VkImageUsageFlags usage)
{
    ETextureUsage flags = ETextureUsage::Unknown;
    if ((usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT) != 0)
        flags |= ETextureUsage::TransferDestination;

    if ((usage & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) != 0)
        flags |= ETextureUsage::TransferSource;

    if ((usage & VK_IMAGE_USAGE_SAMPLED_BIT) != 0)
        flags |= ETextureUsage::Sampled;

    if ((usage & VK_IMAGE_USAGE_STORAGE_BIT) != 0)
        flags |= ETextureUsage::Storage;

    if ((usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) != 0)
        flags |= ETextureUsage::ColorAttachment;

    if ((usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0)
        flags |= ETextureUsage::DepthStencilAttachment;

    if ((usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) != 0)
        flags |= ETextureUsage::TransientAttachment;

    if ((usage & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT) != 0)
        flags |= ETextureUsage::InputAttachment;

    return flags;
}

VkImageType VulkanUtility::ToImageType(ETextureType type)
{
    switch (type)
    {
    case ETextureType::Tex1D:
    case ETextureType::Tex1DArray:
        return VK_IMAGE_TYPE_1D;
    case ETextureType::Tex2D:
    case ETextureType::Tex2DArray:
    case ETextureType::TexCube: // Cube maps use 2D image type with 6 array layers
        return VK_IMAGE_TYPE_2D;
    case ETextureType::Tex3D:
        return VK_IMAGE_TYPE_3D;
    default:
        HS_LOG(error, "Unsupported texture type: %d", static_cast<int>(type));
    }
    return VK_IMAGE_TYPE_2D;
}

ETextureType VulkanUtility::FromImageType(VkImageType type)
{
    switch (type)
    {
    case VK_IMAGE_TYPE_1D:
        return ETextureType::Tex1D;
    case VK_IMAGE_TYPE_2D:
        return ETextureType::Tex2D;
    case VK_IMAGE_TYPE_3D:
        return ETextureType::Tex3D;
    default:
        HS_LOG(error, "Unsupported VkImageType: %d", static_cast<int>(type));
    }
    return ETextureType::Invalid;
}

VkImageViewType VulkanUtility::ToImageViewType(ETextureType type)
{
    switch (type)
    {
    case ETextureType::Tex1D:
        return VK_IMAGE_VIEW_TYPE_1D;
    case ETextureType::Tex1DArray:
        return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
    case ETextureType::Tex2D:
        return VK_IMAGE_VIEW_TYPE_2D;
    case ETextureType::Tex2DArray:
        return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    case ETextureType::TexCube:
        return VK_IMAGE_VIEW_TYPE_CUBE;
    case ETextureType::Tex3D:
        return VK_IMAGE_VIEW_TYPE_3D;
    default:
        HS_LOG(error, "Unsupported texture view type: %d", static_cast<int>(type));
    }
    return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
}

ETextureType VulkanUtility::FromImageViewType(VkImageViewType type)
{
    switch (type)
    {
    case VK_IMAGE_VIEW_TYPE_1D:
        return ETextureType::Tex1D;
    case VK_IMAGE_VIEW_TYPE_1D_ARRAY:
        return ETextureType::Tex1DArray;
    case VK_IMAGE_VIEW_TYPE_2D:
        return ETextureType::Tex2D;
    case VK_IMAGE_VIEW_TYPE_2D_ARRAY:
        return ETextureType::Tex2DArray;
    case VK_IMAGE_VIEW_TYPE_CUBE:
        return ETextureType::TexCube;
    case VK_IMAGE_VIEW_TYPE_3D:
        return ETextureType::Tex3D;
    default:
        HS_LOG(error, "Unsupported VkImageViewType: %d", static_cast<int>(type));
    }
    return ETextureType::Invalid;
}

uint32 VulkanUtility::GetTextureMipLevelCount(const TextureInfo& info)
{
    return info.mipLevel == 0 ? 1 : info.mipLevel;
}

uint32 VulkanUtility::GetTextureLayerCount(const TextureInfo& info)
{
    HS_ASSERT(info.arrayLength > 0, "TextureInfo.arrayLength must be greater than 0");
    return info.type == ETextureType::TexCube ? 6 : info.arrayLength;
}

VkImageAspectFlags VulkanUtility::GetImageAspectMask(const TextureInfo& info)
{
    if (!info.isDepthStencilBuffer)
    {
        return VK_IMAGE_ASPECT_COLOR_BIT;
    }

    VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    if (info.format == EPixelFormat::Depth24Stencil8 || info.format == EPixelFormat::Depth32Stencil8)
    {
        aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
    return aspectMask;
}

VkImageCreateInfo VulkanUtility::MakeTextureCreateInfo(const TextureInfo& info, bool useAlias)
{
    VkImageCreateInfo imageCreateInfo{};
    imageCreateInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType     = ToImageType(info.type);
    imageCreateInfo.format        = ToPixelFormat(info.format);
    imageCreateInfo.usage         = ToTextureUsage(info.usage);
    imageCreateInfo.extent.width  = info.extent.width;
    imageCreateInfo.extent.height = info.extent.height;
    imageCreateInfo.extent.depth  = (info.type == ETextureType::Tex3D) ? info.extent.depth : 1;
    imageCreateInfo.arrayLayers   = GetTextureLayerCount(info);
    imageCreateInfo.mipLevels     = GetTextureMipLevelCount(info);
    imageCreateInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling        = (imageCreateInfo.imageType == VK_IMAGE_TYPE_1D) ? VK_IMAGE_TILING_LINEAR : VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.flags         = useAlias ? VK_IMAGE_CREATE_ALIAS_BIT : 0;
    if (info.type == ETextureType::TexCube)
    {
        imageCreateInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    }
    return imageCreateInfo;
}

void VulkanUtility::TransitionImageLayout(
    VkCommandBuffer cmdBufferVk,
    VulkanTexture* textureVK,
    VkImageLayout oldLayout,
    VkImageLayout newLayout,
    VkPipelineStageFlags srcStage,
    VkAccessFlags srcAccess,
    VkPipelineStageFlags dstStage,
    VkAccessFlags dstAccess,
    VkImageAspectFlags aspectMask)
{
    if (VK_NULL_HANDLE == textureVK->handle || oldLayout != textureVK->layoutVk)
    {
        HS_LOG(error, "Invalid image handle for layout transition.");
        return;
    }

    VkImageMemoryBarrier barrier{
        .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask       = srcAccess,
        .dstAccessMask       = dstAccess,
        .oldLayout           = oldLayout,
        .newLayout           = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image               = textureVK->handle,
        .subresourceRange    = {
               .aspectMask     = aspectMask,
               .baseMipLevel   = 0,
               .levelCount     = VulkanUtility::GetTextureMipLevelCount(textureVK->info),
               .baseArrayLayer = 0,
               .layerCount     = VulkanUtility::GetTextureLayerCount(textureVK->info),
        }};

    vkCmdPipelineBarrier(
        cmdBufferVk,
        srcStage,
        dstStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier);

    textureVK->layoutVk = newLayout;
}

uint32 VulkanUtility::GetMemoryTypeIndex(VkPhysicalDevice physicalDevice, uint32 typeBits, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);
    for (uint32 i = 0; i < deviceMemoryProperties.memoryTypeCount; i++)
    {
        if ((typeBits & 1) == 1)
        {
            if ((deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }
        typeBits >>= 1;
    }

    return 0;
}

VkBlendFactor VulkanUtility::ToBlendFactor(EBlendFactor factor)
{
    switch (factor)
    {
    case EBlendFactor::Zero:
        return VK_BLEND_FACTOR_ZERO;
    case EBlendFactor::One:
        return VK_BLEND_FACTOR_ONE;
    case EBlendFactor::SrcColor:
        return VK_BLEND_FACTOR_SRC_COLOR;
    case EBlendFactor::OneMinusSrcColor:
        return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    case EBlendFactor::DstColor:
        return VK_BLEND_FACTOR_DST_COLOR;
    case EBlendFactor::OneMinusDstColor:
        return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
    case EBlendFactor::SrcAlpha:
        return VK_BLEND_FACTOR_SRC_ALPHA;
    case EBlendFactor::OneMinusSrcAlpha:
        return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    case EBlendFactor::DstAlpha:
        return VK_BLEND_FACTOR_DST_ALPHA;
    case EBlendFactor::OneMinusDstAlpha:
        return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
    case EBlendFactor::SrcAlphaSaturate:
        return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
    case EBlendFactor::Src1Color:
        return VK_BLEND_FACTOR_SRC1_COLOR;
    case EBlendFactor::OneMinusSrc1Color:
        return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
    case EBlendFactor::Src1Alpha:
        return VK_BLEND_FACTOR_SRC1_ALPHA;
    case EBlendFactor::OneMinusSrc1Alpha:
        return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
    default:
        HS_LOG(error, "Unsupported blend factor: %d", static_cast<int>(factor));
    }
    return VK_BLEND_FACTOR_ZERO;
}

EBlendFactor VulkanUtility::FromBlendFactor(VkBlendFactor factor)
{
    switch (factor)
    {
    case VK_BLEND_FACTOR_ZERO:
        return EBlendFactor::Zero;
    case VK_BLEND_FACTOR_ONE:
        return EBlendFactor::One;
    case VK_BLEND_FACTOR_SRC_COLOR:
        return EBlendFactor::SrcColor;
    case VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR:
        return EBlendFactor::OneMinusSrcColor;
    case VK_BLEND_FACTOR_DST_COLOR:
        return EBlendFactor::DstColor;
    case VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR:
        return EBlendFactor::OneMinusDstColor;
    case VK_BLEND_FACTOR_SRC_ALPHA:
        return EBlendFactor::SrcAlpha;
    case VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:
        return EBlendFactor::OneMinusSrcAlpha;
    case VK_BLEND_FACTOR_DST_ALPHA:
        return EBlendFactor::DstAlpha;
    case VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA:
        return EBlendFactor::OneMinusDstAlpha;
    case VK_BLEND_FACTOR_SRC_ALPHA_SATURATE:
        return EBlendFactor::SrcAlphaSaturate;
    case VK_BLEND_FACTOR_SRC1_COLOR:
        return EBlendFactor::Src1Color;
    case VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR:
        return EBlendFactor::OneMinusSrc1Color;
    case VK_BLEND_FACTOR_SRC1_ALPHA:
        return EBlendFactor::Src1Alpha;
    case VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA:
        return EBlendFactor::OneMinusSrc1Alpha;
    default:
        HS_LOG(error, "Unsupported VkBlendFactor: %d", static_cast<int>(factor));
    }
    return EBlendFactor::Invalid;
}

VkBlendOp VulkanUtility::ToBlendOp(EBlendOp operation)
{
    switch (operation)
    {
    case EBlendOp::Add:
        return VK_BLEND_OP_ADD;
    case EBlendOp::Subtract:
        return VK_BLEND_OP_SUBTRACT;
    case EBlendOp::ReverseSubtract:
        return VK_BLEND_OP_REVERSE_SUBTRACT;
    case EBlendOp::Min:
        return VK_BLEND_OP_MIN;
    case EBlendOp::Max:
        return VK_BLEND_OP_MAX;
    default:
        HS_LOG(error, "Unsupported blend operation: %d", static_cast<int>(operation));
    }
    return VK_BLEND_OP_ADD;
}

EBlendOp VulkanUtility::FromBlendOp(VkBlendOp operation)
{
    switch (operation)
    {
    case VK_BLEND_OP_ADD:
        return EBlendOp::Add;
    case VK_BLEND_OP_SUBTRACT:
        return EBlendOp::Subtract;
    case VK_BLEND_OP_REVERSE_SUBTRACT:
        return EBlendOp::ReverseSubtract;
    case VK_BLEND_OP_MIN:
        return EBlendOp::Min;
    case VK_BLEND_OP_MAX:
        return EBlendOp::Max;
    default:
        HS_LOG(error, "Unsupported VkBlendOp: %d", static_cast<int>(operation));
    }
    return EBlendOp::Invalid;
}

VkCompareOp VulkanUtility::ToCompareOp(ECompareOp compareOp)
{
    switch (compareOp)
    {
    case ECompareOp::Never:
        return VK_COMPARE_OP_NEVER;
    case ECompareOp::Less:
        return VK_COMPARE_OP_LESS;
    case ECompareOp::Equal:
        return VK_COMPARE_OP_EQUAL;
    case ECompareOp::LessOrEqual:
        return VK_COMPARE_OP_LESS_OR_EQUAL;
    case ECompareOp::Greater:
        return VK_COMPARE_OP_GREATER;
    case ECompareOp::NotEqual:
        return VK_COMPARE_OP_NOT_EQUAL;
    case ECompareOp::GreaterOrEqual:
        return VK_COMPARE_OP_GREATER_OR_EQUAL;
    case ECompareOp::Always:
        return VK_COMPARE_OP_ALWAYS;
    default:
        HS_LOG(error, "Unsupported compare operation: %d", static_cast<int>(compareOp));
    }
    return VK_COMPARE_OP_NEVER;
}

ECompareOp VulkanUtility::FromCompareOp(VkCompareOp compareOp)
{
    switch (compareOp)
    {
    case VK_COMPARE_OP_NEVER:
        return ECompareOp::Never;
    case VK_COMPARE_OP_LESS:
        return ECompareOp::Less;
    case VK_COMPARE_OP_EQUAL:
        return ECompareOp::Equal;
    case VK_COMPARE_OP_LESS_OR_EQUAL:
        return ECompareOp::LessOrEqual;
    case VK_COMPARE_OP_GREATER:
        return ECompareOp::Greater;
    case VK_COMPARE_OP_NOT_EQUAL:
        return ECompareOp::NotEqual;
    case VK_COMPARE_OP_GREATER_OR_EQUAL:
        return ECompareOp::GreaterOrEqual;
    case VK_COMPARE_OP_ALWAYS:
        return ECompareOp::Always;
    default:
        HS_LOG(error, "Unsupported VkCompareOp: %d", static_cast<int>(compareOp));
    }
    return ECompareOp::Never;
}

VkStencilOp VulkanUtility::ToStencilOp(EStencilOp stencilOp)
{
    switch (stencilOp)
    {
    case EStencilOp::Keep:              return VK_STENCIL_OP_KEEP;
    case EStencilOp::Zero:              return VK_STENCIL_OP_ZERO;
    case EStencilOp::Replace:           return VK_STENCIL_OP_REPLACE;
    case EStencilOp::IncrementAndClamp: return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
    case EStencilOp::DecrementAndClamp: return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
    case EStencilOp::Invert:            return VK_STENCIL_OP_INVERT;
    case EStencilOp::IncrementAndWrap:  return VK_STENCIL_OP_INCREMENT_AND_WRAP;
    case EStencilOp::DecrementAndWrap:  return VK_STENCIL_OP_DECREMENT_AND_WRAP;
    default:
        HS_LOG(error, "Unsupported stencil operation: %d", static_cast<int>(stencilOp));
    }
    return VK_STENCIL_OP_KEEP;
}

EStencilOp VulkanUtility::FromStencilOp(VkStencilOp stencilOp)
{
    switch (stencilOp)
    {
    case VK_STENCIL_OP_KEEP:                return EStencilOp::Keep;
    case VK_STENCIL_OP_ZERO:                return EStencilOp::Zero;
    case VK_STENCIL_OP_REPLACE:             return EStencilOp::Replace;
    case VK_STENCIL_OP_INCREMENT_AND_CLAMP: return EStencilOp::IncrementAndClamp;
    case VK_STENCIL_OP_DECREMENT_AND_CLAMP: return EStencilOp::DecrementAndClamp;
    case VK_STENCIL_OP_INVERT:              return EStencilOp::Invert;
    case VK_STENCIL_OP_INCREMENT_AND_WRAP:  return EStencilOp::IncrementAndWrap;
    case VK_STENCIL_OP_DECREMENT_AND_WRAP:  return EStencilOp::DecrementAndWrap;
    default:
        HS_LOG(error, "Unsupported VkStencilOp: %d", static_cast<int>(stencilOp));
    }
    return EStencilOp::Keep;
}

VkLogicOp VulkanUtility::ToLogicOp(ELogicOp logicOp)
{
    switch (logicOp)
    {
    case ELogicOp::Clear:        return VK_LOGIC_OP_CLEAR;
    case ELogicOp::And:          return VK_LOGIC_OP_AND;
    case ELogicOp::AndReverse:   return VK_LOGIC_OP_AND_REVERSE;
    case ELogicOp::Copy:         return VK_LOGIC_OP_COPY;
    case ELogicOp::AndInverted:  return VK_LOGIC_OP_AND_INVERTED;
    case ELogicOp::NoOp:         return VK_LOGIC_OP_NO_OP;
    case ELogicOp::Xor:          return VK_LOGIC_OP_XOR;
    case ELogicOp::Or:           return VK_LOGIC_OP_OR;
    case ELogicOp::Nor:          return VK_LOGIC_OP_NOR;
    case ELogicOp::Equivalent:   return VK_LOGIC_OP_EQUIVALENT;
    case ELogicOp::Invert:       return VK_LOGIC_OP_INVERT;
    case ELogicOp::OrReverse:    return VK_LOGIC_OP_OR_REVERSE;
    case ELogicOp::CopyInverted: return VK_LOGIC_OP_COPY_INVERTED;
    case ELogicOp::OrInverted:   return VK_LOGIC_OP_OR_INVERTED;
    case ELogicOp::Nand:         return VK_LOGIC_OP_NAND;
    case ELogicOp::Set:          return VK_LOGIC_OP_SET;
    default:
        HS_LOG(error, "Unsupported VkCompareOp: %d", static_cast<int>(logicOp));
    };
    return VK_LOGIC_OP_NO_OP;
}

ELogicOp VulkanUtility::FromLogicOp(VkLogicOp logicOp)
{
    switch (logicOp)
    {
    case VK_LOGIC_OP_CLEAR:         return ELogicOp::Clear;
    case VK_LOGIC_OP_AND:           return ELogicOp::And;
    case VK_LOGIC_OP_AND_REVERSE:   return ELogicOp::AndReverse;
    case VK_LOGIC_OP_COPY:          return ELogicOp::Copy;
    case VK_LOGIC_OP_AND_INVERTED:  return ELogicOp::AndInverted;
    case VK_LOGIC_OP_NO_OP:         return ELogicOp::NoOp;
    case VK_LOGIC_OP_XOR:           return ELogicOp::Xor;
    case VK_LOGIC_OP_OR:            return ELogicOp::Or;
    case VK_LOGIC_OP_NOR:           return ELogicOp::Nor;
    case VK_LOGIC_OP_EQUIVALENT:    return ELogicOp::Equivalent;
    case VK_LOGIC_OP_INVERT:        return ELogicOp::Invert;
    case VK_LOGIC_OP_OR_REVERSE:    return ELogicOp::OrReverse;
    case VK_LOGIC_OP_COPY_INVERTED: return ELogicOp::CopyInverted;
    case VK_LOGIC_OP_OR_INVERTED:   return ELogicOp::OrInverted;
    case VK_LOGIC_OP_NAND:          return ELogicOp::Nand;
    case VK_LOGIC_OP_SET:           return ELogicOp::Set;
    default:
        HS_LOG(error, "Unsupported VkLogicOp: %d", static_cast<int>(logicOp));
    }
    return ELogicOp::NoOp;
}

VkFrontFace VulkanUtility::ToFrontFace(EFrontFace frontFace)
{
    switch (frontFace)
    {
    case EFrontFace::CounterClockwise:
        return VK_FRONT_FACE_COUNTER_CLOCKWISE;
    case EFrontFace::Clockwise:
        return VK_FRONT_FACE_CLOCKWISE;
    default:
        HS_LOG(error, "Unsupported front face: %d", static_cast<int>(frontFace));
    }
    return VK_FRONT_FACE_COUNTER_CLOCKWISE;
}

EFrontFace VulkanUtility::FromFrontFace(VkFrontFace frontFace)
{
    switch (frontFace)
    {
    case VK_FRONT_FACE_COUNTER_CLOCKWISE:
        return EFrontFace::CounterClockwise;
    case VK_FRONT_FACE_CLOCKWISE:
        return EFrontFace::Clockwise;
    default:
        HS_LOG(error, "Unsupported VkFrontFace: %d", static_cast<int>(frontFace));
    }
    return EFrontFace::CounterClockwise;
}

VkCullModeFlags VulkanUtility::ToCullMode(ECullMode cullMode)
{
    switch (cullMode)
    {
    case ECullMode::None:
        return VK_CULL_MODE_NONE;
    case ECullMode::Front:
        return VK_CULL_MODE_FRONT_BIT;
    case ECullMode::Back:
        return VK_CULL_MODE_BACK_BIT;
    case ECullMode::All:
        return VK_CULL_MODE_FRONT_AND_BACK;
    default:
        HS_LOG(error, "Unsupported cull mode: %d", static_cast<int>(cullMode));
    }
    return VK_CULL_MODE_NONE;
}

ECullMode VulkanUtility::FromCullMode(VkCullModeFlags cullMode)
{
    switch (cullMode)
    {
    case VK_CULL_MODE_NONE:
        return ECullMode::None;
    case VK_CULL_MODE_FRONT_BIT:
        return ECullMode::Front;
    case VK_CULL_MODE_BACK_BIT:
        return ECullMode::Back;
    case VK_CULL_MODE_FRONT_AND_BACK:
        return ECullMode::All;
    default:
        HS_LOG(error, "Unsupported VkCullModeFlags: %d", static_cast<int>(cullMode));
    }
    return ECullMode::None;
}

VkBufferUsageFlags VulkanUtility::ToBufferUsage(EBufferUsage usage)
{
    VkBufferUsageFlags flags = 0;
    if ((usage & EBufferUsage::Vertex) != static_cast<EBufferUsage>(0)) flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    if ((usage & EBufferUsage::Index) != static_cast<EBufferUsage>(0)) flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    if ((usage & EBufferUsage::Uniform) != static_cast<EBufferUsage>(0)) flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    if ((usage & EBufferUsage::StorageBuffer) != static_cast<EBufferUsage>(0)) flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    return flags;
}

EBufferUsage VulkanUtility::FromBufferUsage(VkBufferUsageFlags usage)
{
    EBufferUsage flags = EBufferUsage::Invalid;
    if (usage & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) flags = flags | EBufferUsage::Vertex;
    if (usage & VK_BUFFER_USAGE_INDEX_BUFFER_BIT) flags = flags | EBufferUsage::Index;
    if (usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) flags = flags | EBufferUsage::Uniform;
    if (usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) flags = flags | EBufferUsage::StorageBuffer;

    return flags;
}

// Note: These functions seem to have incorrect names in the header
// They should probably handle VkPolygonMode instead of VkPrimitiveTopology
// But implementing according to the function signature provided
VkPolygonMode VulkanUtility::ToPolygonMode(EPolygonMode polygonMode)
{
    // WARNING: Function name suggests VkPolygonMode but returns VkPrimitiveTopology
    // This might be a naming error in the header file
    switch (polygonMode)
    {
    case EPolygonMode::Fill:
        return VK_POLYGON_MODE_FILL; // Default topology for filled polygons
    case EPolygonMode::Line:
        return VK_POLYGON_MODE_LINE;
    case EPolygonMode::Point:
        return VK_POLYGON_MODE_POINT;
    default:
        HS_LOG(error, "Unsupported polygon mode: %d", static_cast<int>(polygonMode));
    }
    return VK_POLYGON_MODE_FILL;
}

EPolygonMode VulkanUtility::FromPolygonMode(VkPolygonMode polygonMode)
{
    // WARNING: Function name suggests VkPolygonMode but accepts VkPrimitiveTopology
    // This might be a naming error in the header file
    switch (polygonMode)
    {
    case VK_POLYGON_MODE_FILL:
        return EPolygonMode::Fill;
    case VK_POLYGON_MODE_LINE:
        return EPolygonMode::Line;
    case VK_POLYGON_MODE_POINT:
        return EPolygonMode::Point;
    default:
        HS_LOG(error, "Unsupported VkPrimitiveTopology: %d", static_cast<int>(polygonMode));
    }
    return EPolygonMode::Fill;
}

VkShaderStageFlagBits VulkanUtility::ToShaderStageFlags(EShaderStage stage)
{
    VkShaderStageFlagBits flags = static_cast<VkShaderStageFlagBits>(0);
    if ((stage & EShaderStage::Vertex) != EShaderStage::None)
    {
        flags = static_cast<VkShaderStageFlagBits>(flags | VK_SHADER_STAGE_VERTEX_BIT);
    }
    if ((stage & EShaderStage::Fragment) != EShaderStage::None)
    {
        flags = static_cast<VkShaderStageFlagBits>(flags | VK_SHADER_STAGE_FRAGMENT_BIT);
    }
    if ((stage & EShaderStage::Compute) != EShaderStage::None)
    {
        flags = static_cast<VkShaderStageFlagBits>(flags | VK_SHADER_STAGE_COMPUTE_BIT);
    }
    if ((stage & EShaderStage::Geometry) != EShaderStage::None)
    {
        flags = static_cast<VkShaderStageFlagBits>(flags | VK_SHADER_STAGE_GEOMETRY_BIT);
    }
    if ((stage & EShaderStage::Domain) != EShaderStage::None)
    {
        flags = static_cast<VkShaderStageFlagBits>(flags | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
    }
    if ((stage & EShaderStage::Hull) != EShaderStage::None)
    {
        flags = static_cast<VkShaderStageFlagBits>(flags | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
    }

    return flags;
}

EShaderStage VulkanUtility::FromShaderStageFlags(VkShaderStageFlagBits flags)
{
    EShaderStage stage = EShaderStage::None;
    if (flags & VK_SHADER_STAGE_VERTEX_BIT)
    {
        stage = stage | EShaderStage::Vertex;
    }
    if (flags & VK_SHADER_STAGE_FRAGMENT_BIT)
    {
        stage = stage | EShaderStage::Fragment;
    }
    if (flags & VK_SHADER_STAGE_COMPUTE_BIT)
    {
        stage = stage | EShaderStage::Compute;
    }
    if (flags & VK_SHADER_STAGE_GEOMETRY_BIT)
    {
        stage = stage | EShaderStage::Geometry;
    }
    if (flags & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT)
    {
        stage = stage | EShaderStage::Domain;
    }
    if (flags & VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)
    {
        stage = stage | EShaderStage::Hull;
    }

    return stage;
}

VkPrimitiveTopology VulkanUtility::ToPrimitiveTopology(EPrimitiveTopology topology)
{
    switch (topology)
    {
    case EPrimitiveTopology::PointList:
        return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    case EPrimitiveTopology::LineList:
        return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    case EPrimitiveTopology::LineStrip:
        return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    case EPrimitiveTopology::TriangleList:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    case EPrimitiveTopology::TriangleStrip:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    case EPrimitiveTopology::TriangleFan:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
    case EPrimitiveTopology::LineListWithAdjacency:
        return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
    case EPrimitiveTopology::LineStripWithAdjacency:
        return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
    case EPrimitiveTopology::TriangleListWithAdjacency:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
    case EPrimitiveTopology::TriangleStripWithAdjacency:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
    case EPrimitiveTopology::PatchList:
        return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
    default:
        HS_LOG(error, "Unsupported primitive topology: %d", static_cast<int>(topology));
    }
    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
}

EPrimitiveTopology VulkanUtility::FromPrimitiveTopology(VkPrimitiveTopology topology)
{
    switch (topology)
    {
    case VK_PRIMITIVE_TOPOLOGY_POINT_LIST:
        return EPrimitiveTopology::PointList;
    case VK_PRIMITIVE_TOPOLOGY_LINE_LIST:
        return EPrimitiveTopology::LineList;
    case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP:
        return EPrimitiveTopology::LineStrip;
    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
        return EPrimitiveTopology::TriangleList;
    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
        return EPrimitiveTopology::TriangleStrip;
    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN:
        return EPrimitiveTopology::TriangleFan;
    case VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY:
        return EPrimitiveTopology::LineListWithAdjacency;
    case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY:
        return EPrimitiveTopology::LineStripWithAdjacency;
    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY:
        return EPrimitiveTopology::TriangleListWithAdjacency;
    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY:
        return EPrimitiveTopology::TriangleStripWithAdjacency;
    case VK_PRIMITIVE_TOPOLOGY_PATCH_LIST:
        return EPrimitiveTopology::PatchList;
    default:
        HS_LOG(error, "Unsupported VkPrimitiveTopology: %d", static_cast<int>(topology));
    }
    return EPrimitiveTopology::TriangleList;
}

VkSamplerAddressMode VulkanUtility::ToAddressMode(EAddressMode addressMode)
{
    switch (addressMode)
    {
    case EAddressMode::Repeat:
        return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    case EAddressMode::MirroredRepeat:
        return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    case EAddressMode::ClampToEdge:
        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    case EAddressMode::ClampToBorder:
        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    case EAddressMode::MirrorClampToEdge:
        return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
    default:
        HS_LOG(error, "Unsupported address mode: %d", static_cast<int>(addressMode));
    }
    return VK_SAMPLER_ADDRESS_MODE_REPEAT; // Default fallback
}

EAddressMode VulkanUtility::FromAddressMode(VkSamplerAddressMode addressMode)
{
    switch (addressMode)
    {
    case VK_SAMPLER_ADDRESS_MODE_REPEAT:
        return EAddressMode::Repeat;
    case VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT:
        return EAddressMode::MirroredRepeat;
    case VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE:
        return EAddressMode::ClampToEdge;
    case VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER:
        return EAddressMode::ClampToBorder;
    case VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE:
        return EAddressMode::MirrorClampToEdge;
    default:
        HS_LOG(error, "Unsupported VkSamplerAddressMode: %d", static_cast<int>(addressMode));
    }
    return EAddressMode::Repeat; // Default fallback
}

VkFilter VulkanUtility::ToFilter(EFilterMode filter)
{
    switch (filter)
    {
    case EFilterMode::Nearest:
        return VK_FILTER_NEAREST;
    case EFilterMode::Linear:
        return VK_FILTER_LINEAR;
    default:
        HS_LOG(error, "Unsupported filter mode: %d", static_cast<int>(filter));
    }
    return VK_FILTER_NEAREST; // Default fallback
}

EFilterMode VulkanUtility::FromFilter(VkFilter filter)
{
    switch (filter)
    {
    case VK_FILTER_NEAREST:
        return EFilterMode::Nearest;
    case VK_FILTER_LINEAR:
        return EFilterMode::Linear;
    default:
        HS_LOG(error, "Unsupported VkFilter: %d", static_cast<int>(filter));
    }
    return EFilterMode::Nearest; // Default fallback
}

VkSamplerMipmapMode VulkanUtility::ToMipmapMode(EFilterMode mipmapMode)
{
    switch (mipmapMode)
    {
    case EFilterMode::Nearest:
        return VK_SAMPLER_MIPMAP_MODE_NEAREST;
    case EFilterMode::Linear:
        return VK_SAMPLER_MIPMAP_MODE_LINEAR;
    default:
        HS_LOG(error, "Unsupported mipmap mode: %d", static_cast<int>(mipmapMode));
    }
    return VK_SAMPLER_MIPMAP_MODE_NEAREST; // Default fallback
}

EFilterMode VulkanUtility::FromMipmapMode(VkSamplerMipmapMode mipmapMode)
{
    switch (mipmapMode)
    {
    case VK_SAMPLER_MIPMAP_MODE_NEAREST:
        return EFilterMode::Nearest;
    case VK_SAMPLER_MIPMAP_MODE_LINEAR:
        return EFilterMode::Linear;
    default:
        HS_LOG(error, "Unsupported VkSamplerMipmapMode: %d", static_cast<int>(mipmapMode));
    }
    return EFilterMode::Nearest; // Default fallback
}

const char* VulkanUtility::ToString(VkResult result)
{
    switch (result)
    {
    case VK_SUCCESS:
        return "VK_SUCCESS";
    case VK_NOT_READY:
        return "VK_NOT_READY";
    case VK_TIMEOUT:
        return "VK_TIMEOUT";
    case VK_EVENT_SET:
        return "VK_EVENT_SET";
    case VK_EVENT_RESET:
        return "VK_EVENT_RESET";
    case VK_INCOMPLETE:
        return "VK_INCOMPLETE";
    case VK_ERROR_OUT_OF_HOST_MEMORY:
        return "VK_ERROR_OUT_OF_HOST_MEMORY";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
    case VK_ERROR_INITIALIZATION_FAILED:
        return "VK_ERROR_INITIALIZATION_FAILED";
    case VK_ERROR_DEVICE_LOST:
        return "VK_ERROR_DEVICE_LOST";
    case VK_ERROR_MEMORY_MAP_FAILED:
        return "VK_ERROR_MEMORY_MAP_FAILED";
    case VK_ERROR_LAYER_NOT_PRESENT:
        return "VK_ERROR_LAYER_NOT_PRESENT";
    case VK_ERROR_EXTENSION_NOT_PRESENT:
        return "VK_ERROR_EXTENSION_NOT_PRESENT";
    case VK_ERROR_FEATURE_NOT_PRESENT:
        return "VK_ERROR_FEATURE_NOT_PRESENT";
    case VK_ERROR_INCOMPATIBLE_DRIVER:
        return "VK_ERROR_INCOMPATIBLE_DRIVER";
    case VK_ERROR_TOO_MANY_OBJECTS:
        return "VK_ERROR_TOO_MANY_OBJECTS";
    case VK_ERROR_FORMAT_NOT_SUPPORTED:
        return "VK_ERROR_FORMAT_NOT_SUPPORTED";
    case VK_ERROR_FRAGMENTED_POOL:
        return "VK_ERROR_FRAGMENTED_POOL";
    case VK_ERROR_UNKNOWN:
        return "VK_ERROR_UNKNOWN";
    case VK_ERROR_OUT_OF_POOL_MEMORY:
        return "VK_ERROR_OUT_OF_POOL_MEMORY";
    case VK_ERROR_INVALID_EXTERNAL_HANDLE:
        return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
    case VK_ERROR_FRAGMENTATION:
        return "VK_ERROR_FRAGMENTATION";
    case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
        return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
    case VK_PIPELINE_COMPILE_REQUIRED:
        return "VK_PIPELINE_COMPILE_REQUIRED";
    case VK_ERROR_SURFACE_LOST_KHR:
        return "VK_ERROR_SURFACE_LOST_KHR";
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
        return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
    case VK_SUBOPTIMAL_KHR:
        return "VK_SUBOPTIMAL_KHR";
    case VK_ERROR_OUT_OF_DATE_KHR:
        return "VK_ERROR_OUT_OF_DATE_KHR";
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
        return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
    case VK_ERROR_VALIDATION_FAILED_EXT:
        return "VK_ERROR_VALIDATION_FAILED_EXT";
    case VK_ERROR_INVALID_SHADER_NV:
        return "VK_ERROR_INVALID_SHADER_NV";
    case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
        return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
    case VK_ERROR_NOT_PERMITTED_KHR:
        return "VK_ERROR_NOT_PERMITTED_KHR";
    case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
        return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
    case VK_THREAD_IDLE_KHR:
        return "VK_THREAD_IDLE_KHR";
    case VK_THREAD_DONE_KHR:
        return "VK_THREAD_DONE_KHR";
    case VK_OPERATION_DEFERRED_KHR:
        return "VK_OPERATION_DEFERRED_KHR";
    case VK_OPERATION_NOT_DEFERRED_KHR:
        return "VK_OPERATION_NOT_DEFERRED_KHR";
    default:
        return "Unhandled VkResult";
    }
}

HS_NS_END
