#include "RHI/Vulkan/VulkanUtility.h"

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

VkFormat RHIUtilityVulkan::ToVertexFormat(EVertexFormat format)
{
	switch (format)
	{
	case EVertexFormat::FLOAT:	return VK_FORMAT_R32_SFLOAT;
	case EVertexFormat::FLOAT2:	return VK_FORMAT_R32G32_SFLOAT;
	case EVertexFormat::FLOAT3:	return VK_FORMAT_R32G32B32_SFLOAT;
	case EVertexFormat::FLOAT4:	return VK_FORMAT_R32G32B32A32_SFLOAT;
	case EVertexFormat::HALF:	return VK_FORMAT_R16_SFLOAT;
	case EVertexFormat::HALF2:	return VK_FORMAT_R16G16_SFLOAT;
	case EVertexFormat::HALF3:	return VK_FORMAT_R16G16B16_SFLOAT;
	case EVertexFormat::HALF4:	return VK_FORMAT_R16G16B16A16_SFLOAT;
	default:
		HS_LOG(error, "Unsupported vertex format: %d", static_cast<int>(format));
	}
	return VK_FORMAT_UNDEFINED;
}

EVertexFormat RHIUtilityVulkan::FromVertexFormat(VkFormat format)
{
	switch (format)
	{
	case VK_FORMAT_R32_SFLOAT:			return EVertexFormat::FLOAT;
	case VK_FORMAT_R32G32_SFLOAT:		return EVertexFormat::FLOAT2;
	case VK_FORMAT_R32G32B32_SFLOAT:	return EVertexFormat::FLOAT3;
	case VK_FORMAT_R32G32B32A32_SFLOAT:	return EVertexFormat::FLOAT4;
	case VK_FORMAT_R16_SFLOAT:			return EVertexFormat::HALF;
	case VK_FORMAT_R16G16_SFLOAT:		return EVertexFormat::HALF2;
	case VK_FORMAT_R16G16B16_SFLOAT:	return EVertexFormat::HALF3;
	case VK_FORMAT_R16G16B16A16_SFLOAT:	return EVertexFormat::HALF4;
	default:
		HS_LOG(error, "Unsupported VkFormat for vertex format: %d", static_cast<int>(format));
	}
	return EVertexFormat::FLOAT;
}

VkAttachmentLoadOp RHIUtilityVulkan::ToLoadOp(ELoadAction action)
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
	}
	return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
}

ELoadAction RHIUtilityVulkan::FromLoadOp(VkAttachmentLoadOp action)
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
	}
	return ELoadAction::INVALID;
}

VkAttachmentStoreOp RHIUtilityVulkan::ToStoreOp(EStoreAction action)
{
	switch (action)
	{
	case EStoreAction::DONT_CARE:
		return VK_ATTACHMENT_STORE_OP_DONT_CARE;
	case EStoreAction::STORE:
		return VK_ATTACHMENT_STORE_OP_STORE;
	default:
		HS_LOG(error, "Unsupported store action: %d", static_cast<int>(action));
	}
	return VK_ATTACHMENT_STORE_OP_DONT_CARE;
}

EStoreAction RHIUtilityVulkan::FromStoreOp(VkAttachmentStoreOp action)
{
	switch (action)
	{
	case VK_ATTACHMENT_STORE_OP_DONT_CARE:
		return EStoreAction::DONT_CARE;
	case VK_ATTACHMENT_STORE_OP_STORE:
		return EStoreAction::STORE;
	default:
		HS_LOG(error, "Unsupported VkAttachmentStoreOp: %d", static_cast<int>(action));
	}
	return EStoreAction::INVALID;
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
	if ((usage & ETextureUsage::STATIC) != 0)
		flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	if ((usage & ETextureUsage::STAGING) != 0)
		flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

	if ((usage & ETextureUsage::SAMPLED) != 0)
		flags |= VK_IMAGE_USAGE_SAMPLED_BIT;

	if ((usage & ETextureUsage::STORAGE) != 0)
		flags |= VK_IMAGE_USAGE_STORAGE_BIT;

	if ((usage & ETextureUsage::COLOR_ATTACHMENT) != 0)
		flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	if ((usage & ETextureUsage::DEPTH_STENCIL_ATTACHMENT) != 0)
		flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

	if ((usage & ETextureUsage::TRANSIENT_ATTACHMENT) != 0)
		flags |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;

	if ((usage & ETextureUsage::INPUT_ATTACHMENT) != 0)
		flags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;

	return flags;
}

