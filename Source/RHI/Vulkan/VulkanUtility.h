//
//  VulkanContext.h
//  Engine
//
//  Created by Yongsik Im on 4/11/25.
//
#ifndef __HS_RHI_UTILITY_VULKAN_H__
#define __HS_RHI_UTILITY_VULKAN_H__

#include "Precompile.h"

#include "Core/Log.h"
#include "RHI/RHIDefinition.h"

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

	static VkFormat ToVertexFormat(EVertexFormat format);
	static EVertexFormat FromVertexFormat(VkFormat format);

	static VkAttachmentLoadOp ToLoadOp(ELoadAction action);
	static ELoadAction FromLoadOp(VkAttachmentLoadOp action);

	static VkAttachmentStoreOp ToStoreOp(EStoreAction action);
	static EStoreAction FromStoreOp(VkAttachmentStoreOp action);

	static VkViewport ToViewport(Viewport vp);
	static Viewport FromViewport(VkViewport vp);

	static VkImageUsageFlags ToTextureUsage(ETextureUsage usage);
	static ETextureUsage FromTextureUsage(VkImageUsageFlags usage);

	static VkImageType ToImageType(ETextureType type);
	static ETextureType FromImageType(VkImageType type);

	static VkImageViewType ToImageViewType(ETextureType type);
	static ETextureType FromImageViewType(VkImageViewType type);

	static VkBlendFactor ToBlendFactor(EBlendFactor factor);
	static EBlendFactor FromBlendFactor(VkBlendFactor factor);

	static VkBlendOp ToBlendOp(EBlendOp operation);
	static EBlendOp FromBlendOp(VkBlendOp operation);

	static VkLogicOp ToLogicOp(ELogicOp logicOp);
	static ELogicOp FromLogicOp(VkLogicOp logicOp);

	static VkCompareOp ToCompareOp(ECompareOp compareOp);
	static ECompareOp FromCompareOp(VkCompareOp compareOp);

	static VkStencilOp ToStencilOp(EStencilOp stencilOp);
	static EStencilOp FromStencilOp(VkStencilOp stencilOp);

	static VkFrontFace ToFrontFace(EFrontFace frontFace);
	static EFrontFace FromFrontFace(VkFrontFace frontFace);

	static VkCullModeFlags ToCullMode(ECullMode cullMode);
	static ECullMode FromCullMode(VkCullModeFlags cullMode);

	static VkBufferUsageFlags ToBufferUsage(EBufferUsage usage);
	static EBufferUsage FromBufferUsage(VkBufferUsageFlags usage);

	static VkPolygonMode ToPolygonMode(EPolygonMode polygonMode);
	static EPolygonMode FromPolygonMode(VkPolygonMode polygonMode);

	static VkShaderStageFlagBits ToShaderStageFlags(EShaderStage stage);
	static EShaderStage FromShaderStageFlags(VkShaderStageFlagBits flags);

	static VkPrimitiveTopology ToPrimitiveTopology(EPrimitiveTopology topology);
	static EPrimitiveTopology FromPrimitiveTopology(VkPrimitiveTopology topology);
	
	static VkSamplerAddressMode ToAddressMode(EAddressMode addressMode);
	static EAddressMode FromAddressMode(VkSamplerAddressMode addressMode);

	static VkFilter ToFilter(EFilterMode filter);
	static EFilterMode FromFilter(VkFilter filter);

	static VkSamplerMipmapMode ToMipmapMode(EFilterMode mipmapMode);
	static EFilterMode FromMipmapMode(VkSamplerMipmapMode mipmapMode);

	//static VkVertexInputRate ToInputRate(uint8 stepRate);
	//static uint8 FromInputRate(VkVertexInputRate inputRate);


	static const char* ToString(VkResult result);
};

HS_NS_END

#endif