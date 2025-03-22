set(TARGET_NAME Engine)

set(TOTAL_FILES)

set(CORE_HEADERS
    Core/Application.h
    Core/Log.h
    Core/EngineContext.h
    Core/FileSystem.h
    Core/Window.h
    Core/Event.h
    Core/Flag.h
)

source_group("Core\\Public" FILES ${CORE_HEADERS})
list(APPEND TOTAL_FILES ${CORE_HEADERS})

set(CORE_SOURCES
    Core/Application.cpp
    Core/Log.cpp
    Core/EngineContext.cpp
    Core/FileSystem.cpp
    Core/Window.cpp
    Core/Event.cpp
    Core/Flag.cpp
)

source_group("Core\\Private" FILES ${CORE_SOURCES})
list(APPEND TOTAL_FILES ${CORE_SOURCES})

set(ECS_HEADERS
    ECS/Entity.h
    ECS/Scene.h
)

source_group("ECS\\Public" FILES ${ECS_HEADERS})
list(APPEND TOTAL_FILES ${ECS_HEADERS})

set(ECS_SOURCES
    ECS/Entity.cpp
    ECS/Scene.cpp
)

source_group("ECS\\Private" FILES ${ECS_SOURCES})
list(APPEND TOTAL_FILES ${ECS_SOURCES})

set(ECS_COMPONENT_HEADERS
    ECS/Component/Component.h
    ECS/Component/NameComponent.h
    ECS/Component/TransformComponent.h
    ECS/Component/LightComponent.h
    ECS/Component/CameraComponent.h
    ECS/Component/RenderComponent.h
)

source_group("ECS\\Component\\Public" FILES ${ECS_COMPONENT_HEADERS})
list(APPEND TOTAL_FILES ${ECS_COMPONENT_HEADERS})

set(ECS_COMPONENT_SOURCES
    ECS/Component/NameComponent.cpp
    ECS/Component/TransformComponent.cpp
    ECS/Component/LightComponent.cpp
    ECS/Component/CameraComponent.cpp
    ECS/Component/RenderComponent.cpp
)

source_group("ECS\\Component\\Private" FILES ${ECS_COMPONENT_SOURCES})
list(APPEND TOTAL_FILES ${ECS_COMPONENT_SOURCES})

set(ECS_SYSTEM_HEADERS
    ECS/System/ComponentSystem.h
    ECS/System/NameComponentSystem.h
    ECS/System/TransformComponentSystem.h
    ECS/System/LightComponentSystem.h
    ECS/System/CameraComponentSystem.h
    ECS/System/RenderComponentSystem.h
)

source_group("ECS\\System\\Public" FILES ${ECS_SYSTEM_HEADERS})
list(APPEND TOTAL_FILES ${ECS_SYSTEM_HEADERS})

set(ECS_SYSTEM_SOURCES
    ECS/System/NameComponentSystem.cpp
    ECS/System/TransformComponentSystem.cpp
    ECS/System/LightComponentSystem.cpp
    ECS/System/CameraComponentSystem.cpp
    ECS/System/RenderComponentSystem.cpp
)

source_group("ECS\\System\\Private" FILES ${ECS_SYSTEM_SOURCES})
list(APPEND TOTAL_FILES ${ECS_SYSTEM_SOURCES})

set(RHI_HEADERS
    RHI/RHIContext.h
    RHI/RHIDefinition.h
    RHI/RenderHandle.h
    RHI/CommandHandle.h
    RHI/ResourceHandle.h
    RHI/Swapchain.h
)

source_group("RHI\\Common\\Public" FILES ${RHI_HEADERS})
list(APPEND TOTAL_FILES ${RHI_HEADERS})

set(RHI_SOURCES
    RHI/RHIDefinition.cpp
    RHI/RenderHandle.cpp
    RHI/ResourceHandle.cpp
    RHI/CommandHandle.cpp
    RHI/Swapchain.cpp
)