ETextureUsage RHIUtilityVulkan::FromTextureUsage(VkImageUsageFlags usage)
{
	ETextureUsage flags = ETextureUsage::UNKNOWN;
	if ((usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT) != 0)
		flags |= ETextureUsage::STATIC;

	if ((usage & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) != 0)
		flags |= ETextureUsage::STAGING;

	if ((usage & VK_IMAGE_USAGE_SAMPLED_BIT) != 0)
		flags |= ETextureUsage::SAMPLED;

	if ((usage & VK_IMAGE_USAGE_STORAGE_BIT) != 0)
		flags |= ETextureUsage::STORAGE;

	if ((usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) != 0)
		flags |= ETextureUsage::COLOR_ATTACHMENT;

	if ((usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0)
		flags |= ETextureUsage::DEPTH_STENCIL_ATTACHMENT;

	if ((usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) != 0)
		flags |= ETextureUsage::TRANSIENT_ATTACHMENT;

	if ((usage & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT) != 0)
		flags |= ETextureUsage::INPUT_ATTACHMENT;

	return flags;
}

VkImageType RHIUtilityVulkan::ToImageType(ETextureType type)
{
	switch (type)
	{
	case ETextureType::TEX_1D:
	case ETextureType::TEX_1D_ARRAY:
		return VK_IMAGE_TYPE_1D;
	case ETextureType::TEX_2D:
	case ETextureType::TEX_2D_ARRAY:
	case ETextureType::TEX_CUBE:  // Cube maps use 2D image type with 6 array layers
		return VK_IMAGE_TYPE_2D;
	case ETextureType::TEX_3D:
		return VK_IMAGE_TYPE_3D;
	default:
		HS_LOG(error, "Unsupported texture type: %d", static_cast<int>(type));
	}
	return VK_IMAGE_TYPE_2D;
}

ETextureType RHIUtilityVulkan::FromImageType(VkImageType type)
{
	switch (type)
	{
	case VK_IMAGE_TYPE_1D:
		return ETextureType::TEX_1D;
	case VK_IMAGE_TYPE_2D:
		return ETextureType::TEX_2D;
	case VK_IMAGE_TYPE_3D:
		return ETextureType::TEX_3D;
	default:
		HS_LOG(error, "Unsupported VkImageType: %d", static_cast<int>(type));
	}
	return ETextureType::INVALID;
}

VkImageViewType RHIUtilityVulkan::ToImageViewType(ETextureType type)
{
	switch (type)
	{
	case ETextureType::TEX_1D:
		return VK_IMAGE_VIEW_TYPE_1D;
	case ETextureType::TEX_1D_ARRAY:
		return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
	case ETextureType::TEX_2D:
		return VK_IMAGE_VIEW_TYPE_2D;
	case ETextureType::TEX_2D_ARRAY:
		return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
	case ETextureType::TEX_CUBE:
		return VK_IMAGE_VIEW_TYPE_CUBE;
	case ETextureType::TEX_3D:
		return VK_IMAGE_VIEW_TYPE_3D;
	default:
		HS_LOG(error, "Unsupported texture view type: %d", static_cast<int>(type));
	}
	return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
}

ETextureType RHIUtilityVulkan::FromImageViewType(VkImageViewType type)
{
	switch (type)
	{
	case VK_IMAGE_VIEW_TYPE_1D:
		return ETextureType::TEX_1D;
	case VK_IMAGE_VIEW_TYPE_1D_ARRAY:
		return ETextureType::TEX_1D_ARRAY;
	case VK_IMAGE_VIEW_TYPE_2D:
		return ETextureType::TEX_2D;
	case VK_IMAGE_VIEW_TYPE_2D_ARRAY:
		return ETextureType::TEX_2D_ARRAY;
	case VK_IMAGE_VIEW_TYPE_CUBE:
		return ETextureType::TEX_CUBE;
	case VK_IMAGE_VIEW_TYPE_3D:
		return ETextureType::TEX_3D;
	default:
		HS_LOG(error, "Unsupported VkImageViewType: %d", static_cast<int>(type));
	}
	return ETextureType::INVALID;
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
	}
	return VK_BLEND_FACTOR_ZERO;
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
	}
	return EBlendFactor::INVALID;
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
	}
	return VK_BLEND_OP_ADD;
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
	}
	return EBlendOp::INVALID;
}

