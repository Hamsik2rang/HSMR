set(TARGET_NAME Editor)

set(TOTAL_FILES)

set(CORE_HEADERS
    Core/EditorApplication.h
    Core/EditorWindow.h
)

source_group("Core\\Public" FILES ${CORE_HEADERS})

list(APPEND TOTAL_FILES ${CORE_HEADERS})

set(CORE_SOURCES
    Core/EditorApplication.cpp
    Core/EditorWindow.cpp
)

source_group("Core\\Private" FILES ${CORE_SOURCES})

list(APPEND TOTAL_FILES ${CORE_SOURCES})

set(GUI_HEADERS
    GUI/GUIContext.h
    GUI/GUIRenderer.h
    GUI/ImGuiExtension.h
)

source_group("GUI\\Public" FILES ${GUI_HEADERS})

list(APPEND TOTAL_FILES ${GUI_HEADERS})

set(GUI_SOURCES
    GUI/GUIContext.cpp
    GUI/GUIRenderer.cpp
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

add_library(${TARGET_NAME}
    SHARED
    ${TOTAL_FILES}
)

target_link_libraries(${TARGET_NAME}
    PRIVATE
    Engine
)

set_target_properties(${TARGET_PATH} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${HS_PROJECT_BINARY_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${HS_PROJECT_BINARY_DIR}
)

target_compile_definitions(${TARGET_NAME} PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:MinSizeRel>:_RELEASE>
    $<$<CONFIG:Release>:_RELEASE>
    $<$<CONFIG:RelWithDebInfo>:_RELWITHDEBINFO>
    HS_EDITOR
    HS_EDITOR_API_EXPORT
)
