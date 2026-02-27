set(TARGET_NAME Scene)

set(TOTAL_FILES)

set(SCENE_HEADERS
    Scene.h
    SceneGraph.h
    Entity.h
    SceneSerializer.h
)

source_group("Public" FILES ${SCENE_HEADERS})
list(APPEND TOTAL_FILES ${SCENE_HEADERS})

set(SCENE_COMPONENT_HEADERS
    Components/Components.h
    Components/TransformComponent.h
    Components/TagComponent.h
    Components/MeshRendererComponent.h
    Components/CameraComponent.h
    Components/LightComponent.h
)

source_group("Components\\Public" FILES ${SCENE_COMPONENT_HEADERS})
list(APPEND TOTAL_FILES ${SCENE_COMPONENT_HEADERS})

set(SCENE_SYSTEM_HEADERS
    Systems/IComponentSystem.h
    Systems/TransformSystem.h
    Systems/MeshBoundsSystem.h
)

source_group("Systems\\Public" FILES ${SCENE_SYSTEM_HEADERS})
list(APPEND TOTAL_FILES ${SCENE_SYSTEM_HEADERS})

set(SCENE_SOURCES
    Private/Scene.cpp
    Private/SceneGraph.cpp
    Private/Entity.cpp
    Private/SceneSerializer.cpp
    Private/TransformSystem.cpp
    Private/MeshBoundsSystem.cpp
)

source_group("Private" FILES ${SCENE_SOURCES})
list(APPEND TOTAL_FILES ${SCENE_SOURCES})

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
        Core Resource
    )
else()
    target_link_libraries(${TARGET_NAME}
        PRIVATE
        Core Resource
    )
endif()

if(WIN32)
    # Core is SHARED, no WHOLEARCHIVE needed
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

    HS_SCENE_API_EXPORT
    HS_RESOURCE_API_IMPORT
    HS_CORE_API_IMPORT
)

install(TARGETS ${TARGET_NAME}
    ARCHIVE DESTINATION "${INSTALL_FOLDER}/dev/lib"
    LIBRARY DESTINATION "${INSTALL_FOLDER}"
    RUNTIME DESTINATION "${INSTALL_FOLDER}"
)

hs_auto_install(${TARGET_NAME})