VkCompareOp RHIUtilityVulkan::ToCompareOp(ECompareOp compareOp)
{
	switch (compareOp)
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
		HS_LOG(error, "Unsupported compare operation: %d", static_cast<int>(compareOp));
	}
	return VK_COMPARE_OP_NEVER;
}

ECompareOp RHIUtilityVulkan::FromCompareOp(VkCompareOp compareOp)
{
	switch (compareOp)
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
		HS_LOG(error, "Unsupported VkCompareOp: %d", static_cast<int>(compareOp));
	}
	return ECompareOp::NEVER;
}

VkStencilOp RHIUtilityVulkan::ToStencilOp(EStencilOp stencilOp)
{
	switch (stencilOp)
	{
	case EStencilOp::KEEP:					return VK_STENCIL_OP_KEEP;
	case EStencilOp::ZERO:					return VK_STENCIL_OP_ZERO;
	case EStencilOp::REPLACE:				return VK_STENCIL_OP_REPLACE;
	case EStencilOp::INCREMENT_AND_CLAMP:	return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
	case EStencilOp::DECREMENT_AND_CLAMP:	return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
	case EStencilOp::INVERT:				return VK_STENCIL_OP_INVERT;
	case EStencilOp::INCREMENT_AND_WARP:	return VK_STENCIL_OP_INCREMENT_AND_WRAP;
	case EStencilOp::DECREMENT_AND_WRAP:	return VK_STENCIL_OP_DECREMENT_AND_WRAP;
	default:
		HS_LOG(error, "Unsupported stencil operation: %d", static_cast<int>(stencilOp));
	}
	return VK_STENCIL_OP_KEEP;
}

EStencilOp RHIUtilityVulkan::FromStencilOp(VkStencilOp stencilOp)
{
	switch (stencilOp)
	{
	case VK_STENCIL_OP_KEEP:				return EStencilOp::KEEP;
	case VK_STENCIL_OP_ZERO:				return EStencilOp::ZERO;
	case VK_STENCIL_OP_REPLACE:				return EStencilOp::REPLACE;
	case VK_STENCIL_OP_INCREMENT_AND_CLAMP: return EStencilOp::INCREMENT_AND_CLAMP;
	case VK_STENCIL_OP_DECREMENT_AND_CLAMP: return EStencilOp::DECREMENT_AND_CLAMP;
	case VK_STENCIL_OP_INVERT:				return EStencilOp::INVERT;
	case VK_STENCIL_OP_INCREMENT_AND_WRAP:	return EStencilOp::INCREMENT_AND_WARP;
	case VK_STENCIL_OP_DECREMENT_AND_WRAP:	return EStencilOp::DECREMENT_AND_WRAP;
	default:
		HS_LOG(error, "Unsupported VkStencilOp: %d", static_cast<int>(stencilOp));
	}
	return EStencilOp::KEEP;
}