source_group("RHI\\Common\\Private" FILES ${RHI_SOURCES})
list(APPEND TOTAL_FILES ${RHI_SOURCES})

if(APPLE)
    set(RHI_METAL_HEADERS
        RHI/Metal/RHIContextMetal.h
        RHI/Metal/RHIDefinitionMetal.h
        RHI/Metal/RHIUtilityMetal.h
        RHI/Metal/SwapchainMetal.h
        RHI/Metal/RenderHandleMetal.h
        RHI/Metal/CommandHandleMetal.h
        RHI/Metal/ResourceHandleMetal.h
    )

    source_group("RHI\\Metal\\Public" FILES ${RHI_METAL_HEADERS})
    list(APPEND TOTAL_FILES ${RHI_METAL_HEADERS})

    set(RHI_METAL_SOURCES
        RHI/Metal/RHIContextMetal.mm
        RHI/Metal/RHIDefinitionMetal.mm
        RHI/Metal/RHIUtilityMetal.mm
        RHI/Metal/SwapchainMetal.mm
        RHI/Metal/RenderHandleMetal.mm
        RHI/Metal/CommandHandleMetal.mm
        RHI/Metal/ResourceHandleMetal.mm
    )

    source_group("RHI\\Metal\\Private" FILES ${RHI_METAL_SOURCES})
    list(APPEND TOTAL_FILES ${RHI_METAL_SOURCES})

else()
endif()

set(RENDERER_HEADERS
    Renderer/Renderer.h
    Renderer/ForwardRenderer.h
    Renderer/DeferredRenderer.h
    Renderer/RenderTarget.h
    Renderer/RenderDefinition.h
)

source_group("Renderer\\Public" FILES ${RENDERER_HEADERS})
list(APPEND TOTAL_FILES ${RENDERER_HEADERS})

set(RENDERER_SOURCES
    Renderer/Renderer.cpp
    Renderer/ForwardRenderer.cpp
    Renderer/DeferredRenderer.cpp
    Renderer/RenderTarget.cpp
)

source_group("Renderer\\Private" FILES ${RENDERER_SOURCES})
list(APPEND TOTAL_FILES ${RENDERER_SOURCES})

set(OBJECT_HEADERS
    Object/Object.h
    Object/Material.h
    Object/Mesh.h
    Object/Image.h
)

source_group("Object\\Public" FILES ${OBJECT_HEADERS})
list(APPEND TOTAL_FILES ${OBJECT_HEADERS})
set(OBJECT_SOURCES
    Object/Object.cpp
    Object/Material.cpp
    Object/Mesh.cpp
    Object/Image.cpp
)

source_group("Object\\Private" FILES ${OBJECT_SOURCES})
list(APPEND TOTAL_FILES ${OBJECT_SOURCES})

set(UTILITY_HEADERS
    Utility/ResourceManager.h
    Utility/Timer.h
    Utility/Geometry.h
    Utility/Color.h
    Utility/Hash.h
    Utility/Image.h
    Utility/Mesh.h
)

source_group("Utility\\Public" FILES ${UTILITY_HEADERS})
list(APPEND TOTAL_FILES ${UTILITY_HEADERS})
set(UTILITY_SOURCES
    Utility/ResourceManager.cpp
    Utility/Timer.cpp
    Utility/Geometry.cpp
    Utility/Color.cpp
    Utility/Hash.cpp
    Utility/Image.cpp
    Utility/Mesh.cpp
)

source_group("Utility\\Private" FILES ${UTILITY_SOURCES})

list(APPEND TOTAL_FILES ${UTILITY_SOURCES})

set(RENDERER_PASS_HEADERS
    RendererPass/RendererPass.h
)

source_group("RendererPass\\Public" FILES ${RENDERER_PASS_HEADERS})

list(APPEND TOTAL_FILES ${RENDERER_PASS_HEADERS})
set(RENDERER_PASS_SOURCES
    RendererPass/RendererPass.cpp
)

