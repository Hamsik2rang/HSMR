set(TARGET_NAME Resource)

if(APPLE)
    set(ASSIMP ${HS_DEPS_DLL_DIR}/libassimp.dylib)
    set(STB_INCLUDE ${HS_DEPS_INCLUDE_DIR}/stb)
else()
    set(ASSIMP ${HS_DEPS_LIB_DIR}/assimp-vc143-mt.lib)
    set(STB_INCLUDE ${HS_DEPS_INCLUDE_DIR}/stb)
endif()

set(TOTAL_FILES)

set(RESOURCE_HEADERS
    Image.h
    Mesh.h
    Material.h
    Shader.h
    Model.h
    ObjectManager.h
    ResourceDefinition.h
    GeometryDefinition.h
)

source_group("Public" FILES ${RESOURCE_HEADERS})
list(APPEND TOTAL_FILES ${RESOURCE_HEADERS})

set(RESOURCE_SOURCES
    Private/Image.cpp
    Private/Mesh.cpp
    Private/Material.cpp
    Private/Shader.cpp
    Private/Model.cpp
    Private/ObjectManager.cpp
    Private/ResourceDefinition.cpp
    Private/GeometryDefinition.cpp
)

source_group("Private" FILES ${RESOURCE_SOURCES})
list(APPEND TOTAL_FILES ${RESOURCE_SOURCES})

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
        Core RHI ShaderSystem
        ${ASSIMP}
    )
else()
    target_link_libraries(${TARGET_NAME}
        PRIVATE
        Core RHI ShaderSystem
        ${ASSIMP}
    )
endif()

if(WIN32)
    # Core and RHI are SHARED, no WHOLEARCHIVE needed
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

    HS_RESOURCE_API_EXPORT
    HS_SHADER_SYSTEM_API_IMPORT
    HS_CORE_API_IMPORT
    HS_RHI_API_IMPORT
)

install(TARGETS ${TARGET_NAME}
    ARCHIVE DESTINATION "${INSTALL_FOLDER}/dev/lib"
    LIBRARY DESTINATION "${INSTALL_FOLDER}"
    RUNTIME DESTINATION "${INSTALL_FOLDER}"
)

hs_auto_install(${TARGET_NAME})
