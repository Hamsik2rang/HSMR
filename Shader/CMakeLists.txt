set(TARGET_NAME Shader)

# Collect all shader source files
file(GLOB_RECURSE SLANG_SHADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.slang")
file(GLOB_RECURSE HLSLI_SHADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.hlsli")

# Create source groups for IDE organization
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SLANG_SHADERS})
source_group("Public" FILES ${HLSLI_SHADERS})

set(SHADER_COPY_COMMANDS
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:Client>/Assets/Shaders"
)

foreach(shader ${SLANG_SHADERS} ${HLSLI_SHADERS})
    get_filename_component(SHADER_FILENAME ${shader} NAME)
    list(APPEND SHADER_COPY_COMMANDS
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${shader}
            "$<TARGET_FILE_DIR:Client>/Assets/Shaders/${SHADER_FILENAME}"
    )
endforeach()

# Custom target: copy shader sources to Assets/Shaders/
add_custom_target(${TARGET_NAME} ALL
    ${SHADER_COPY_COMMANDS}
    DEPENDS ${SLANG_SHADERS} ${HLSLI_SHADERS}
    COMMENT "Copying shader source files to Assets/Shaders/"
    VERBATIM
    SOURCES ${SLANG_SHADERS} ${HLSLI_SHADERS}
)

# Set shader files to be displayed in IDE but not compiled by C++ compiler
set_source_files_properties(${SLANG_SHADERS} ${HLSLI_SHADERS} PROPERTIES
    HEADER_FILE_ONLY TRUE
    VS_TOOL_OVERRIDE "None"
)
