#include "Engine/RHI/Vulkan/RHIUtilityVulkan.h"

HS_NS_BEGIN

VkFormat RHIUtilityVulkan::ToPixelFormat(EPixelFormat format)
{
    switch (format)
    {
    case EPixelFormat::R8G8B8A8_UNORM:
        return VK_FORMAT_R8G8B8A8_UNORM;
    case EPixelFormat::R8G8B8A8_SRGB:
        return VK_FORMAT_R8G8B8A8_SRGB;
    case EPixelFormat::B8G8A8R8_UNORM:
        return VK_FORMAT_B8G8R8A8_UNORM;
    case EPixelFormat::B8G8A8R8_SRGB:
        return VK_FORMAT_B8G8R8A8_SRGB;
    case EPixelFormat::DEPTH32:
        return VK_FORMAT_D32_SFLOAT;
    case EPixelFormat::STENCIL8:
        return VK_FORMAT_S8_UINT;
    case EPixelFormat::DEPTH24_STENCIL8:
        return VK_FORMAT_D24_UNORM_S8_UINT;
    case EPixelFormat::DEPTH32_STENCIL8:
        return VK_FORMAT_D32_SFLOAT_S8_UINT;
    default:
        HS_LOG(error, "Unsupported pixel format: %d", static_cast<int>(format));
    }
    return VK_FORMAT_UNDEFINED;
}

EPixelFormat RHIUtilityVulkan::FromPixelFormat(VkFormat format)
{
    switch (format)
    {
    case VK_FORMAT_R8G8B8A8_UNORM:
        return EPixelFormat::R8G8B8A8_UNORM;
    case VK_FORMAT_R8G8B8A8_SRGB:
        return EPixelFormat::R8G8B8A8_SRGB;
    case VK_FORMAT_B8G8R8A8_UNORM:
        return EPixelFormat::B8G8A8R8_UNORM;
    case VK_FORMAT_B8G8R8A8_SRGB:
        return EPixelFormat::B8G8A8R8_SRGB;
    case VK_FORMAT_D32_SFLOAT:
        return EPixelFormat::DEPTH32;
    case VK_FORMAT_S8_UINT:
        return EPixelFormat::STENCIL8;
    case VK_FORMAT_D24_UNORM_S8_UINT:
        return EPixelFormat::DEPTH24_STENCIL8;
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
        return EPixelFormat::DEPTH32_STENCIL8;
    default:
        HS_LOG(error, "Unsupported VkFormat: %d", static_cast<int>(format));
        return EPixelFormat::INVALID;
    }
}

VkAttachmentLoadOp RHIUtilityVulkan::ToLoadAction(ELoadAction action)
{
    switch (action)
    {
    case ELoadAction::DONT_CARE:
        return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    case ELoadAction::LOAD:
        return VK_ATTACHMENT_LOAD_OP_LOAD;
    case ELoadAction::CLEAR:
        return VK_ATTACHMENT_LOAD_OP_CLEAR;
    default:
        HS_LOG(error, "Unsupported load action: %d", static_cast<int>(action));
        return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    }
}

ELoadAction RHIUtilityVulkan::FromLoadAction(VkAttachmentLoadOp action)
{
    switch (action)
    {
    case VK_ATTACHMENT_LOAD_OP_DONT_CARE:
        return ELoadAction::DONT_CARE;
    case VK_ATTACHMENT_LOAD_OP_LOAD:
        return ELoadAction::LOAD;
    case VK_ATTACHMENT_LOAD_OP_CLEAR:
        return ELoadAction::CLEAR;
    default:
        HS_LOG(error, "Unsupported VkAttachmentLoadOp: %d", static_cast<int>(action));
        return ELoadAction::INVALID;
    }
}

VkAttachmentStoreOp RHIUtilityVulkan::ToStoreAction(EStoreAction action)
{
    switch (action)
    {
    case EStoreAction::DONT_CARE:
        return VK_ATTACHMENT_STORE_OP_DONT_CARE;
    case EStoreAction::STORE:
        return VK_ATTACHMENT_STORE_OP_STORE;
    default:
        HS_LOG(error, "Unsupported store action: %d", static_cast<int>(action));
        return VK_ATTACHMENT_STORE_OP_DONT_CARE;
    }
}

