set(TARGET_NAME Editor)

set(TOTAL_FILES)

set(EDITOR_CORE_HEADERS
    Core/EditorApplication.h
    Core/EditorWindow.h
    Core/EditorCamera.h
    Core/EditorContext.h
    Core/SimpleApplication.h
    Core/SimpleWindow.h
)

source_group("Core\\Public" FILES ${EDITOR_CORE_HEADERS})

list(APPEND TOTAL_FILES ${EDITOR_CORE_HEADERS})

set(EDITOR_CORE_SOURCES
    Core/EditorApplication.cpp
    Core/EditorWindow.cpp
    Core/EditorCamera.cpp
    Core/EditorContext.cpp
    Core/SimpleApplication.cpp
    Core/SimpleWindow.cpp
)

source_group("Core\\Private" FILES ${EDITOR_CORE_SOURCES})

list(APPEND TOTAL_FILES ${EDITOR_CORE_SOURCES})

set(EDITOR_ENTRY_POINT_HEADERS
    EntryPoint/EditorMain.h
    EntryPoint/SimpleMain.h
)

source_group("EntryPoint\\Public" FILES ${EDITOR_ENTRY_POINT_HEADERS})

list(APPEND TOTAL_FILES ${EDITOR_ENTRY_POINT_HEADERS})

if(APPLE)
    set(EDITOR_ENTRY_POINT_SOURCES
        EntryPoint/OSX/EditorMain.mm
        EntryPoint/OSX/SimpleMain.mm
    )

else()
    set(EDITOR_ENTRY_POINT_SOURCES
        EntryPoint/Windows/EditorMain.cpp
        EntryPoint/Windows/SimpleMain.cpp
    )
endif()

source_group("EntryPoint\\Private" FILES ${EDITOR_ENTRY_POINT_SOURCES})

list(APPEND TOTAL_FILES ${EDITOR_ENTRY_POINT_SOURCES})

set(EDITOR_GUI_HEADERS
    GUI/EditorIcons.h
    GUI/GUIContext.h
    GUI/ImGuiExtension.h
    GUI/MaterialSymbolsIcons.h
    GUI/GUIRenderer.h
    GUI/GUIRendererPass.h
)

source_group("GUI\\Public" FILES ${EDITOR_GUI_HEADERS})

list(APPEND TOTAL_FILES ${EDITOR_GUI_HEADERS})

set(EDITOR_GUI_SOURCES
    GUI/GUIContext.cpp
    GUI/GUIRenderer.cpp
    GUI/GUIRendererPass.cpp
)

list(APPEND TOTAL_FILES ${EDITOR_GUI_SOURCES})

source_group("GUI\\Private" FILES ${EDITOR_GUI_SOURCES})

if(APPLE)
    set(EDITOR_GUI_SOURCES_METAL
        GUI/Metal/ImGuiExtensionMetal.mm
    )

    list(APPEND TOTAL_FILES ${EDITOR_GUI_SOURCES_METAL})
    source_group("GUI\\Private\\Metal" FILES ${EDITOR_GUI_SOURCES_METAL})

else()
    set(EDITOR_GUI_SOURCES_VULKAN
        GUI/Vulkan/ImGuiExtensionVulkan.cpp
    )

    list(APPEND TOTAL_FILES ${EDITOR_GUI_SOURCES_VULKAN})
    source_group("GUI\\Private\\Vulkan" FILES ${EDITOR_GUI_SOURCES_VULKAN})
endif()

set(EDITOR_RESOURCE_HEADERS
    Resource/ShaderWatcher.h
)

list(APPEND TOTAL_FILES ${EDITOR_RESOURCE_HEADERS})
source_group("Resource\\Public" FILES ${EDITOR_RESOURCE_HEADERS})

set(EDITOR_RESOURCE_SOURCES
    Resource/Private/ShaderWatcher.cpp
)

list(APPEND TOTAL_FILES ${EDITOR_RESOURCE_SOURCES})
source_group("Resource\\Private" FILES ${EDITOR_RESOURCE_SOURCES})

set(EDITOR_ASSET_HEADERS
    Asset/AssetTypes.h
    Asset/AssetDatabase.h
)

list(APPEND TOTAL_FILES ${EDITOR_ASSET_HEADERS})
source_group("Asset\\Public" FILES ${EDITOR_ASSET_HEADERS})

set(EDITOR_ASSET_SOURCES
    Asset/AssetDatabase.cpp
)

list(APPEND TOTAL_FILES ${EDITOR_ASSET_SOURCES})
source_group("Asset\\Private" FILES ${EDITOR_ASSET_SOURCES})