VkLogicOp RHIUtilityVulkan::ToLogicOp(ELogicOp logicOp)
{
	switch (logicOp)
	{
	case ELogicOp::CLEAR:			return VK_LOGIC_OP_CLEAR;
	case ELogicOp::AND:				return VK_LOGIC_OP_AND;
	case ELogicOp::AND_REVERSE:		return VK_LOGIC_OP_AND_REVERSE;
	case ELogicOp::COPY:			return VK_LOGIC_OP_COPY;
	case ELogicOp::AND_INVERTED:	return VK_LOGIC_OP_AND_INVERTED;
	case ELogicOp::NO_OP:			return VK_LOGIC_OP_NO_OP;
	case ELogicOp::XOR:				return VK_LOGIC_OP_XOR;
	case ELogicOp::OR:				return VK_LOGIC_OP_OR;
	case ELogicOp::NOR:				return VK_LOGIC_OP_NOR;
	case ELogicOp::EQUIVALENT:		return VK_LOGIC_OP_EQUIVALENT;
	case ELogicOp::INVERT:			return VK_LOGIC_OP_INVERT;
	case ELogicOp::OR_REVERSE:		return VK_LOGIC_OP_OR_REVERSE;
	case ELogicOp::COPY_INVERTED:	return VK_LOGIC_OP_COPY_INVERTED;
	case ELogicOp::OR_INVERTED:		return VK_LOGIC_OP_OR_INVERTED;
	case ELogicOp::NAND:			return VK_LOGIC_OP_NAND;
	case ELogicOp::SET:				return VK_LOGIC_OP_SET;
	default:
		HS_LOG(error, "Unsupported VkCompareOp: %d", static_cast<int>(logicOp));
	};
	return VK_LOGIC_OP_NO_OP;
}

ELogicOp RHIUtilityVulkan::FromLogicOp(VkLogicOp logicOp)
{
	switch (logicOp)
	{
	case VK_LOGIC_OP_CLEAR:			return ELogicOp::CLEAR;
	case VK_LOGIC_OP_AND:			return ELogicOp::AND;
	case VK_LOGIC_OP_AND_REVERSE:	return ELogicOp::AND_REVERSE;
	case VK_LOGIC_OP_COPY:			return ELogicOp::COPY;
	case VK_LOGIC_OP_AND_INVERTED:	return ELogicOp::AND_INVERTED;
	case VK_LOGIC_OP_NO_OP:			return ELogicOp::NO_OP;
	case VK_LOGIC_OP_XOR:			return ELogicOp::XOR;
	case VK_LOGIC_OP_OR:			return ELogicOp::OR;
	case VK_LOGIC_OP_NOR:			return ELogicOp::NOR;
	case VK_LOGIC_OP_EQUIVALENT:	return ELogicOp::EQUIVALENT;
	case VK_LOGIC_OP_INVERT:		return ELogicOp::INVERT;
	case VK_LOGIC_OP_OR_REVERSE:	return ELogicOp::OR_REVERSE;
	case VK_LOGIC_OP_COPY_INVERTED: return ELogicOp::COPY_INVERTED;
	case VK_LOGIC_OP_OR_INVERTED:	return ELogicOp::OR_INVERTED;
	case VK_LOGIC_OP_NAND:			return ELogicOp::NAND;
	case VK_LOGIC_OP_SET:			return ELogicOp::SET;
	default:
		HS_LOG(error, "Unsupported VkLogicOp: %d", static_cast<int>(logicOp));
	}
	return ELogicOp::NO_OP;
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
	}
	return VK_FRONT_FACE_COUNTER_CLOCKWISE;
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
	}
	return EFrontFace::COUNTER_CLOCKWISE;
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
	}
	return VK_CULL_MODE_NONE;
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
	}
	return ECullMode::NONE;
}

VkBufferUsageFlags RHIUtilityVulkan::ToBufferUsage(EBufferUsage usage)
{
	VkBufferUsageFlags flags = 0;
	if ((usage & EBufferUsage::VERTEX) != static_cast<EBufferUsage>(0))			flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	if ((usage & EBufferUsage::INDEX) != static_cast<EBufferUsage>(0))			flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	if ((usage & EBufferUsage::UNIFORM) != static_cast<EBufferUsage>(0))		flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	if ((usage & EBufferUsage::STORAGE_BUFFER) != static_cast<EBufferUsage>(0))	flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

	return flags;
}