EStoreAction RHIUtilityVulkan::FromStoreAction(VkAttachmentStoreOp action)
{
    switch (action)
    {
    case VK_ATTACHMENT_STORE_OP_DONT_CARE:
        return EStoreAction::DONT_CARE;
    case VK_ATTACHMENT_STORE_OP_STORE:
        return EStoreAction::STORE;
    default:
        HS_LOG(error, "Unsupported VkAttachmentStoreOp: %d", static_cast<int>(action));
        return EStoreAction::INVALID;
    }
}

VkViewport RHIUtilityVulkan::ToViewport(Viewport vp)
{
    VkViewport viewport{};
    viewport.x = vp.x;
    viewport.y = vp.y;
    viewport.width = vp.width;
    viewport.height = vp.height;
    viewport.minDepth = vp.zNear;
    viewport.maxDepth = vp.zFar;
    return viewport;
}

Viewport RHIUtilityVulkan::FromViewport(VkViewport vp)
{
    Viewport viewport{};
    viewport.x = vp.x;
    viewport.y = vp.y;
    viewport.width = vp.width;
    viewport.height = vp.height;
    viewport.zNear = vp.minDepth;
    viewport.zFar = vp.maxDepth;
    return viewport;
}

VkImageUsageFlags RHIUtilityVulkan::ToTextureUsage(ETextureUsage usage)
{
    VkImageUsageFlags flags = 0;

    if ((usage & ETextureUsage::SHADER_READ) != static_cast<ETextureUsage>(0))
        flags |= VK_IMAGE_USAGE_SAMPLED_BIT;

    if ((usage & ETextureUsage::SHADER_WRITE) != static_cast<ETextureUsage>(0))
        flags |= VK_IMAGE_USAGE_STORAGE_BIT;

    if ((usage & ETextureUsage::RENDER_TARGET) != static_cast<ETextureUsage>(0))
        flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    if ((usage & ETextureUsage::PIXELFORMAT_VIEW) != static_cast<ETextureUsage>(0))
        flags |= VK_IMAGE_USAGE_SAMPLED_BIT;  // Additional view capability

    // Default transfer usage for most textures
    flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    return flags;
}

ETextureUsage RHIUtilityVulkan::FromTextureUsage(VkImageUsageFlags usage)
{
    ETextureUsage flags = ETextureUsage::UNKNOWN;

    if (usage & VK_IMAGE_USAGE_SAMPLED_BIT)
        flags = flags | ETextureUsage::SHADER_READ;

    if (usage & VK_IMAGE_USAGE_STORAGE_BIT)
        flags = flags | ETextureUsage::SHADER_WRITE;

    if (usage & (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT))
        flags = flags | ETextureUsage::RENDER_TARGET;

    return flags;
}

VkImageType RHIUtilityVulkan::ToTextureType(ETextureType type)
{
    switch (type)
    {
    case ETextureType::TEX_1D:
    case ETextureType::TEX_1D_ARRAY:
        return VK_IMAGE_TYPE_1D;
    case ETextureType::TEX_2D:
    case ETextureType::TEX_2D_ARRAY:
    case ETextureType::TEX_CUBE:
        return VK_IMAGE_TYPE_2D;
    default:
        HS_LOG(error, "Unsupported texture type: %d", static_cast<int>(type));
        return VK_IMAGE_TYPE_2D;
    }
}

ETextureType RHIUtilityVulkan::FromTextureType(VkImageType type)
{
    switch (type)
    {
    case VK_IMAGE_TYPE_1D:
        return ETextureType::TEX_1D;
    case VK_IMAGE_TYPE_2D:
        return ETextureType::TEX_2D;
    case VK_IMAGE_TYPE_3D:
        // 3D texture type not defined in enum, default to 2D
        return ETextureType::TEX_2D;
    default:
        HS_LOG(error, "Unsupported VkImageType: %d", static_cast<int>(type));
        return ETextureType::INVALID;
    }
}