set(EDITOR_PROJECT_HEADERS
    Project/ProjectContext.h
    Project/RecentProjects.h
    Project/ProjectLauncherWindow.h
)

list(APPEND TOTAL_FILES ${EDITOR_PROJECT_HEADERS})
source_group("Project\\Public" FILES ${EDITOR_PROJECT_HEADERS})

set(EDITOR_PROJECT_SOURCES
    Project/ProjectContext.cpp
    Project/RecentProjects.cpp
    Project/ProjectLauncherWindow.cpp
)

list(APPEND TOTAL_FILES ${EDITOR_PROJECT_SOURCES})
source_group("Project\\Private" FILES ${EDITOR_PROJECT_SOURCES})

set(PANEL_HEADERS
    Panel/Panel.h
    Panel/DockspacePanel.h
    Panel/ScenePanel.h
    Panel/InspectorPanel.h
    Panel/MenuPanel.h
    Panel/ResourcePanel.h
    Panel/SceneStatusPanel.h
    Panel/HierarchyPanel.h
    Panel/ProfilerPanel.h
    Panel/SimpleMenuPanel.h
    Panel/SimpleInspectorPanel.h
)

source_group("Panel\\Public" FILES ${PANEL_HEADERS})

list(APPEND TOTAL_FILES ${PANEL_HEADERS})

set(PANEL_SOURCES
    Panel/Private/Panel.cpp
    Panel/Private/DockspacePanel.cpp
    Panel/Private/ScenePanel.cpp
    Panel/Private/InspectorPanel.cpp
    Panel/Private/MenuPanel.cpp
    Panel/Private/ResourcePanel.cpp
    Panel/Private/SceneStatusPanel.cpp
    Panel/Private/HierarchyPanel.cpp
    Panel/Private/ProfilerPanel.cpp
    Panel/Private/SimpleMenuPanel.cpp
    Panel/Private/SimpleInspectorPanel.cpp
)

source_group("Panel\\Private" FILES ${PANEL_SOURCES})

list(APPEND TOTAL_FILES ${PANEL_SOURCES})

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

set(THIRDPARTY_IMGUI_BACKENDS)

if(APPLE)
    if(HSMR_USE_SDL)
        # macOS + SDL3 + Metal backend
        list(APPEND THIRDPARTY_IMGUI_BACKENDS
            ThirdParty/ImGui/imgui_impl_sdl3.h
            ThirdParty/ImGui/imgui_impl_sdl3.cpp
            ThirdParty/ImGui/imgui_impl_metal.h
            ThirdParty/ImGui/imgui_impl_metal.mm
        )
    else()
        # macOS + Native OSX + Metal backend
        list(APPEND THIRDPARTY_IMGUI_BACKENDS
            ThirdParty/ImGui/imgui_impl_osx.h
            ThirdParty/ImGui/imgui_impl_osx.mm
            ThirdParty/ImGui/imgui_impl_metal.h
            ThirdParty/ImGui/imgui_impl_metal.mm
        )
    endif()
elseif(HSMR_USE_SDL)
    # Windows + SDL3 + Vulkan backend
    list(APPEND THIRDPARTY_IMGUI_BACKENDS
        ThirdParty/ImGui/imgui_impl_sdl3.h
        ThirdParty/ImGui/imgui_impl_sdl3.cpp
        ThirdParty/ImGui/imgui_impl_vulkan.h
        ThirdParty/ImGui/imgui_impl_vulkan.cpp
    )
else()
    # Windows + Native Win32 + Vulkan backend
    list(APPEND THIRDPARTY_IMGUI_BACKENDS
        ThirdParty/ImGui/imgui_impl_win32.h
        ThirdParty/ImGui/imgui_impl_win32.cpp
        ThirdParty/ImGui/imgui_impl_vulkan.h
        ThirdParty/ImGui/imgui_impl_vulkan.cpp
    )
endif()

source_group("ThirdParty\\ImGui\\backends" FILES ${THIRDPARTY_IMGUI_BACKENDS})

list(APPEND THIRDPARTY_IMGUI
    ${THIRDPARTY_IMGUI_BACKENDS}
)

set(THIRDPARTY_IMGUIZMO
    ThirdParty/ImGuizmo/ImGuizmo.cpp
)

source_group("ThirdParty\\ImGuizmo" FILES ${THIRDPARTY_IMGUIZMO})

set(THIRDPARTY_IMPLOT
    ThirdParty/ImPlot/implot.cpp
    ThirdParty/ImPlot/implot_items.cpp
)

source_group("ThirdParty\\ImPlot" FILES ${THIRDPARTY_IMPLOT})