EBufferUsage RHIUtilityVulkan::FromBufferUsage(VkBufferUsageFlags usage)
{
	EBufferUsage flags = EBufferUsage::INVALID;
	if (usage & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)		flags = flags | EBufferUsage::VERTEX;
	if (usage & VK_BUFFER_USAGE_INDEX_BUFFER_BIT)		flags = flags | EBufferUsage::INDEX;
	if (usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)		flags = flags | EBufferUsage::UNIFORM;
	if (usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)		flags = flags | EBufferUsage::STORAGE_BUFFER;

	return flags;
}

// Note: These functions seem to have incorrect names in the header
// They should probably handle VkPolygonMode instead of VkPrimitiveTopology
// But implementing according to the function signature provided
VkPolygonMode RHIUtilityVulkan::ToPolygonMode(EPolygonMode polygonMode)
{
	// WARNING: Function name suggests VkPolygonMode but returns VkPrimitiveTopology
	// This might be a naming error in the header file
	switch (polygonMode)
	{
	case EPolygonMode::FILL:
		return VK_POLYGON_MODE_FILL; // Default topology for filled polygons
	case EPolygonMode::LINE:
		return VK_POLYGON_MODE_LINE;
	case EPolygonMode::POINT:
		return VK_POLYGON_MODE_POINT;
	default:
		HS_LOG(error, "Unsupported polygon mode: %d", static_cast<int>(polygonMode));
	}
	return VK_POLYGON_MODE_FILL;
}

EPolygonMode RHIUtilityVulkan::FromPolygonMode(VkPolygonMode polygonMode)
{
	// WARNING: Function name suggests VkPolygonMode but accepts VkPrimitiveTopology
	// This might be a naming error in the header file
	switch (polygonMode)
	{
	case VK_POLYGON_MODE_FILL:
		return EPolygonMode::FILL;
	case VK_POLYGON_MODE_LINE:
		return EPolygonMode::LINE;
	case VK_POLYGON_MODE_POINT:
		return EPolygonMode::POINT;
	default:
		HS_LOG(error, "Unsupported VkPrimitiveTopology: %d", static_cast<int>(polygonMode));
	}
	return EPolygonMode::FILL;
}

VkShaderStageFlagBits RHIUtilityVulkan::ToShaderStageFlags(EShaderStage stage)
{
	VkShaderStageFlagBits flags = static_cast<VkShaderStageFlagBits>(0);
	if ((stage & EShaderStage::VERTEX) != EShaderStage::NONE)
	{
		flags = static_cast<VkShaderStageFlagBits>(flags | VK_SHADER_STAGE_VERTEX_BIT);
	}
	if ((stage & EShaderStage::FRAGMENT) != EShaderStage::NONE)
	{
		flags = static_cast<VkShaderStageFlagBits>(flags | VK_SHADER_STAGE_FRAGMENT_BIT);
	}
	if ((stage & EShaderStage::COMPUTE) != EShaderStage::NONE)
	{
		flags = static_cast<VkShaderStageFlagBits>(flags | VK_SHADER_STAGE_COMPUTE_BIT);
	}
	if ((stage & EShaderStage::GEOMETRY) != EShaderStage::NONE)
	{
		flags = static_cast<VkShaderStageFlagBits>(flags | VK_SHADER_STAGE_GEOMETRY_BIT);
	}
	if ((stage & EShaderStage::DOMAIN) != EShaderStage::NONE)
	{
		flags = static_cast<VkShaderStageFlagBits>(flags | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
	}
	if ((stage & EShaderStage::HULL) != EShaderStage::NONE)
	{
		flags = static_cast<VkShaderStageFlagBits>(flags | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
	}

	return flags;
}

EShaderStage RHIUtilityVulkan::FromShaderStageFlags(VkShaderStageFlagBits flags)
{
	EShaderStage stage = EShaderStage::NONE;
	if (flags & VK_SHADER_STAGE_VERTEX_BIT)
	{
		stage = stage | EShaderStage::VERTEX;
	}
	if (flags & VK_SHADER_STAGE_FRAGMENT_BIT)
	{
		stage = stage | EShaderStage::FRAGMENT;
	}
	if (flags & VK_SHADER_STAGE_COMPUTE_BIT)
	{
		stage = stage | EShaderStage::COMPUTE;
	}
	if (flags & VK_SHADER_STAGE_GEOMETRY_BIT)
	{
		stage = stage | EShaderStage::GEOMETRY;
	}
	if (flags & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT)
	{
		stage = stage | EShaderStage::DOMAIN;
	}
	if (flags & VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)
	{
		stage = stage | EShaderStage::HULL;
	}

	return stage;
}

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
	}
	return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
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
	}
	return EPrimitiveTopology::TRIANGLE_LIST;
}