VkBlendFactor RHIUtilityVulkan::ToBlendFactor(EBlendFactor factor)
{
    switch (factor)
    {
    case EBlendFactor::ZERO:
        return VK_BLEND_FACTOR_ZERO;
    case EBlendFactor::ONE:
        return VK_BLEND_FACTOR_ONE;
    case EBlendFactor::SRC_COLOR:
        return VK_BLEND_FACTOR_SRC_COLOR;
    case EBlendFactor::ONE_MINUS_SRC_COLOR:
        return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    case EBlendFactor::DST_COLOR:
        return VK_BLEND_FACTOR_DST_COLOR;
    case EBlendFactor::ONE_MINUS_DST_COLOR:
        return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
    case EBlendFactor::SRC_ALPHA:
        return VK_BLEND_FACTOR_SRC_ALPHA;
    case EBlendFactor::ONE_MINUS_SRC_ALPHA:
        return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    case EBlendFactor::DST_ALPHA:
        return VK_BLEND_FACTOR_DST_ALPHA;
    case EBlendFactor::ONE_MINUS_DST_ALPHA:
        return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
    case EBlendFactor::SRC_ALPHA_SATURATE:
        return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
    case EBlendFactor::SRC1_COLOR:
        return VK_BLEND_FACTOR_SRC1_COLOR;
    case EBlendFactor::ONE_MINUS_SRC1_COLOR:
        return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
    case EBlendFactor::SRC1_ALPHA:
        return VK_BLEND_FACTOR_SRC1_ALPHA;
    case EBlendFactor::ONE_MINUS_SRC1_ALPHA:
        return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
    default:
        HS_LOG(error, "Unsupported blend factor: %d", static_cast<int>(factor));
        return VK_BLEND_FACTOR_ZERO;
    }
}

EBlendFactor RHIUtilityVulkan::FromBlendFactor(VkBlendFactor factor)
{
    switch (factor)
    {
    case VK_BLEND_FACTOR_ZERO:
        return EBlendFactor::ZERO;
    case VK_BLEND_FACTOR_ONE:
        return EBlendFactor::ONE;
    case VK_BLEND_FACTOR_SRC_COLOR:
        return EBlendFactor::SRC_COLOR;
    case VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR:
        return EBlendFactor::ONE_MINUS_SRC_COLOR;
    case VK_BLEND_FACTOR_DST_COLOR:
        return EBlendFactor::DST_COLOR;
    case VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR:
        return EBlendFactor::ONE_MINUS_DST_COLOR;
    case VK_BLEND_FACTOR_SRC_ALPHA:
        return EBlendFactor::SRC_ALPHA;
    case VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:
        return EBlendFactor::ONE_MINUS_SRC_ALPHA;
    case VK_BLEND_FACTOR_DST_ALPHA:
        return EBlendFactor::DST_ALPHA;
    case VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA:
        return EBlendFactor::ONE_MINUS_DST_ALPHA;
    case VK_BLEND_FACTOR_SRC_ALPHA_SATURATE:
        return EBlendFactor::SRC_ALPHA_SATURATE;
    case VK_BLEND_FACTOR_SRC1_COLOR:
        return EBlendFactor::SRC1_COLOR;
    case VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR:
        return EBlendFactor::ONE_MINUS_SRC1_COLOR;
    case VK_BLEND_FACTOR_SRC1_ALPHA:
        return EBlendFactor::SRC1_ALPHA;
    case VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA:
        return EBlendFactor::ONE_MINUS_SRC1_ALPHA;
    default:
        HS_LOG(error, "Unsupported VkBlendFactor: %d", static_cast<int>(factor));
        return EBlendFactor::INVALID;
    }
}

VkBlendOp RHIUtilityVulkan::ToBlendOp(EBlendOp operation)
{
    switch (operation)
    {
    case EBlendOp::ADD:
        return VK_BLEND_OP_ADD;
    case EBlendOp::SUBTRACT:
        return VK_BLEND_OP_SUBTRACT;
    case EBlendOp::REVERSE_SUBTRACT:
        return VK_BLEND_OP_REVERSE_SUBTRACT;
    case EBlendOp::MIN:
        return VK_BLEND_OP_MIN;
    case EBlendOp::MAX:
        return VK_BLEND_OP_MAX;
    default:
        HS_LOG(error, "Unsupported blend operation: %d", static_cast<int>(operation));
        return VK_BLEND_OP_ADD;
    }
}

EBlendOp RHIUtilityVulkan::FromBlendOp(VkBlendOp operation)
{
    switch (operation)
    {
    case VK_BLEND_OP_ADD:
        return EBlendOp::ADD;
    case VK_BLEND_OP_SUBTRACT:
        return EBlendOp::SUBTRACT;
    case VK_BLEND_OP_REVERSE_SUBTRACT:
        return EBlendOp::REVERSE_SUBTRACT;
    case VK_BLEND_OP_MIN:
        return EBlendOp::MIN;
    case VK_BLEND_OP_MAX:
        return EBlendOp::MAX;
    default:
        HS_LOG(error, "Unsupported VkBlendOp: %d", static_cast<int>(operation));
        return EBlendOp::INVALID;
    }
}

