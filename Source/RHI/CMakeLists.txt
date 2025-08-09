set(TARGET_NAME RHI)

set(TOTAL_FILES)

set(RHI_HEADERS
    CommandHandle.h
    RenderHandle.h
    ResourceHandle.h
    RHIContext.h
    RHIDefinition.h
)

source_group("Public" FIELS ${RHI_HEADERS})
list(APPEND TOTLA_FILES ${RHI_HEADERS})

set(RHI_SOURCES
    Private/CommandHandle.cpp
    Private/RenderHandle.cpp
    Private/ResourceHandle.cpp
    Private/RHIDefinition.cpp
)

source_group("Private" FIELS ${RHI_SOURCES})
list(APPEND TOTLA_FILES ${RHI_SOURCES})

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
        Metal/MetalDescriptorPoolAllocator.h
        Metal/MetalDevice.h
        Metal/MetalRenderHandle.h
        Metal/MetalResourceHandle.h
        Metal/MetalStagingBuffer.h
        Metal/MetalSwapchain.h
        Metal/MetalUtility.h
    )

    source_group("Metal\\Public" FILES ${RHI_METAL_HEADERS})
    list(APPEND TOTAL_FILES ${RHI_METAL_HEADERS})

    set(RHI_METAL_SOURCES
        Metal/Private/MetalCommandHandle.cpp
        Metal/Private/MetalContext.cpp
        Metal/Private/MetalDefinition.cpp
        Metal/Private/MetalDescriptorPoolAllocator.cpp
        Metal/Private/MetalDevice.cpp
        Metal/Private/MetalRenderHandle.cpp
        Metal/Private/MetalResourceHandle.cpp
        Metal/Private/MetalStagingBuffer.cpp
        Metal/Private/MetalSwapchain.cpp
        Metal/Private/MetalUtility.cpp
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

if(WIN32)
    target_link_libraries(${TARGET_NAME}
        PRIVATE
        ${VULKAN} ${SPIRV})
endif()

target_include_directories(${TARGET_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_dependencies(${TARGET_NAME} Resource)

target_compile_definitions(${TARGET_NAME}
    PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:MinSizeRel>:_RELEASE>
    $<$<CONFIG:Release>:_RELEASE>
    $<$<CONFIG:RelWithDebInfo>:_RELWITHDEBINFO>
    HS_RHI
    HS_API_EXPORT
    HS_RESOURCE_API_IMPORT
)

if(USE_SHADERSYSTEM)
    add_dependencies(${TARGET_NAME} ShaderSystem)
    target_compile_definitions(${TARGET_NAME} PRIVATE HS_SHADERSYSTEM_API_IMPORT)
endif()

install(TARGETS ${TARGET_NAME}
    ARCHIVE DESTINATION "${INSTALL_FOLDER}/dev/lib"
    LIBRARY DESTINATION "${INSTALL_FOLDER}"
    RUNTIME DESTINATION "${INSTALL_FOLDER}"
)