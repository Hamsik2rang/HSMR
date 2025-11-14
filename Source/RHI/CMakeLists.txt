set(TARGET_NAME RHI)

set(TOTAL_FILES)

set(RHI_HEADERS
    CommandHandle.h
    RenderHandle.h
    ResourceHandle.h
    RHIContext.h
    RHIDefinition.h
    Swapchain.h
)

source_group("Public" FILES ${RHI_HEADERS})
list(APPEND TOTAL_FILES ${RHI_HEADERS})

set(RHI_SOURCES
    Private/CommandHandle.cpp
    Private/RenderHandle.cpp
    Private/ResourceHandle.cpp
    Private/RHIContext.cpp
    Private/RHIDefinition.cpp
    Private/Swapchain.cpp
)

source_group("Private" FILES ${RHI_SOURCES})
list(APPEND TOTAL_FILES ${RHI_SOURCES})

if(WIN32)
    set(RHI_VULKAN_HEADERS
        Vulkan/VulkanCommandHandle.h
        Vulkan/VulkanContext.h
        Vulkan/VulkanDefinition.h
        Vulkan/VulkanDescriptorPoolAllocator.h
        Vulkan/VulkanDevice.h
        Vulkan/VulkanRenderHandle.h
        Vulkan/VulkanResourceHandle.h
        Vulkan/VulkanStagingBuffer.h
        Vulkan/VulkanSwapchain.h
        Vulkan/VulkanUtility.h
    )

    source_group("Vulkan\\Public" FILES ${RHI_VULKAN_HEADERS})
    list(APPEND TOTAL_FILES ${RHI_VULKAN_HEADERS})

    set(RHI_VULKAN_SOURCES
        Vulkan/Private/VulkanCommandHandle.cpp
        Vulkan/Private/VulkanContext.cpp
        Vulkan/Private/VulkanDefinition.cpp
        Vulkan/Private/VulkanDescriptorPoolAllocator.cpp
        Vulkan/Private/VulkanDevice.cpp
        Vulkan/Private/VulkanRenderHandle.cpp
        Vulkan/Private/VulkanResourceHandle.cpp
        Vulkan/Private/VulkanStagingBuffer.cpp
        Vulkan/Private/VulkanSwapchain.cpp
        Vulkan/Private/VulkanUtility.cpp
    )

    source_group("Vulkan\\Private" FILES ${RHI_VULKAN_SOURCES})
    list(APPEND TOTAL_FILES ${RHI_VULKAN_SOURCES})
else()
    set(RHI_METAL_HEADERS
        Metal/MetalCommandHandle.h
        Metal/MetalContext.h
        Metal/MetalDefinition.h
        Metal/MetalRenderHandle.h
        Metal/MetalResourceHandle.h
        Metal/MetalSwapchain.h
        Metal/MetalUtility.h
    )

    source_group("Metal\\Public" FILES ${RHI_METAL_HEADERS})
    list(APPEND TOTAL_FILES ${RHI_METAL_HEADERS})

    set(RHI_METAL_SOURCES
        Metal/Private/MetalCommandHandle.mm
        Metal/Private/MetalContext.mm
        Metal/Private/MetalDefinition.mm
        Metal/Private/MetalRenderHandle.mm
        Metal/Private/MetalResourceHandle.mm
        Metal/Private/MetalSwapchain.mm
        Metal/Private/MetalUtility.mm
    )

    source_group("Metal\\Private" FILES ${RHI_METAL_SOURCES})
    list(APPEND TOTAL_FILES ${RHI_METAL_SOURCES})
endif()

add_library(${TARGET_NAME} STATIC ${TOTAL_FILES})

if(APPLE)
    set(VULKAN)
    set(SPIRV)
else()
    set(VULKAN ${HS_DEPS_LIB_DIR}/vulkan-1.lib)
    set(SPIRV ${HS_DEPS_LIB_DIR}/SPIRV.lib)
endif()

target_link_libraries(${TARGET_NAME}
    PRIVATE
    Platform Core)

if(WIN32)
    target_link_libraries(${TARGET_NAME}
        PRIVATE
        ${VULKAN} ${SPIRV})
endif()

target_include_directories(${TARGET_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_dependencies(${TARGET_NAME} Core Platform)

target_compile_definitions(${TARGET_NAME}
    PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:MinSizeRel>:_RELEASE>
    $<$<CONFIG:Release>:_RELEASE>
    $<$<CONFIG:RelWithDebInfo>:_RELWITHDEBINFO>
    HS_RHI
)

install(TARGETS ${TARGET_NAME}
    ARCHIVE DESTINATION "${INSTALL_FOLDER}/dev/lib"
    LIBRARY DESTINATION "${INSTALL_FOLDER}"
    RUNTIME DESTINATION "${INSTALL_FOLDER}"
)