VkCompareOp RHIUtilityVulkan::ToCompareOp(ECompareOp compare)
{
    switch (compare)
    {
    case ECompareOp::NEVER:
        return VK_COMPARE_OP_NEVER;
    case ECompareOp::LESS:
        return VK_COMPARE_OP_LESS;
    case ECompareOp::EQUAL:
        return VK_COMPARE_OP_EQUAL;
    case ECompareOp::LESS_OR_EQUAL:
        return VK_COMPARE_OP_LESS_OR_EQUAL;
    case ECompareOp::GREATER:
        return VK_COMPARE_OP_GREATER;
    case ECompareOp::NOT_EQUAL:
        return VK_COMPARE_OP_NOT_EQUAL;
    case ECompareOp::GREATER_OR_EQUAL:
        return VK_COMPARE_OP_GREATER_OR_EQUAL;
    case ECompareOp::ALWAYS:
        return VK_COMPARE_OP_ALWAYS;
    default:
        HS_LOG(error, "Unsupported compare operation: %d", static_cast<int>(compare));
        return VK_COMPARE_OP_NEVER;
    }
}

ECompareOp RHIUtilityVulkan::FromCompareOp(VkCompareOp compare)
{
    switch (compare)
    {
    case VK_COMPARE_OP_NEVER:
        return ECompareOp::NEVER;
    case VK_COMPARE_OP_LESS:
        return ECompareOp::LESS;
    case VK_COMPARE_OP_EQUAL:
        return ECompareOp::EQUAL;
    case VK_COMPARE_OP_LESS_OR_EQUAL:
        return ECompareOp::LESS_OR_EQUAL;
    case VK_COMPARE_OP_GREATER:
        return ECompareOp::GREATER;
    case VK_COMPARE_OP_NOT_EQUAL:
        return ECompareOp::NOT_EQUAL;
    case VK_COMPARE_OP_GREATER_OR_EQUAL:
        return ECompareOp::GREATER_OR_EQUAL;
    case VK_COMPARE_OP_ALWAYS:
        return ECompareOp::ALWAYS;
    default:
        HS_LOG(error, "Unsupported VkCompareOp: %d", static_cast<int>(compare));
        return ECompareOp::NEVER;
    }
}

VkFrontFace RHIUtilityVulkan::ToFrontFace(EFrontFace frontFace)
{
    switch (frontFace)
    {
    case EFrontFace::COUNTER_CLOCKWISE:
        return VK_FRONT_FACE_COUNTER_CLOCKWISE;
    case EFrontFace::CLOCKWISE:
        return VK_FRONT_FACE_CLOCKWISE;
    default:
        HS_LOG(error, "Unsupported front face: %d", static_cast<int>(frontFace));
        return VK_FRONT_FACE_COUNTER_CLOCKWISE;
    }
}

EFrontFace RHIUtilityVulkan::FromFrontFace(VkFrontFace frontFace)
{
    switch (frontFace)
    {
    case VK_FRONT_FACE_COUNTER_CLOCKWISE:
        return EFrontFace::COUNTER_CLOCKWISE;
    case VK_FRONT_FACE_CLOCKWISE:
        return EFrontFace::CLOCKWISE;
    default:
        HS_LOG(error, "Unsupported VkFrontFace: %d", static_cast<int>(frontFace));
        return EFrontFace::COUNTER_CLOCKWISE;
    }
}

VkCullModeFlags RHIUtilityVulkan::ToCullMode(ECullMode cullMode)
{
    switch (cullMode)
    {
    case ECullMode::NONE:
        return VK_CULL_MODE_NONE;
    case ECullMode::FRONT:
        return VK_CULL_MODE_FRONT_BIT;
    case ECullMode::BACK:
        return VK_CULL_MODE_BACK_BIT;
    case ECullMode::ALL:
        return VK_CULL_MODE_FRONT_AND_BACK;
    default:
        HS_LOG(error, "Unsupported cull mode: %d", static_cast<int>(cullMode));
        return VK_CULL_MODE_NONE;
    }
}

ECullMode RHIUtilityVulkan::FromCullMode(VkCullModeFlags cullMode)
{
    switch (cullMode)
    {
    case VK_CULL_MODE_NONE:
        return ECullMode::NONE;
    case VK_CULL_MODE_FRONT_BIT:
        return ECullMode::FRONT;
    case VK_CULL_MODE_BACK_BIT:
        return ECullMode::BACK;
    case VK_CULL_MODE_FRONT_AND_BACK:
        return ECullMode::ALL;
    default:
        HS_LOG(error, "Unsupported VkCullModeFlags: %d", static_cast<int>(cullMode));
        return ECullMode::NONE;
    }
}

