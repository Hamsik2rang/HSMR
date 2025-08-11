set(TARGET_NAME Renderer)

set(TOTAL_FILES)

set(RENDERER_HEADERS
    DeferredPath.h
    ForwardPath.h
    RenderPath.h
    RenderDefinition.h
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

set(RENDERER_RENDERING_PASS_HEADERS
    RenderingPass/RenderingPass.h
)

source_group("RenderingPass\\Public" FILES ${RENDERER_RENDERING_PASS_HEADERS})
list(APPEND TOTAL_FILES ${RENDERER_RENDERING_PASS_HEADERS})

set(RENDERER_RENDERING_PASS_SOURCES
    RenderingPass/Private/RenderingPass.cpp
)

source_group("RenderingPass\\Private" FILES ${RENDERER_RENDERING_PASS_SOURCES})
list(APPEND TOTAL_FILES ${RENDERER_RENDERING_PASS_SOURCES})

set(RENDERER_RENDERING_FORWARD_PASS_HEADERS
    RenderingPass/Forward/ForwardPass.h
    RenderingPass/Forward/ForwardOpaquePass.h
)

source_group("RenderingPass\\Forward\\Public" FILES ${RENDERER_RENDERING_FORWARD_PASS_HEADERS})
list(APPEND TOTAL_FILES ${RENDERER_RENDERING_FORWARD_PASS_HEADERS})

set(RENDERER_RENDERING_FORWARD_PASS_SOURCES
    RenderingPass/Forward/Private/ForwardPass.cpp
    RenderingPass/Forward/Private/ForwardOpaquePass.cpp
)

source_group("RenderingPass\\Forward\\Private" FILES ${RENDERER_RENDERING_FORWARD_PASS_SOURCES})
list(APPEND TOTAL_FILES ${RENDERER_RENDERING_FORWARD_PASS_SOURCES})

add_library(${TARGET_NAME}
    SHARED
    ${TOTAL_FILES}
)

target_include_directories(${TARGET_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_dependencies(${TARGET_NAME} HAL Core RHI)

target_compile_definitions(${TARGET_NAME}
    PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:MinSizeRel>:_RELEASE>
    $<$<CONFIG:Release>:_RELEASE>
    $<$<CONFIG:RelWithDebInfo>:_RELWITHDEBINFO>
    HS_RENDERER
    HS_API_EXPORT
    HS_RENDERER_API_EXPORT

    HS_RESOURCE_API_IMPORT
)

if(USE_SHADERSYSTEM)
    add_dependencies(${TARGET_NAME} ShaderSystem)
    target_compile_definitions(${TARGET_NAME} PRIVATE HS_SHADERSYSTEM_API_IMPORT)
endif()

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