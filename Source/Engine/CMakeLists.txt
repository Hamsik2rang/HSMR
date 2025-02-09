set(TARGET_NAME Engine)

set(TOTAL_FILES)

set(CORE_HEADERS
    Core/Application.h
    Core/Scene.h
    Core/Log.h
    Core/EngineContext.h
    Core/FileSystem.h
    Core/Window.h
    Core/Event.h
)

source_group("Core\\Public" FILES ${CORE_HEADERS})
list(APPEND TOTAL_FILES ${CORE_HEADERS})

set(CORE_SOURCES
    Core/Application.mm
    Core/Scene.cpp
    Core/Log.cpp
    Core/EngineContext.cpp
    Core/FileSystem.cpp
    Core/Window.cpp
    Core/Event.cpp
)

source_group("Core\\Private" FILES ${CORE_SOURCES})
list(APPEND TOTAL_FILES ${CORE_SOURCES})

set(RENDERER_HEADERS
    Renderer/Renderer.h
    Renderer/RenderDefinition.h
    Renderer/Swapchain.h
    Renderer/RenderPass.h
    Renderer/Framebuffer.h
    Renderer/Pipeline.h
    Renderer/CommandHandle.h
    Renderer/RHIUtility.h
)

source_group("Renderer\\Public" FILES ${RENDERER_HEADERS})
list(APPEND TOTAL_FILES ${RENDERER_HEADERS})

set(RENDERER_SOURCES
    Renderer/Renderer.mm
    Renderer/RenderDefinition.mm
    Renderer/Swapchain.mm
    Renderer/CommandHandle.mm
    Renderer/RenderPass.mm
    Renderer/Framebuffer.mm
    Renderer/Pipeline.mm
    Renderer/RHIUtility.mm
)

source_group("Renderer\\Private" FILES ${RENDERER_SOURCES})
list(APPEND TOTAL_FILES ${RENDERER_SOURCES})
set(OBJECT_HEADERS
    Object/Object.h
    Object/Material.h
    Object/Mesh.h
    Object/Image.h
    Object/Shader.h
)

source_group("Object\\Public" FILES ${OBJECT_HEADERS})
list(APPEND TOTAL_FILES ${OBJECT_HEADERS})
set(OBJECT_SOURCES
    Object/Object.cpp
    Object/Material.cpp
    Object/Mesh.cpp
    Object/Image.cpp
    Object/Shader.cpp
)

source_group("Object\\Private" FILES ${OBJECT_SOURCES})
list(APPEND TOTAL_FILES ${OBJECT_SOURCES})

set(UTILITY_HEADERS
    Utility/ResourceManager.h
    Utility/Timer.h
    Utility/Geometry.h
    Utility/Color.h
)

source_group("Utility\\Public" FILES ${UTILITY_HEADERS})
list(APPEND TOTAL_FILES ${UTILITY_HEADERS})
set(UTILITY_SOURCES
    Utility/ResourceManager.cpp
    Utility/Timer.cpp
    Utility/Geometry.cpp
    Utility/Color.cpp
)

source_group("Utility\\Private" FILES ${UTILITY_SOURCES})
list(APPEND TOTAL_FILES ${UTILITY_SOURCES})
set(RENDERER_PASS_HEADERS
    RendererPass/RendererPass.h
    RendererPass/DebugPass.h
)

source_group("RendererPass\\Public" FILES ${RENDERER_PASS_HEADERS})
list(APPEND TOTAL_FILES ${RENDERER_PASS_HEADERS})
set(RENDERER_PASS_SOURCES
    RendererPass/RendererPass.mm
    RendererPass/DebugPass.mm
)

source_group("RendererPass\\Private" FILES ${RENDERER_PASS_SOURCES})
list(APPEND TOTAL_FILES ${RENDERER_PASS_SOURCES})
set(FORWARD_PASS_HEADERS
    RendererPass/ForwardPass/ForwardPass.h
    RendererPass/ForwardPass/ForwardOpaquePass.h
)

source_group("RendererPass\\ForwardPass\\Public" FILES ${FORWARD_PASS_HEADERS})
list(APPEND TOTAL_FILES ${FORWARD_PASS_HEADERS})
set(FORWARD_PASS_SOURCES
    RendererPass/ForwardPass/ForwardPass.mm
    RendererPass/ForwardPass/ForwardOpaquePass.mm
)

source_group("RendererPass\\ForwardPass\\Private" FILES ${FORWARD_PASS_SOURCES})
list(APPEND TOTAL_FILES ${FORWARD_PASS_SOURCES})
list(APPEND RENDERER_PASS_HEADERS
    ${FORWARD_PASS_HEADERS}
)

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

target_include_directories(${TARGET_NAME}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC ThirdParty ${HS_RES_SHADER_DIR}
)

add_dependencies(${TARGET_NAME} Shader)

target_link_libraries(${TARGET_NAME}
    PUBLIC
    ${SDL3}
    PRIVATE
    "-framework OpenGL -framework Foundation -framework CoreFoundation -framework QuartzCore -framework AppKit -framework IOKit -framework Metal -framework GameController"
)

# set_property(TARGET ${TARGET_NAME} PROPERTY XCODE_GENERATE_DEBUGGING_SYMBOLS ON)
target_compile_definitions(${TARGET_NAME} PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:MinSizeRel>:_RELEASE>
    $<$<CONFIG:Release>:_RELEASE>
    $<$<CONFIG:RelWithDebInfo>:_RELWITHDEBINFO>
    HS_ENGINE
)