set(THIRDPARTY
    ${THIRDPARTY_IMGUI}
    ${THIRDPARTY_IMGUIZMO}
    ${THIRDPARTY_IMPLOT}
)

list(APPEND TOTAL_FILES ${THIRDPARTY})

add_library(${TARGET_NAME}
    SHARED
    ${TOTAL_FILES}
)

if(WIN32)
    target_link_libraries(${TARGET_NAME}
        PRIVATE
        Core RHI Engine Resource Renderer Scene
        ${HS_DEPS_LIB_DIR}/vulkan-1.lib
        $<$<BOOL:${HSMR_USE_SDL}>:${HS_SDL3_LIB}>
    )
else()
    list(APPEND OSX_FRAMEWORK "-framework OpenGL -framework Foundation -framework CoreFoundation -framework QuartzCore -framework AppKit -framework IOKit -framework Metal -framework GameController")

    # For SDL3 on macOS, link against the framework
    if(HSMR_USE_SDL)
        target_link_libraries(${TARGET_NAME}
            PRIVATE
            "-Wl, -all_load" Core RHI Engine Resource Renderer Scene
            ${OSX_FRAMEWORK}
            "-F${HS_SDL3_FRAMEWORK_DIR}/.." "-framework SDL3"
        )
    else()
        target_link_libraries(${TARGET_NAME}
            PRIVATE
            "-Wl, -all_load" Core RHI Engine Resource Renderer Scene
            ${OSX_FRAMEWORK}
        )
    endif()
endif()

target_include_directories(${TARGET_NAME}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../Engine
    PUBLIC ThirdParty
    $<$<BOOL:${HSMR_USE_SDL}>:${HS_SDL3_INCLUDE_DIR}>
    ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/ImGuizmo
    ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/ImPlot
    ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/ImGui
)

set_target_properties(${TARGET_PATH} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${HS_EXECUTABLE_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${HS_EXECUTABLE_DIR}
)

add_dependencies(${TARGET_NAME} Engine)

target_compile_definitions(${TARGET_NAME}
    PUBLIC
    HS_EDITOR_MODE

    PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:MinSizeRel>:_RELEASE>
    $<$<CONFIG:Release>:_RELEASE>
    $<$<CONFIG:RelWithDebInfo>:_RELWITHDEBINFO>

    HS_EDITOR
    HS_EDITOR_API_EXPORT

    HS_API_IMPORT
    HS_CORE_API_IMPORT
    HS_RHI_API_IMPORT
    HS_RESOURCE_API_IMPORT
    HS_RENDERER_API_IMPORT
    HS_SHADER_SYSTEM_API_IMPORT
    HS_SCENE_API_IMPORT

    HAS_IMGUIZMO=1
)

if(APPLE)
    # On macOS, copy the .dylib file
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:${TARGET_NAME}>"
        "${INSTALL_FOLDER}/$<TARGET_FILE_NAME:${TARGET_NAME}>"
        COMMENT "Copying ${TARGET_NAME} library to install folder: ${INSTALL_FOLDER}"
    )
else()
    # On Windows, copy both .dll and .lib files
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:${TARGET_NAME}>"
        "${INSTALL_FOLDER}/$<TARGET_FILE_NAME:${TARGET_NAME}>"
        COMMENT "Copying ${TARGET_NAME}.dll to install folder: ${INSTALL_FOLDER}"
    )

    # Also copy the import library (.lib) if it exists
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_LINKER_FILE:${TARGET_NAME}>"
        "${INSTALL_FOLDER}/dev/lib/$<TARGET_LINKER_FILE_NAME:${TARGET_NAME}>"
        COMMENT "Copying ${TARGET_NAME}.lib to install folder: ${INSTALL_FOLDER}/dev/lib"
    )

    # Create dev/lib directory for .lib file
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${INSTALL_FOLDER}/dev/lib"
        COMMENT "Creating dev/lib directory: ${INSTALL_FOLDER}/dev/lib"
    )
endif()

# Optional: Copy PDB files for debugging on Windows
if(WIN32 AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_PDB_FILE:${TARGET_NAME}>"
        "${INSTALL_FOLDER}/$<TARGET_PDB_FILE_NAME:${TARGET_NAME}>"
        COMMENT "Copying ${TARGET_NAME}.pdb to install folder: ${INSTALL_FOLDER}"
    )
endif()

install(TARGETS ${TARGET_NAME}
    ARCHIVE DESTINATION "${INSTALL_FOLDER}/dev/lib"
    LIBRARY DESTINATION "${INSTALL_FOLDER}"
    RUNTIME DESTINATION "${INSTALL_FOLDER}")
