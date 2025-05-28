//
//  RHIContextVulkan.h
//  Engine
//
//  Created by Yongsik Im on 4/11/25.
//
#ifndef __HS_RHI_UTILITY_VULKAN_H__
#define __HS_RHI_UTILITY_VULKAN_H__

#include "Precompile.h"

#include "Engine/Core/Log.h"
#include "Engine/RHI/RHIDefinition.h"

#define VK_USE_PLATFORM_WIN32_KHR 

#include <vulkan/vulkan.h>
#include <string>


HS_NS_BEGIN

#define VK_CHECK_RESULT(vkFunc)                                                             \
    do                                                                                      \
    {                                                                                       \
        if (VkResult result = vkFunc; result != VK_SUCCESS)                                 \
        {                                                                                   \
            HS_LOG(error, "%s returns %s.", #vkFunc, HS::RHIUtilityVulkan::ToString(result));    \
        }                                                                                   \
    } while (0)

#define VK_CHECK_RESULT_AND_RETURN(vkFunc)                                                  \
    do                                                                                      \
    {                                                                                       \
        if (VkResult result = vkFunc; result != VK_SUCCESS)                                 \
        {                                                                                   \
            HS_LOG(error, "%s returns %s.", #vkFunc, HS::RHIUtilityVulkan::ToString(result));    \
            return result;                                                                  \
        }                                                                                   \
    } while (0)

#define VK_CHECK_RESULT_AND_THROW(vkFunc)                                                   \
    do                                                                                      \
    {                                                                                       \
        if (VkResult result = vkFunc; result != VK_SUCCESS)                                 \
        {                                                                                   \
            HS_LOG(error, "%s returns %s.", #vkFunc, HS::RHIUtilityVulkan::ToString(result));    \
            throw Exception(__FILE__, __LINE__, result)                                     \
        }                                                                                   \
    } while (0)


class RHIUtilityVulkan
{
public:
	static VkFormat ToPixelFormat(EPixelFormat format);
	static EPixelFormat FromPixelFormat(VkFormat format);

	static VkAttachmentLoadOp ToLoadAction(ELoadAction action);
	static ELoadAction FromLoadAction(VkAttachmentLoadOp action);

	static VkAttachmentStoreOp ToStoreAction(EStoreAction action);
	static EStoreAction FromStoreAction(VkAttachmentStoreOp action);

	static VkViewport ToViewport(Viewport vp);
	static Viewport FromViewport(VkViewport vp);

	static VkImageUsageFlags ToTextureUsage(ETextureUsage usage);
	static ETextureUsage FromTextureUsage(VkImageUsageFlags usage);

	static VkImageType ToTextureType(ETextureType type);
	static ETextureType FromTextureType(VkImageType type);

	static VkBlendFactor ToBlendFactor(EBlendFactor factor);
	static EBlendFactor FromBlendFactor(VkBlendFactor factor);

	static VkBlendOp ToBlendOp(EBlendOp operation);
	static EBlendOp FromBlendOp(VkBlendOp operation);

	static VkCompareOp ToCompareOp(ECompareOp compare);
	static ECompareOp FromCompareOp(VkCompareOp compare);

	static VkFrontFace ToFrontFace(EFrontFace frontFace);
	static EFrontFace FromFrontFace(VkFrontFace frontFace);

	static VkCullModeFlags ToCullMode(ECullMode cullMode);
	static ECullMode FromCullMode(VkCullModeFlags cullMode);

	//static VkPrimitiveTopology ToPolygonMode(EPolygonMode polygonMode);
	//static EPolygonMode FromPolygonMode(VkPrimitiveTopology polygonMode);

	static VkPrimitiveTopology ToPrimitiveTopology(EPrimitiveTopology topology);
	static EPrimitiveTopology FromPrimitiveTopology(VkPrimitiveTopology topology);

	static std::string ToString(VkResult result);
};
//
//size_t hs_rhi_get_bytes_per_pixel(EPixelFormat format);
//size_t hs_rhi_get_bytes_per_pixel(VkFormat format);
//
//VkFormat hs_rhi_get_vertex_format_from_size(size_t size);
//size_t hs_rhi_get_size_from_vertex_format(VkFormat format);
//
//VkBufferUsageFlags hs_rhi_to_buffer_option(EBufferMemoryOption option);
//EBufferMemoryOption hs_rhi_from_buffer_option(VkBufferUsageFlags option);
//
//VkClearColorValue hs_rhi_to_clear_color(const float* color);
//void hs_rhi_from_clear_color(VkClearColorValue color, float* outColor);

HS_NS_END

#endif