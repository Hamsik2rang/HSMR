set(TARGET_NAME Engine)

set(TOTAL_FILES)

set(CORE_HEADERS
    Core/Application.h
    Core/Scene.h
    Core/Log.h
    Core/EngineContext.h
    Core/FileSystem.h
    Core/Window.h
    Core/Swapchain.h
    Core/Event.h
)

source_group("Core\\Public" FILES ${CORE_HEADERS})
list(APPEND TOTAL_FILES ${CORE_HEADERS})

set(CORE_SOURCES
    Core/Application.cpp
    Core/Scene.cpp
    Core/Log.cpp
    Core/EngineContext.cpp
    Core/Swapchain.cpp
    Core/FileSystem.cpp
    Core/Window.cpp
    Core/Event.cpp
)

source_group("Core\\Private" FILES ${CORE_SOURCES})
list(APPEND TOTAL_FILES ${CORE_SOURCES})

set(RHI_HEADERS
    RHI/RHIContext.h
    RHI/RHIDefinition.h
    RHI/RenderPass.h
    RHI/Framebuffer.h
    RHI/Pipeline.h
    RHI/CommandHandle.h
    RHI/ResourceHandle.h
)

source_group("RHI\\Common\\Public" FILES ${RHI_HEADERS})
list(APPEND TOTAL_FILES ${RHI_HEADERS})

set(RHI_SOURCES
    RHI/RHIDefinition.cpp
)

source_group("RHI\\Common\\Private" FILES ${RHI_SOURCES})
list(APPEND TOTAL_FILES ${RHI_SOURCES})

if(APPLE)
    set(RHI_METAL_HEADERS
        RHI/Metal/RHIContextMetal.h
        RHI/Metal/RHIDefinitionMetal.h
        RHI/Metal/RHIUtilityMetal.h
        RHI/Metal/SwapchainMetal.h
        RHI/Metal/RenderPassMetal.h
        RHI/Metal/FramebufferMetal.h
        RHI/Metal/PipelineMetal.h
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
        RHI/Metal/RenderPassMetal.mm
        RHI/Metal/FramebufferMetal.mm
        RHI/Metal/PipelineMetal.mm
        RHI/Metal/CommandHandleMetal.mm
        RHI/Metal/ResourceHandleMetal.mm
    )

    source_group("RHI\\Metal\\Private" FILES ${RHI_METAL_SOURCES})
    list(APPEND TOTAL_FILES ${RHI_METAL_SOURCES})

else()
endif()

set(RENDERER_HEADERS
    Renderer/Renderer.h
    Renderer/RenderTarget.h
    Renderer/RenderDefinition.h
)

source_group("Renderer\\Public" FILES ${RENDERER_HEADERS})
list(APPEND TOTAL_FILES ${RENDERER_HEADERS})

