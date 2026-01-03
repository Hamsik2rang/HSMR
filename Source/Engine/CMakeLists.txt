set(TARGET_NAME Engine)

if(APPLE)
    set(ASSIMP ${HS_DEPS_DLL_DIR}/libassimp.dylib)
    set(STB_INCLUDE ${HS_DEPS_INCLUDE_DIR}/stb)
else()
    set(ASSIMP ${HS_DEPS_LIB_DIR}/assimp-vc143-mt.lib)
    set(STB_INCLUDE ${HS_DEPS_INCLUDE_DIR}/stb)
endif()

set(TOTAL_FILES)
set(ENGINE_COMMON_HEADERS
    Application.h
    EngineContext.h
    Window.h
)

source_group("Public" FILES ${ENGINE_COMMON_HEADERS})
list(APPEND TOTAL_FILES ${ENGINE_COMMON_HEADERS})

set(ENGINE_COMMON_SOURCES
    Private/Application.cpp
    Private/Window.cpp
    Private/EngineContext.cpp
)

source_group("Private" FILES ${ENGINE_COMMON_SOURCES})
list(APPEND TOTAL_FILES ${ENGINE_COMMON_SOURCES})

set(ENGINE_GEOMETRY_HEADERS
    Geometry/GeometryDefinition.h
)
source_group("Geometry\\Public" FILES ${ENGINE_GEOMETRY_HEADERS})
list(APPEND TOTAL_FILES ${ENGINE_GEOMETRY_HEADERS})

set(ENGINE_GEOMETRY_SOURCES
    Geometry/Private/GeometryDefinition.cpp
)
source_group("Geometry\\Private" FILES ${ENGINE_GEOMETRY_SOURCES})
list(APPEND TOTAL_FILES ${ENGINE_GEOMETRY_SOURCES})

set(ENGINE_RESOURCE_HEADERS
    Resource/ResourceDefinition.h
    Resource/Image.h
    Resource/Material.h
    Resource/Mesh.h
    Resource/Shader.h
    Resource/Object.h
    Resource/ObjectManager.h
)

source_group("Resource\\Public" FILES ${ENGINE_RESOURCE_HEADERS})
list(APPEND TOTAL_FILES ${ENGINE_RESOURCE_HEADERS})

set(ENGINE_RESOURCE_SOURCES
    Resource/Private/ResourceDefinition.cpp
    Resource/Private/ObjectManager.cpp
    Resource/Private/Image.cpp
    Resource/Private/Material.cpp
    Resource/Private/Mesh.cpp
    Resource/Private/Shader.cpp
    Resource/Private/Object.cpp
)

source_group("Resource\\Private" FILES ${ENGINE_RESOURCE_SOURCES})
list(APPEND TOTAL_FILES ${ENGINE_RESOURCE_SOURCES})

set(ENGINE_RENDERER_HEADERS
    Renderer/DeferredPath.h
    Renderer/ForwardPath.h
    Renderer/RenderPath.h
    Renderer/RendererDefinition.h
    Renderer/RenderTarget.h
)

source_group("Renderer\\Public" FILES ${ENGINE_RENDERER_HEADERS})
list(APPEND TOTAL_FILES ${ENGINE_RENDERER_HEADERS})

set(ENGINE_RENDERER_SOURCES
    Renderer/Private/DeferredPath.cpp
    Renderer/Private/ForwardPath.cpp
    Renderer/Private/RenderPath.cpp
    Renderer/Private/RenderTarget.cpp
)

source_group("Renderer\\Private" FILES ${ENGINE_RENDERER_SOURCES})
list(APPEND TOTAL_FILES ${ENGINE_RENDERER_SOURCES})

set(ENGINE_RENDERER_RENDER_PASS_HEADERS
    Renderer/RenderPass/RenderPass.h
    Renderer/RenderPass/ForwardRenderPass.h
    Renderer/RenderPass/ForwardOpaquePass.h
)

source_group("Renderer\\RenderPass\\Public" FILES ${ENGINE_RENDERER_RENDER_PASS_HEADERS})
list(APPEND TOTAL_FILES ${ENGINE_RENDERER_RENDER_PASS_HEADERS})

set(ENGINE_RENDERER_RENDER_PASS_SOURCES
    Renderer/RenderPass/Private/RenderPass.cpp
    Renderer/RenderPass/Private/ForwardRenderPass.cpp
    Renderer/RenderPass/Private/ForwardOpaquePass.cpp
)

source_group("Renderer\\RenderPass\\Private" FILES ${ENGINE_RENDERER_RENDER_PASS_SOURCES})
list(APPEND TOTAL_FILES ${ENGINE_RENDERER_RENDER_PASS_SOURCES})

set(ENGINE_RENDERER_ATMOSPHERE_HEADERS
    Renderer/Atmosphere/AtmosphereParameters.h
    Renderer/Atmosphere/AtmospherePrecompute.h
    Renderer/Atmosphere/AtmosphereRenderer.h
    Renderer/Atmosphere/AtmosphereSkyPass.h
)

source_group("Renderer\\Atmosphere\\Public" FILES ${ENGINE_RENDERER_ATMOSPHERE_HEADERS})
list(APPEND TOTAL_FILES ${ENGINE_RENDERER_ATMOSPHERE_HEADERS})

set(ENGINE_RENDERER_ATMOSPHERE_SOURCES
    Renderer/Atmosphere/Private/AtmospherePrecompute.cpp
    Renderer/Atmosphere/Private/AtmosphereRenderer.cpp
    Renderer/Atmosphere/Private/AtmosphereSkyPass.cpp
)

source_group("Renderer\\Atmosphere\\Private" FILES ${ENGINE_RENDERER_ATMOSPHERE_SOURCES})
list(APPEND TOTAL_FILES ${ENGINE_RENDERER_ATMOSPHERE_SOURCES})

add_library(${TARGET_NAME} SHARED ${TOTAL_FILES})

if(APPLE)
    list(APPEND OSX_FRAMEWORK "-framework OpenGL -framework Foundation -framework CoreFoundation -framework QuartzCore -framework AppKit -framework IOKit -framework Metal -framework GameController")
    target_link_libraries(${TARGET_NAME}
        PRIVATE
        "-Wl, -all_load" Core Platform RHI
        "-Wl, -force_load" ${ASSIMP} ${OSX_FRAMEWORK}
    )
else()
    target_link_libraries(${TARGET_NAME}
        PRIVATE
Core Platform RHI
        ${ASSIMP}
    )
endif()

if(WIN32)
    target_link_options(${TARGET_NAME}
        PRIVATE
        "/WHOLEARCHIVE:Core"
        "/WHOLEARCHIVE:Platform"
        "/WHOLEARCHIVE:RHI"
    )
endif()

target_include_directories(${TARGET_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR} ${STB_INCLUDE}
)

target_compile_definitions(${TARGET_NAME}
    PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:MinSizeRel>:_RELEASE>
    $<$<CONFIG:Release>:_RELEASE>
    $<$<CONFIG:RelWithDebInfo>:_RELWITHDEBINFO>
    HS_ENGINE

    HS_API_EXPORT
)

install(TARGETS ${TARGET_NAME}
    ARCHIVE DESTINATION "${INSTALL_FOLDER}/dev/lib"
    LIBRARY DESTINATION "${INSTALL_FOLDER}"
    RUNTIME DESTINATION "${INSTALL_FOLDER}"
)

hs_auto_install(${TARGET_NAME})
