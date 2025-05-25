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

#include <vulkan/vulkan.h>
#include <string>


HS_NS_BEGIN

#define VK_CHECK_RESULT(vkFunc)                                                             \
    do                                                                                      \
    {                                                                                       \
        if (VkResult result = vkFunc; result != VK_SUCCESS)                                 \
        {                                                                                   \
            HS_LOG(error, "%s returns %s.", #vkFunc, hs_rhi_vkresult_to_string(result));    \
        }                                                                                   \
    } while (0)

#define VK_CHECK_RESULT_AND_RETURN(vkFunc)                                                  \
    do                                                                                      \
    {                                                                                       \
        if (VkResult result = vkFunc; result != VK_SUCCESS)                                 \
        {                                                                                   \
            HS_LOG(error, "%s returns %s.", #vkFunc, hs_rhi_vkresult_to_string(result));    \
            return result;                                                                  \
        }                                                                                   \
    } while (0)

#define VK_CHECK_RESULT_AND_THROW(vkFunc)                                                   \
    do                                                                                      \
    {                                                                                       \
        if (VkResult result = vkFunc; result != VK_SUCCESS)                                 \
        {                                                                                   \
            HS_LOG(error, "%s returns %s.", #vkFunc, hs_rhi_vkresult_to_string(result));    \
            throw Exception(__FILE__, __LINE__, result);                                                                  \
        }                                                                                   \
    } while (0)



VkFormat hs_rhi_to_pixel_format(EPixelFormat format);
EPixelFormat   hs_rhi_from_pixel_format(VkFormat format);

VkAttachmentLoadOp hs_rhi_to_load_action(ELoadAction action);
ELoadAction   hs_rhi_from_laod_action(VkAttachmentLoadOp action);

VkAttachmentStoreOp hs_rhi_to_store_action(EStoreAction action);
EStoreAction   hs_rhi_from_store_action(VkAttachmentStoreOp action);

VkViewport hs_rhi_to_viewport(Viewport vp);
Viewport    hs_rhi_from_viewport(VkViewport vp);

VkImageUsageFlags hs_rhi_to_texture_usage(ETextureUsage usage);
ETextureUsage   hs_rhi_from_texture_usage(VkImageUsageFlags usage);

VkImageType hs_rhi_to_texture_type(ETextureType type);
ETextureType   hs_rhi_from_texture_type(VkImageType type);

VkBlendFactor hs_rhi_to_blend_factor(EBlendFactor factor);
EBlendFactor hs_rhi_from_blend_factor(VkBlendFactor factor);

VkBlendOp hs_rhi_to_blend_operation(EBlendOp operation);
EBlendOp hs_rhi_from_blend_operation(VkBlendOp operation);

VkCompareOp hs_rhi_to_compare_function(ECompareOp compare);
ECompareOp hs_rhi_from_compare_function(VkCompareOp compare);

VkFrontFace hs_rhi_to_front_face(EFrontFace frontFace);
EFrontFace hs_rhi_from_front_face(VkFrontFace frontFace);

VkCullModeFlags hs_rhi_to_cull_mode(ECullMode cullMode);
ECullMode hs_rhi_from_cull_mode(VkCullModeFlags cullMode);

VkPrimitiveTopology hs_rhi_to_polygon_mode(EPolygonMode polygonMode);
EPolygonMode hs_rhi_from_polygon_mode(VkPrimitiveTopology                         polygonMode);

VkPrimitiveTopology hs_rhi_to_primitive_topology(EPrimitiveTopology topology);
EPrimitiveTopology hs_rhi_from_primitive_topology(VkPrimitiveTopology topology);

size_t hs_rhi_get_bytes_per_pixel(EPixelFormat format);
size_t hs_rhi_get_bytes_per_pixel(VkFormat format);

MTLVertexFormat hs_rhi_get_vertex_format_from_size(size_t size);
size_t hs_rhi_get_size_from_vertex_format(MTLVertexFormat format);

MTLResourceOptions  hs_rhi_to_buffer_option(EBufferMemoryOption option);
EBufferMemoryOption hs_rhi_from_buffer_option(MTLResourceOptions option);

MTLClearColor hs_rhi_to_clear_color(const float* color);
void          hs_rhi_from_clear_color(MTLClearColor color, float* outColor);


std::string hs_rhi_vkresult_to_string(VkResult result);

HS_NS_END

#endif