// Note: These functions seem to have incorrect names in the header
// They should probably handle VkPolygonMode instead of VkPrimitiveTopology
// But implementing according to the function signature provided
//VkPrimitiveTopology RHIUtilityVulkan::ToPolygonMode(EPolygonMode polygonMode)
//{
//    // WARNING: Function name suggests VkPolygonMode but returns VkPrimitiveTopology
//    // This might be a naming error in the header file
//    switch (polygonMode)
//    {
//    case EPolygonMode::FILL:
//        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // Default topology for filled polygons
//    case EPolygonMode::LINE:
//        return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
//    case EPolygonMode::POINT:
//        return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
//    default:
//        HS_LOG(error, "Unsupported polygon mode: %d", static_cast<int>(polygonMode));
//        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
//    }
//}
//
//EPolygonMode RHIUtilityVulkan::FromPolygonMode(VkPrimitiveTopology polygonMode)
//{
//    // WARNING: Function name suggests VkPolygonMode but accepts VkPrimitiveTopology
//    // This might be a naming error in the header file
//    switch (polygonMode)
//    {
//    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
//    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
//    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN:
//        return EPolygonMode::FILL;
//    case VK_PRIMITIVE_TOPOLOGY_LINE_LIST:
//    case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP:
//        return EPolygonMode::LINE;
//    case VK_PRIMITIVE_TOPOLOGY_POINT_LIST:
//        return EPolygonMode::POINT;
//    default:
//        HS_LOG(error, "Unsupported VkPrimitiveTopology: %d", static_cast<int>(polygonMode));
//        return EPolygonMode::FILL;
//    }
//}

VkPrimitiveTopology RHIUtilityVulkan::ToPrimitiveTopology(EPrimitiveTopology topology)
{
    switch (topology)
    {
    case EPrimitiveTopology::POINT_LIST:
        return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    case EPrimitiveTopology::LINE_LIST:
        return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    case EPrimitiveTopology::LINE_STRIP:
        return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    case EPrimitiveTopology::TRIANGLE_LIST:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    case EPrimitiveTopology::TRIANGLE_STRIP:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    case EPrimitiveTopology::TRIANGLE_FAN:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
    case EPrimitiveTopology::LINE_LIST_WITH_ADJACENCY:
        return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
    case EPrimitiveTopology::LINE_STRIP_WITH_ADJACENCY:
        return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
    case EPrimitiveTopology::TRIANGLE_LIST_WITH_ADJACENCY:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
    case EPrimitiveTopology::TRIANGLE_STRIP_WITH_ADJACENCY:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
    case EPrimitiveTopology::PATCH_LIST:
        return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
    default:
        HS_LOG(error, "Unsupported primitive topology: %d", static_cast<int>(topology));
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    }
}

EPrimitiveTopology RHIUtilityVulkan::FromPrimitiveTopology(VkPrimitiveTopology topology)
{
    switch (topology)
    {
    case VK_PRIMITIVE_TOPOLOGY_POINT_LIST:
        return EPrimitiveTopology::POINT_LIST;
    case VK_PRIMITIVE_TOPOLOGY_LINE_LIST:
        return EPrimitiveTopology::LINE_LIST;
    case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP:
        return EPrimitiveTopology::LINE_STRIP;
    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
        return EPrimitiveTopology::TRIANGLE_LIST;
    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
        return EPrimitiveTopology::TRIANGLE_STRIP;
    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN:
        return EPrimitiveTopology::TRIANGLE_FAN;
    case VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY:
        return EPrimitiveTopology::LINE_LIST_WITH_ADJACENCY;
    case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY:
        return EPrimitiveTopology::LINE_STRIP_WITH_ADJACENCY;
    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY:
        return EPrimitiveTopology::TRIANGLE_LIST_WITH_ADJACENCY;
    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY:
        return EPrimitiveTopology::TRIANGLE_STRIP_WITH_ADJACENCY;
    case VK_PRIMITIVE_TOPOLOGY_PATCH_LIST:
        return EPrimitiveTopology::PATCH_LIST;
    default:
        HS_LOG(error, "Unsupported VkPrimitiveTopology: %d", static_cast<int>(topology));
        return EPrimitiveTopology::TRIANGLE_LIST;
    }
}

const char* RHIUtilityVulkan::ToString(VkResult result)
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