VkSamplerAddressMode RHIUtilityVulkan::ToAddressMode(EAddressMode addressMode)
{
	switch (addressMode)
	{
	case EAddressMode::REPEAT:
		return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	case EAddressMode::MIRRORED_REPEAT:
		return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
	case EAddressMode::CLAMP_TO_EDGE:
		return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	case EAddressMode::CLAMP_TO_BORDER:
		return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	case EAddressMode::MIRROR_CLAMP_TO_EDGE:
		return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
	default:
		HS_LOG(error, "Unsupported address mode: %d", static_cast<int>(addressMode));
	}
	return VK_SAMPLER_ADDRESS_MODE_REPEAT; // Default fallback
}

EAddressMode RHIUtilityVulkan::FromAddressMode(VkSamplerAddressMode addressMode)
{
	switch (addressMode)
	{
	case VK_SAMPLER_ADDRESS_MODE_REPEAT:
		return EAddressMode::REPEAT;
	case VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT:
		return EAddressMode::MIRRORED_REPEAT;
	case VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE:
		return EAddressMode::CLAMP_TO_EDGE;
	case VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER:
		return EAddressMode::CLAMP_TO_BORDER;
	case VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE:
		return EAddressMode::MIRROR_CLAMP_TO_EDGE;
	default:
		HS_LOG(error, "Unsupported VkSamplerAddressMode: %d", static_cast<int>(addressMode));
	}
	return EAddressMode::REPEAT; // Default fallback
}

VkFilter RHIUtilityVulkan::ToFilter(EFilterMode filter)
{
	switch (filter)
	{
	case EFilterMode::NEAREST:
		return VK_FILTER_NEAREST;
	case EFilterMode::LINEAR:
		return VK_FILTER_LINEAR;
	default:
		HS_LOG(error, "Unsupported filter mode: %d", static_cast<int>(filter));
	}
	return VK_FILTER_NEAREST; // Default fallback
}

EFilterMode RHIUtilityVulkan::FromFilter(VkFilter filter)
{
	switch (filter)
	{
	case VK_FILTER_NEAREST:
		return EFilterMode::NEAREST;
	case VK_FILTER_LINEAR:
		return EFilterMode::LINEAR;
	default:
		HS_LOG(error, "Unsupported VkFilter: %d", static_cast<int>(filter));
	}
	return EFilterMode::NEAREST; // Default fallback
}

VkSamplerMipmapMode RHIUtilityVulkan::ToMipmapMode(EFilterMode mipmapMode)
{
	switch (mipmapMode)
	{
	case EFilterMode::NEAREST:
		return VK_SAMPLER_MIPMAP_MODE_NEAREST;
	case EFilterMode::LINEAR:
		return VK_SAMPLER_MIPMAP_MODE_LINEAR;
	default:
		HS_LOG(error, "Unsupported mipmap mode: %d", static_cast<int>(mipmapMode));
	}
	return VK_SAMPLER_MIPMAP_MODE_NEAREST; // Default fallback
}

EFilterMode RHIUtilityVulkan::FromMipmapMode(VkSamplerMipmapMode mipmapMode)
{
	switch (mipmapMode)
	{
	case VK_SAMPLER_MIPMAP_MODE_NEAREST:
		return EFilterMode::NEAREST;
	case VK_SAMPLER_MIPMAP_MODE_LINEAR:
		return EFilterMode::LINEAR;
	default:
		HS_LOG(error, "Unsupported VkSamplerMipmapMode: %d", static_cast<int>(mipmapMode));
	}
	return EFilterMode::NEAREST; // Default fallback
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