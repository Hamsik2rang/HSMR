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

# Create source groups for IDE organization (mirrors folder structure)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SLANG_SHADERS})

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
        -g3 -O0
        -matrix-layout-column-major
        -o "$<TARGET_FILE_DIR:Client>/Assets/Shaders/${SHADER_NAME}.${SHADER_NAME_SIGNATURE}.${SHADER_OUTPUT_EXT}"
    )
endforeach()

# Custom target with SOURCES for IDE visibility
add_custom_target(${TARGET_NAME}
    ${SHADER_COMPILE_COMMANDS}
    COMMAND ${CMAKE_COMMAND} -E remove -f "${HS_DEPS_BIN_DIR}/slang-glsl-module.bin"
    DEPENDS ${SLANG_SHADERS}
    WORKING_DIRECTORY ${HS_DEPS_BIN_DIR}
    COMMENT "Compiling all Slang shaders and cleaning up temporary files"
    VERBATIM
    SOURCES ${SLANG_SHADERS}
)

# Set shader files to be displayed in IDE but not compiled by C++ compiler
set_source_files_properties(${SLANG_SHADERS} PROPERTIES
    HEADER_FILE_ONLY TRUE
    VS_TOOL_OVERRIDE "None"
)

# INSTALL 타겟 빌드 시 Shader 타겟도 함께 빌드되도록 설정
# install(CODE ...) 내부에서는 generator expression이 동작하지 않으므로
# 변수를 미리 문자열로 치환하여 전달
install(CODE "
    execute_process(
        COMMAND \"${CMAKE_COMMAND}\" --build \"${CMAKE_BINARY_DIR}\" --target Shader --config \${CMAKE_INSTALL_CONFIG_NAME}
        RESULT_VARIABLE result
    )
    if(NOT result EQUAL 0)
        message(FATAL_ERROR \"Shader build failed\")
    endif()
")
