set(TARGET_NAME Shader)

if(WIN32)
    set(SLANG_EXECUTABLE "${HS_DEPS_BIN_DIR}/slangc.exe")
    set(SHADER_TARGET_API "spirv")
    set(SHADER_OUTPUT_EXT "spv")
    set(SHADER_CAPABILITY "spirv_1_3")

elseif(APPLE)
    set(SLANG_EXECUTABLE "${HS_DEPS_BIN_DIR}/slangc")
    set(SHADER_TARGET_API "metal")
    set(SHADER_OUTPUT_EXT "metal")
    set(SHADER_CAPABILITY "METAL_2_4")
endif()

# Collect all .slang shader files recursively
file(GLOB_RECURSE SLANG_SHADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.slang"
)

file(GLOB_RECURSE HLSLI_SHADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hlsli"
)

# Create source groups for IDE organization (mirrors folder structure)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SLANG_SHADERS})
source_group("Public" FILES ${HLSLI_SHADERS})

set(SHADER_COMPILE_COMMANDS)
set(SHADER_OUTPUT_FILES)

list(APPEND SHADER_COMPILE_COMMANDS
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:Client>/Assets/Shaders"
)

foreach(shader ${SLANG_SHADERS})
    get_filename_component(SHADER_NAME ${shader} NAME_WE)

    message(STATUS "Processing Shader: ${SHADER_NAME}")

    string(FIND "${shader}" ".vert" VERT_POS)
    string(FIND "${shader}" ".frag" FRAG_POS)
    string(FIND "${shader}" ".comp" COMP_POS)

    if(NOT VERT_POS EQUAL -1)
        set(SHADER_NAME_SIGNATURE "vert")
        set(SHADER_STAGE "vertex")
        set(ENTRY_NAME "VertexMain")
    elseif(NOT FRAG_POS EQUAL -1)
        set(SHADER_NAME_SIGNATURE "frag")
        set(SHADER_STAGE "fragment")
        set(ENTRY_NAME "FragmentMain")
    elseif(NOT COMP_POS EQUAL -1)
        set(SHADER_NAME_SIGNATURE "comp")
        set(SHADER_STAGE "compute")
        set(ENTRY_NAME "ComputeMain")
    else()
        message(FATAL_ERROR "Unknown shader type: ${shader}")
        continue()
    endif()

    message(STATUS "Shader Type: ${SHADER_STAGE}, Entry: ${ENTRY_NAME}")

    # Add compilation command to list
    list(APPEND SHADER_COMPILE_COMMANDS
        COMMAND ${SLANG_EXECUTABLE} ${shader}
        -target ${SHADER_TARGET_API}
        -profile sm_6_0
        -stage ${SHADER_STAGE}
        -entry ${ENTRY_NAME}
        -capability ${SHADER_CAPABILITY}
        -I ${CMAKE_CURRENT_SOURCE_DIR}
        -g3 -O0
        -matrix-layout-column-major
        -o "$<TARGET_FILE_DIR:Client>/Assets/Shaders/${SHADER_NAME}.${SHADER_NAME_SIGNATURE}.${SHADER_OUTPUT_EXT}"
    )
endforeach()

# Custom target with SOURCES for IDE visibility
# ALL ensures Shader is built before install
add_custom_target(${TARGET_NAME} ALL
    ${SHADER_COMPILE_COMMANDS}
    COMMAND ${CMAKE_COMMAND} -E remove -f "${HS_DEPS_BIN_DIR}/slang-glsl-module.bin"
    DEPENDS ${SLANG_SHADERS}
    WORKING_DIRECTORY ${HS_DEPS_BIN_DIR}
    COMMENT "Compiling all Slang shaders and cleaning up temporary files"
    VERBATIM
    SOURCES ${SLANG_SHADERS}
    SOURCES ${HLSLI_SHADERS}
)

# Set shader files to be displayed in IDE but not compiled by C++ compiler
set_source_files_properties(${SLANG_SHADERS} PROPERTIES
    HEADER_FILE_ONLY TRUE
    VS_TOOL_OVERRIDE "None"
)