source_group("RendererPass\\Private" FILES ${RENDERER_PASS_SOURCES})
list(APPEND TOTAL_FILES ${RENDERER_PASS_SOURCES})

set(FORWARD_PASS_HEADERS
    RendererPass/Forward/ForwardPass.h
    RendererPass/Forward/ForwardOpaquePass.h
)

source_group("RendererPass\\Forward\\Public" FILES ${FORWARD_PASS_HEADERS})
list(APPEND TOTAL_FILES ${FORWARD_PASS_HEADERS})

set(FORWARD_PASS_SOURCES
    RendererPass/Forward/ForwardPass.cpp
    RendererPass/Forward/ForwardOpaquePass.cpp
)

source_group("RendererPass\\Forward\\Private" FILES ${FORWARD_PASS_SOURCES})
list(APPEND TOTAL_FILES ${FORWARD_PASS_SOURCES})

set(THIRDPARTY_IMGUI
    ThirdParty/ImGui/imconfig.h
    ThirdParty/ImGui/imgui_internal.h
    ThirdParty/ImGui/imgui.h
    ThirdParty/ImGui/imstb_rectpack.h
    ThirdParty/ImGui/imstb_textedit.h
    ThirdParty/ImGui/imstb_truetype.h
    ThirdParty/ImGui/imgui.cpp
    ThirdParty/ImGui/imgui_demo.cpp
    ThirdParty/ImGui/imgui_draw.cpp
    ThirdParty/ImGui/imgui_tables.cpp
    ThirdParty/ImGui/imgui_widgets.cpp
)

source_group("ThirdParty\\ImGui" FILES ${THIRDPARTY_IMGUI})

set(THIRDPARTY_IMGUI_BACKENDS
    ThirdParty/ImGui/backends/imgui_impl_metal.h
    ThirdParty/ImGui/backends/imgui_impl_metal.mm
    ThirdParty/ImGui/backends/imgui_impl_osx.h
    ThirdParty/ImGui/backends/imgui_impl_osx.mm
    ThirdParty/ImGui/backends/imgui_impl_sdl3.h
    ThirdParty/ImGui/backends/imgui_impl_sdl3.cpp
)

source_group("ThirdParty\\ImGui\\backends" FILES ${THIRDPARTY_IMGUI_BACKENDS})

list(APPEND THIRDPARTY_IMGUI
    ${THIRDPARTY_IMGUI_BACKENDS}
)

set(THIRDPARTY
    ${THIRDPARTY_IMGUI}
)

list(APPEND TOTAL_FILES ${THIRDPARTY})

set(SHADER_LAYOUT ${HS_RES_SHADER_DIR}/MSL/BuiltInMaterialLayout.h)

source_group("Renderer\\Reference" FILES ${SHADER_LAYOUT})
list(APPEND TOTAL_FILES ${SHADER_LAYOUT})

add_library(${TARGET_NAME} STATIC
    ${TOTAL_FILES}
)

set(INCLUDE_DIRECTORIES)

target_include_directories(${TARGET_NAME}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC ThirdParty ${HS_RES_SHADER_DIR}
)

add_dependencies(${TARGET_NAME} Shader)

if(APPLE)
    target_link_libraries(${TARGET_NAME}
        PUBLIC
        ${SDL3}
        PRIVATE
        "-framework OpenGL -framework Foundation -framework CoreFoundation -framework QuartzCore -framework AppKit -framework IOKit -framework Metal -framework MetalKit -framework GameController"
    )
else()
    target_link_libraries(${TARGET_NAME}
        PUBLIC
        ${SDL3}
    )
endif()

# set_property(TARGET ${TARGET_NAME} PROPERTY XCODE_GENERATE_DEBUGGING_SYMBOLS ON)
target_compile_definitions(${TARGET_NAME} PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:MinSizeRel>:_RELEASE>
    $<$<CONFIG:Release>:_RELEASE>
    $<$<CONFIG:RelWithDebInfo>:_RELWITHDEBINFO>
    HS_ENGINE
)
