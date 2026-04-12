set(TARGET_NAME Renderer)

set(TOTAL_FILES)

set(RENDERER_HEADERS
    CameraUtils.h
    CoordinateConventionValidation.h
    RenderDefinition.h
    RendererDefinition.h
    RenderTarget.h
    Renderer.h
    ForwardRenderer.h
    ForwardOverlayRenderer.h
    DeferredRenderer.h
    RenderResourceManager.h
    ShaderLibrary.h
    RenderGraph.h
)

source_group("Public" FILES ${RENDERER_HEADERS})
list(APPEND TOTAL_FILES ${RENDERER_HEADERS})

set(RENDERER_SOURCES
    Private/CoordinateConventionValidation.cpp
    Private/Renderer.cpp
    Private/ForwardRenderer.cpp
    Private/ForwardOverlayRenderer.cpp
    Private/DeferredRenderer.cpp
    Private/RenderTarget.cpp
    Private/RenderResourceManager.cpp
    Private/ShaderLibrary.cpp
)

source_group("Private" FILES ${RENDERER_SOURCES})
list(APPEND TOTAL_FILES ${RENDERER_SOURCES})

set(RENDERER_RENDER_GRAPH_HEADERS
    RenderGraph/RenderGraphBuilder.h
    RenderGraph/RenderGraphResource.h
    RenderGraph/RenderGraphTransientAllocator.h
)

source_group("RenderGraph\\Public" FILES ${RENDERER_RENDER_GRAPH_HEADERS})
list(APPEND TOTAL_FILES ${RENDERER_RENDER_GRAPH_HEADERS})

set(RENDERER_RENDER_GRAPH_SOURCES
    RenderGraph/Private/RenderGraphBuilder.cpp
    RenderGraph/Private/RenderGraphResource.cpp
    RenderGraph/Private/RenderGraphTransientAllocator.cpp
)

source_group("RenderGraph\\Private" FILES ${RENDERER_RENDER_GRAPH_SOURCES})
list(APPEND TOTAL_FILES ${RENDERER_RENDER_GRAPH_SOURCES})

set(RENDERER_RENDER_PASS_HEADERS
    RenderPass/RenderPass.h
    RenderPass/ForwardRenderPass.h
    RenderPass/ForwardOpaquePass.h
    RenderPass/ForwardGridPass.h
    RenderPass/ForwardDebugPass.h
    RenderPass/ForwardIconPass.h
)

source_group("RenderPass\\Public" FILES ${RENDERER_RENDER_PASS_HEADERS})
list(APPEND TOTAL_FILES ${RENDERER_RENDER_PASS_HEADERS})

set(RENDERER_RENDER_PASS_SOURCES
    RenderPass/Private/RenderPass.cpp
    RenderPass/Private/ForwardRenderPass.cpp
    RenderPass/Private/ForwardOpaquePass.cpp
    RenderPass/Private/ForwardGridPass.cpp
    RenderPass/Private/ForwardDebugPass.cpp
    RenderPass/Private/ForwardIconPass.cpp
)

source_group("RenderPass\\Private" FILES ${RENDERER_RENDER_PASS_SOURCES})
list(APPEND TOTAL_FILES ${RENDERER_RENDER_PASS_SOURCES})

add_library(${TARGET_NAME} SHARED ${TOTAL_FILES})

if(APPLE)
    if(HSMR_USE_SDL)
        target_link_options(${TARGET_NAME}
            PRIVATE
            -F${HS_DEPS_LIB_DIR}
        )
    endif()

    target_link_libraries(${TARGET_NAME}
        PRIVATE
        Core RHI ShaderSystem Resource Scene
    )
else()
    target_link_libraries(${TARGET_NAME}
        PRIVATE
        Core RHI ShaderSystem Resource Scene
    )
endif()

if(WIN32)
    # Core and RHI are SHARED, no WHOLEARCHIVE needed
endif()

target_include_directories(${TARGET_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(${TARGET_NAME}
    PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:MinSizeRel>:_RELEASE>
    $<$<CONFIG:Release>:_RELEASE>
    $<$<CONFIG:RelWithDebInfo>:_RELWITHDEBINFO>

    HS_RENDERER_API_EXPORT
    HS_RESOURCE_API_IMPORT
    HS_SHADER_SYSTEM_API_IMPORT
    HS_SCENE_API_IMPORT
    HS_CORE_API_IMPORT
    HS_RHI_API_IMPORT
)

install(TARGETS ${TARGET_NAME}
    ARCHIVE DESTINATION "${INSTALL_FOLDER}/dev/lib"
    LIBRARY DESTINATION "${INSTALL_FOLDER}"
    RUNTIME DESTINATION "${INSTALL_FOLDER}"
)

hs_auto_install(${TARGET_NAME})