set(RENDERER_SOURCES
    Renderer/Renderer.cpp
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

# if(APPLE)
# set(METAL_CPP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Thirdparty/metal-cpp)

# set(METAL_CPP_HEADERS
# "${METAL_CPP_DIR}/Foundation/Foundation.hpp"
# "${METAL_CPP_DIR}/QuartzCore/QuartzCore.hpp"
# "${METAL_CPP_DIR}/Metal/Metal.hpp"
# )

# option(MAKE_METAL_SINGLE_HEADER "use metal-cpp as a single header" OFF)

# if(MAKE_METAL_SINGLE_HEADER)
# add_compile_definitions(SINGLE_METAL_HEADER)

# # Python required to run the metal-cpp/SingleHeader/MakeSingleHeader.py
# find_package(Python3 REQUIRED COMPONENTS Interpreter)

# set(MAKE_SINGLE_HEADER_SCRIPT_PY "${METAL_CPP_DIR}/SingleHeader/MakeSingleHeader.py")

# if(EXISTS ${MAKE_SINGLE_HEADER_SCRIPT_PY})
# # if(INCLUDE_APPKIT_EXTENSION)
# # file(COPY "${METAL_CPP_EXTENSIONS_DIR}/AppKit"
# # DESTINATION "${METAL_CPP_DIR}/")
# # list(APPEND METAL_CPP_HEADERS "${METAL_CPP_DIR}/AppKit/AppKit.hpp")
# # endif()

# # if(INCLUDE_METALKIT_EXTENSION)
# # file(COPY "${METAL_CPP_EXTENSIONS_DIR}/MetalKit"
# # DESTINATION "${METAL_CPP_DIR}/")
# # list(APPEND METAL_CPP_HEADERS "${METAL_CPP_DIR}/MetalKit/MetalKit.hpp")
# # endif()
# message(STATUS "run metal-cpp single header script...")

# # Run the python script.
# execute_process(
# COMMAND python3 ${MAKE_SINGLE_HEADER_SCRIPT_PY} ${HEADERS}
# WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
# RESULT_VARIABLE EXIT_CODE
# OUTPUT_QUIET
# )
# endif()
# endif()

# if(${MAKE_METAL_SINGLE_HEADER} AND EXISTS "${METAL_CPP_DIR}/SingleHeader/Metal.hpp")
# set(METAL_CPP_INCLUDE_DIRS "${METAL_CPP_DIR}/SingleHeader")
# set(METAL_CPP_HEADERS ${METAL_CPP_DIR}/SingleHeader/Metal.hpp)

# # Remove unnecessary  headers.
# execute_process(
# COMMAND python3 "${CMAKE_CURRENT_LIST_DIR}/scripts/RemoveUselessImports.py"
# "${METAL_CPP_DIR}/SingleHeader/Metal.hpp"
# WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
# RESULT_VARIABLE EXIT_CODE2
# OUTPUT_QUIET)

# if(NOT ${EXIT_CODE1} EQUAL 0 OR NOT ${EXIT_CODE2} EQUAL 0)
# message(
# FATAL_ERROR "An error has ocurred: the script did not successfully complete.")
# else()
# message(STATUS "Single header created: ${METAL_CPP_DIR}/SingleHeader/Metal.hpp")
# endif()

# else(NOT ${MAKE_METAL_SINGLE_HEADER})
# list(APPEND METAL_CPP_INCLUDE_DIRS ${METAL_CPP_DIR})

# # if(INCLUDE_APPKIT_EXTENSION OR INCLUDE_METALKIT_EXTENSION)
# # list(APPEND METAL_CPP_INCLUDE_DIRS ${METAL_CPP_EXTENSIONS_DIR})
# # endif()
# endif()

# source_group("ThirdParty\\metal-cpp" FILES ${METAL_CPP_HEADERS})
# list(APPEND THIRDPARTY ${METAL_CPP_HEADERS})
# endif()
list(APPEND TOTAL_FILES ${THIRDPARTY})

set(SHADER_LAYOUT ${HS_RES_SHADER_DIR}/MSL/BuiltInMaterialLayout.h)

source_group("Renderer\\Reference" FILES ${SHADER_LAYOUT})
list(APPEND TOTAL_FILES ${SHADER_LAYOUT})

add_library(${TARGET_NAME} STATIC
    ${TOTAL_FILES}
)

set(INCLUDE_DIRECTORIES)

if(APPLE)
    target_include_directories(${TARGET_NAME}
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
        PUBLIC ThirdParty ThirdParty/metal-cpp ${HS_RES_SHADER_DIR}
    )
else()
endif()

add_dependencies(${TARGET_NAME} Shader)

target_link_libraries(${TARGET_NAME}
    PUBLIC
    ${SDL3}
    PRIVATE
    "-framework OpenGL -framework Foundation -framework CoreFoundation -framework QuartzCore -framework AppKit -framework IOKit -framework Metal -framework MetalKit -framework GameController"
)

# set_property(TARGET ${TARGET_NAME} PROPERTY XCODE_GENERATE_DEBUGGING_SYMBOLS ON)
target_compile_definitions(${TARGET_NAME} PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:MinSizeRel>:_RELEASE>
    $<$<CONFIG:Release>:_RELEASE>
    $<$<CONFIG:RelWithDebInfo>:_RELWITHDEBINFO>
    HS_ENGINE
)
