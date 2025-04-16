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


std::string hs_rhi_vkresult_to_string(VkResult result);


HS_NS_END

#endif