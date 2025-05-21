set(TARGET_NAME Editor)

set(TOTAL_FILES)

set(CORE_HEADERS
    Core/EditorApplication.h
    Core/EditorWindow.h
    Core/EditorCamera.h
)

source_group("Core\\Public" FILES ${CORE_HEADERS})

list(APPEND TOTAL_FILES ${CORE_HEADERS})

set(CORE_SOURCES
    Core/EditorApplication.cpp
    Core/EditorWindow.cpp
    Core/EditorCamera.cpp
)

source_group("Core\\Private" FILES ${CORE_SOURCES})

list(APPEND TOTAL_FILES ${CORE_SOURCES})

set(ENTRY_POINT_HEADERS
    EntryPoint/EditorMain.h
)

source_group("EntryPoint\\Public" FILES ${ENTRY_POINT_HEADERS})

list(APPEND TOTAL_FILES ${ENTRY_POINT_HEADERS})

if(APPLE)
    set(ENTRY_POINT_SOURCES
        EntryPoint/OSX/EditorMain.mm
    )

else()
    set(ENTRY_POINT_SOURCES
        EntryPoint/Windows/EditorMain.cpp
    )
endif()

source_group("EntryPoint\\Private" FILES ${ENTRY_POINT_SOURCES})

list(APPEND TOTAL_FILES ${ENTRY_POINT_SOURCES})

set(GUI_HEADERS
    GUI/GUIContext.h
    GUI/ImGuiExtension.h
)

source_group("GUI\\Public" FILES ${GUI_HEADERS})

list(APPEND TOTAL_FILES ${GUI_HEADERS})

set(GUI_SOURCES
    GUI/GUIContext.cpp
)

list(APPEND TOTAL_FILES ${GUI_SOURCES})

source_group("GUI\\Private" FILES ${GUI_SOURCES})

if(APPLE)
    set(GUI_SOURCES_METAL
        GUI/Metal/ImGuiExtensionMetal.mm
    )

    list(APPEND TOTAL_FILES ${GUI_SOURCES_METAL})
    source_group("GUI\\Private\\Metal" FILES ${GUI_SOURCES_METAL})

else()
    set(GUI_SOURCES_VULKAN
        GUI/Vulkan/ImGuiExtensionVulkan.cpp
    )
    message(STATUS "GUI_SOURCES_VULKAN: ${GUI_SOURCES_VULKAN}")

    list(APPEND TOTAL_FILES ${GUI_SOURCES_VULKAN})
    source_group("GUI\\Private\\Vulkan" FILES ${GUI_SOURCES_VULKAN})
endif()

set(PANEL_HEADERS
    Panel/Panel.h
    Panel/DockspacePanel.h
    Panel/ScenePanel.h
    Panel/InspectorPanel.h
    Panel/MenuPanel.h
)

source_group("Panel\\Public" FILES ${PANEL_HEADERS})

list(APPEND TOTAL_FILES ${PANEL_HEADERS})

set(PANEL_SOURCES
    Panel/Panel.cpp
    Panel/DockspacePanel.cpp
    Panel/ScenePanel.cpp
    Panel/InspectorPanel.cpp
    Panel/MenuPanel.cpp
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
    list(APPEND THIRDPARTY_IMGUI_BACKENDS
        ThirdParty/ImGui/imgui_impl_metal.h
        ThirdParty/ImGui/imgui_impl_metal.mm
        ThirdParty/ImGui/imgui_impl_osx.h
        ThirdParty/ImGui/imgui_impl_osx.mm
    )
else()
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

set(THIRDPARTY
    ${THIRDPARTY_IMGUI}
)

list(APPEND TOTAL_FILES ${THIRDPARTY})

add_library(${TARGET_NAME}
    SHARED
    ${TOTAL_FILES}
)

target_link_libraries(${TARGET_NAME}
    PRIVATE
    Engine
)

target_include_directories(${TARGET_NAME}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC ThirdParty
)

set_target_properties(${TARGET_PATH} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${HS_PROJECT_BINARY_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${HS_PROJECT_BINARY_DIR}
)

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
)
