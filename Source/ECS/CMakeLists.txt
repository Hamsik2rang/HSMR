set(TARGET_NAME ECS)

set(TOTAL_FILES)

set(ECS_HEADERS
    Entity.h
    Scene.h
)

source_group("Public" FILES ${ECS_HEADERS})
list(APPEND TOTAL_FILES ${ECS_HEADERS})

set(ECS_SOURCES
    Private/Entity.cpp
    Private/Scene.cpp
)
source_group("Private" FILES ${ECS_SOURCES})
list(APPEND TOTAL_FILES ${ECS_SOURCES})

set(ECS_COMPONENT_HEADERS
    Component/CameraComponent.h
    Component/Component.h
    Component/LightComponent.h
    Component/NameComponent.h
    Component/RenderComponent.h
    Component/TransformComponent.h
)
source_group("Component\\Public" FILES ${ECS_COMPONENT_HEADERS})
list(APPEND TOTAL_FILES ${ECS_COMPONENT_HEADERS})

set(ECS_COMPONENT_SOURCES
    Component/Private/CameraComponent.cpp
    Component/Private/LightComponent.cpp
    Component/Private/NameComponent.cpp
    Component/Private/RenderComponent.cpp
    Component/Private/TransformComponent.cpp
)
source_group("Component\\Private" FILES ${ECS_COMPONENT_SOURCES})
list(APPEND TOTAL_FILES ${ECS_COMPONENT_SOURCES})

set(ECS_SYSTEM_HEADERS
    System/CameraSystem.h
    System/LightSystem.h
    System/NameSystem.h
    System/RenderSystem.h
    System/System.h
    System/TransformSystem.h
)
source_group("System\\Public" FILES ${ECS_SYSTEM_HEADERS})
list(APPEND TOTAL_FILES ${ECS_SYSTEM_HEADERS})

set(ECS_SYSTEM_SOURCES
    System/Private/CameraSystem.cpp
    System/Private/LightSystem.cpp
    System/Private/NameSystem.cpp
    System/Private/RenderSystem.cpp
    System/Private/TransformSystem.cpp
)
source_group("System\\Private" FILES ${ECS_SYSTEM_SOURCES})
list(APPEND TOTAL_FILES ${ECS_SYSTEM_SOURCES})

add_library(${TARGET_NAME}
    STATIC
    ${TOTAL_FILES}
)

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
    HS_ECS
    HS_ECS_API_EXPORT
    HS_API_EXPORT

    HS_RESOURCE_API_IMPORT
    HS_RENDERER_API_IMPORT
)

install(TARGETS ${TARGET_NAME}
    ARCHIVE DESTINATION "${INSTALL_FOLDER}/dev/lib"
    LIBRARY DESTINATION "${INSTALL_FOLDER}"
    RUNTIME DESTINATION "${INSTALL_FOLDER}"
)