set(TARGET_NAME Engine)

set(TOTAL_FILES)
set(ENGINE_COMMON_HEADERS
    Application.h
    Window.h
    Camera.h
    EngineContext.h
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

add_library(${TARGET_NAME} SHARED ${TOTAL_FILES})

if(APPLE)
    list(APPEND OSX_FRAMEWORK "-framework OpenGL -framework Foundation -framework CoreFoundation -framework QuartzCore -framework AppKit -framework IOKit -framework Metal -framework GameController -framework UniformTypeIdentifiers")

    # Add SDL3 framework support
    if(HSMR_USE_SDL)
        target_compile_options(${TARGET_NAME}
            PRIVATE
            -F${HS_DEPS_LIB_DIR}
        )

        target_link_options(${TARGET_NAME}
            PRIVATE
            -F${HS_DEPS_LIB_DIR}
        )

        list(APPEND OSX_FRAMEWORK "-framework SDL3")
    endif()

    target_link_libraries(${TARGET_NAME}
        PRIVATE
        "-Wl, -all_load" Core Platform RHI ShaderSystem Resource Renderer Scene
        ${OSX_FRAMEWORK}
    )
else()
    target_link_libraries(${TARGET_NAME}
        PRIVATE
        Core Platform RHI ShaderSystem Resource Renderer Scene
    )
endif()

if(WIN32)
    target_link_options(${TARGET_NAME}
        PRIVATE
        "/WHOLEARCHIVE:Platform"
    )
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
    HS_ENGINE

    HS_API_EXPORT
    HS_CORE_API_IMPORT
    HS_RHI_API_IMPORT
    HS_SHADER_SYSTEM_API_IMPORT
    HS_RESOURCE_API_IMPORT
    HS_RENDERER_API_IMPORT
    HS_SCENE_API_IMPORT
)

install(TARGETS ${TARGET_NAME}
    ARCHIVE DESTINATION "${INSTALL_FOLDER}/dev/lib"
    LIBRARY DESTINATION "${INSTALL_FOLDER}"
    RUNTIME DESTINATION "${INSTALL_FOLDER}"
)

hs_auto_install(${TARGET_NAME})
