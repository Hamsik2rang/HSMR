set(TARGET_NAME Renderer)

set(TOTAL_FILES)

set(RENDERER_HEADERS
    DeferredPath.h
    ForwardPath.h
    RenderPath.h
    RendererDefinition.h
    RenderTarget.h
)

source_group("Public" FILES ${RENDERER_HEADERS})
list(APPEND TOTAL_FILES ${RENDERER_HEADERS})

set(RENDERER_SOURCES
    Private/DeferredPath.cpp
    Private/ForwardPath.cpp
    Private/RenderPath.cpp
    Private/RenderTarget.cpp
)

source_group("Private" FILES ${RENDERER_SOURCES})
list(APPEND TOTAL_FILES ${RENDERER_SOURCES})

set(RENDERER_RENDER_PASS_HEADERS
    RenderPass/RenderPass.h
)

source_group("RenderPass\\Public" FILES ${RENDERER_RENDER_PASS_HEADERS})
list(APPEND TOTAL_FILES ${RENDERER_RENDER_PASS_HEADERS})

set(RENDERER_RENDER_PASS_SOURCES
    RenderPass/Private/RenderPass.cpp
)

source_group("RenderPass\\Private" FILES ${RENDERER_RENDER_PASS_SOURCES})
list(APPEND TOTAL_FILES ${RENDERER_RENDER_PASS_SOURCES})

set(RENDERER_RENDER_FORWARD_PASS_HEADERS
    RenderPass/Forward/ForwardRenderPass.h
    RenderPass/Forward/ForwardOpaquePass.h
)

source_group("RenderPass\\Forward\\Public" FILES ${RENDERER_RENDER_FORWARD_PASS_HEADERS})
list(APPEND TOTAL_FILES ${RENDERER_RENDER_FORWARD_PASS_HEADERS})

set(RENDERER_RENDER_FORWARD_PASS_SOURCES
    RenderPass/Forward/Private/ForwardRenderPass.cpp
    RenderPass/Forward/Private/ForwardOpaquePass.cpp
)

source_group("RenderPass\\Forward\\Private" FILES ${RENDERER_RENDER_FORWARD_PASS_SOURCES})
list(APPEND TOTAL_FILES ${RENDERER_RENDER_FORWARD_PASS_SOURCES})

add_library(${TARGET_NAME}
    SHARED
    ${TOTAL_FILES}
)

target_include_directories(${TARGET_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_dependencies(${TARGET_NAME} Platform Core RHI Resource)

target_link_libraries(${TARGET_NAME} PRIVATE Platform Core RHI Resource)

target_compile_definitions(${TARGET_NAME}
    PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:MinSizeRel>:_RELEASE>
    $<$<CONFIG:Release>:_RELEASE>
    $<$<CONFIG:RelWithDebInfo>:_RELWITHDEBINFO>
    HS_RENDERER
    HS_RENDERER_API_EXPORT

    HS_PLATFORM_API_IMPORT
    HS_CORE_API_IMPORT
    HS_RHI_API_IMPORT

    HS_RESOURCE_API_IMPORT
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
    RUNTIME DESTINATION "${INSTALL_FOLDER}"
)