set(TARGET_NAME Client)

set(EXECUTABLE_NAME "HSMR")

set(TOTAL_FILES
    main.cpp
)

if(APPLE)
    list(APPEND OSX_FRAMEWORK "-framework OpenGL -framework Foundation -framework CoreFoundation -framework QuartzCore -framework AppKit -framework IOKit -framework Metal -framework GameController")
    add_executable(${TARGET_NAME}
        ${TOTAL_FILES}
    )
    set_target_properties(${TARGET_NAME} PROPERTIES
        XCODE_ATTRIBUTE_INSTALL_PATH "/Client"
        OUTPUT_NAME ${EXECUTABLE_NAME}
        RUNTIME_OUTPUT_DIRECTORY ${HS_PROJECT_BINARY_DIR}
    )

    target_link_libraries(${TARGET_NAME}
        PRIVATE
        HAL Core RHI Renderer ShaderSystem Resource ECS
        Editor
        ${OSX_FRAMEWORK}
    )
else()
    add_executable(${TARGET_NAME}
        ${TOTAL_FILES}
    )
    set_target_properties(${TARGET_NAME} PROPERTIES
        OUTPUT_NAME ${EXECUTABLE_NAME}
        RUNTIME_OUTPUT_DIRECTORY ${HS_PROJECT_BINARY_DIR})

    target_link_libraries(${TARGET_NAME}
        PRIVATE
        HAL Core RHI Renderer ShaderSystem Resource ECS
        Editor
    )
endif()

add_dependencies(${TARGET_NAME} Editor)

target_compile_definitions(${TARGET_NAME} PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:MinSizeRel>:_RELEASE>
    $<$<CONFIG:Release>:_RELEASE>
    $<$<CONFIG:RelWithDebInfo>:_RELWITHDEBINFO>
    HS_CLIENT
    HS_HAL_API_IMPORT
    HS_CORE_API_IMPORT
    HS_RHI_API_IMPORT
    HS_EDITOR_API_IMPORT
    HS_RESOURCE_API_IMPORT
    HS_SHADERSYSTEM_API_IMPORT
    HS_RENDERER_API_IMPORT
    HS_PHYSICS_API_IMPORT
    HS_ECS_API_IMPORT
    HS_EXECUTABLE_NAME="${EXECUTABLE_NAME}"
)

if(APPLE)
    set(CMAKE_XCODE_GENERATE_SCHEME TRUE)
    set_property(TARGET ${TARGET_NAME} PROPERTY XCODE_SCHEME_ENABLE_GPU_FRAME_CAPTURE_MODE Metal)
    set_property(TARGET ${TARGET_NAME} PROPERTY XCODE_ATTRIBUTE_MTL_ENABLE_DEBUG_INFO "INCLUDE_SOURCE")
endif()

# set_property(TARGET ${TARGET_NAME} PROPERTY XCODE_GENERATE_DEBUGGING_SYMBOLS ON)
message(STATUS "HS_DEPS_DLL_DIR : ${HS_DEPS_DLL_DIR}")
message(STATUS "HS_DEPS_LIB_DIR : ${HS_DEPS_LIB_DIR}")
message(STATUS "INSTALL_FOLDER: ${INSTALL_FOLDER}")

install(DIRECTORY "${HS_DEPS_DLL_DIR}/" DESTINATION "${INSTALL_FOLDER}")
install(DIRECTORY "${HS_DEPS_LIB_DIR}/" DESTINATION "${INSTALL_FOLDER}")

# install(DIRECTORY "${HS_DEPS_BIN_DIR}/" DESTINATION "${INSTALL_FOLDER}")
install(DIRECTORY "${HS_ASSET_DIR}/" DESTINATION "${INSTALL_FOLDER}/Assets")

# Shader
# install(FILES "${METALLIB_FILE}/" DESTINATION "${INSTALL_FOLDER}/Resource/Shader")
# if(WIN32)
# set(SLANGC_EXE "slangc.exe")
# set(SHADER_TARGET_API "spirv")
# set(SHADER_OUTPUT_EXT ".spv")
# set(SHADER_OUTPUT_SUBDIR "SPV")

# # Windows DLLs to copy
# # set(SLANG_RUNTIME_FILES
# # "${SLANG_BIN_DIR}/slang.dll"
# # "${SLANG_BIN_DIR}/slang-glslang.dll"
# # "${SLANG_BIN_DIR}/slang-glsl-module.dll"
# # "${SLANG_BIN_DIR}/slang-rt.dll"
# # )
# elseif(APPLE)
# # if(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
# # set(SLANG_BIN_DIR "${SLANG_DEPENDENCY_DIR}/bin/macosx-aarch64/release")
# # else()
# # set(SLANG_BIN_DIR "${SLANG_DEPENDENCY_DIR}/bin/macosx-x64/release")
# # endif()
# set(SLANGC_EXE "slangc")
# set(SHADER_TARGET_API "metal")
# set(SHADER_OUTPUT_EXT ".metal")
# set(SHADER_OUTPUT_SUBDIR "MSL")

# # macOS dylibs to copy
# # set(SLANG_RUNTIME_FILES
# # "${SLANG_BIN_DIR}/libslang.dylib"
# # "${SLANG_BIN_DIR}/libslang-glslang.dylib"
# # "${SLANG_BIN_DIR}/libslang-glsl-module.dylib"
# # "${SLANG_BIN_DIR}/libslang-rt.dylib"
# # "${SLANG_BIN_DIR}/libslang-llvm.dylib"
# # )
# endif()
set(HS_SHADER_SRC_DIR ${HS_ASSET_DIR}/Shaders)
message(STATUS "HS_SHADER_SRC_DIR: ${HS_SHADER_SRC_DIR}")

set(HS_SLANG_SHADER_SRC_DIR ${HS_SHADER_SRC_DIR}/Slang)

if(APPLE)
    set(SLANG_EXECUTABLE "${HS_DEPS_BIN_DIR}/slangc")
    set(SHADER_TARGET_API "metal")
    set(SHADER_OUTPUT_EXT "metal")
    set(SHADER_CAPABILITY "METAL_2_4")

else()
    set(SLANG_EXECUTABLE "${HS_DEPS_BIN_DIR}/slangc.exe")
    set(SHADER_TARGET_API "spirv")
    set(SHADER_OUTPUT_EXT "spv")
    set(SHADER_CAPABILITY "spirv_1_3")
endif()

file(GLOB SLANG_SHADERS "${HS_SLANG_SHADER_SRC_DIR}/*.slang")
set(SHADER_COMPILE_COMMANDS)

# Create directory command
list(APPEND SHADER_COMPILE_COMMANDS
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${TARGET_NAME}>/Assets/Shaders"
)

foreach(shader ${SLANG_SHADERS})
    get_filename_component(SHADER_NAME ${shader} NAME_WE)

    message(STATUS "Processing shader: ${SHADER_NAME}")

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
        -o "$<TARGET_FILE_DIR:${TARGET_NAME}>/Assets/Shaders/${SHADER_NAME}.${SHADER_NAME_SIGNATURE}.${SHADER_OUTPUT_EXT}"
    )
endforeach()

add_custom_target(CompileShaders
    ${SHADER_COMPILE_COMMANDS}
    COMMAND ${CMAKE_COMMAND} -E remove -f "${HS_DEPS_BIN_DIR}/slang-glsl-module.bin"
    DEPENDS ${SLANG_SHADERS}
    WORKING_DIRECTORY ${HS_DEPS_BIN_DIR}
    COMMENT "Compiling all Slang shaders and cleaning up temporary files"
    VERBATIM
)

set_property(TARGET CompileShaders PROPERTY FOLDER "Commands")

# add_custom_command(
# TARGET ${TARGET_NAME} # 이 타겟에 명령을 추가합니다.
# POST_BUILD # 빌드가 완료된 직후에 실행됩니다.
# COMMAND ${CMAKE_COMMAND} # CMake 실행 파일을 호출합니다.
# ARGS --build ${CMAKE_BINARY_DIR} --target install

# # 현재 빌드 디렉토리에서 INSTALL 타겟을 실행합니다.
# COMMENT "Running automatic 'install' target after ${TARGET_NAME} build."
# )

# ---------- Shader compilation at install time ----------
# install(CODE "
# # Setup paths
# set(SLANGC_PATH \"\${HS_DEPS_BIN_DIR}/\${SLANGC_EXE}\")
# set(SHADER_SOURCE_DIR \"\${INSTALL_FOLDER}/Slang\")
# set(SHADER_OUTPUT_DIR \"\${INSTALL_FOLDER}/Assets/Shaders/\${SHADER_OUTPUT_SUBDIR}\")

# # Create output directory
# file(MAKE_DIRECTORY \"\${SHADER_OUTPUT_DIR}\")

# # Log start
# message(STATUS \"========================================\")
# message(STATUS \"Starting shader compilation with slangc\")
# message(STATUS \"Source: \${SHADER_SOURCE_DIR}\")
# message(STATUS \"Output: \${SHADER_OUTPUT_DIR}\")
# message(STATUS \"Target: \${SHADER_TARGET_API}\")
# message(STATUS \"========================================\")

# # Find all shader files
# file(GLOB_RECURSE SLANG_SHADERS \"\${SHADER_SOURCE_DIR}/*.slang\")

# # Shader stage detection function
# function(get_shader_stage SHADER_FILE STAGE_VAR ENTRY_VAR)
# get_filename_component(SHADER_NAME \"\${SHADER_FILE}\" NAME_WE)

# # Detect stage from filename
# if(SHADER_NAME MATCHES \"\.vert$\" OR SHADER_NAME MATCHES \"_vs$\" OR SHADER_NAME MATCHES \"Vertex\")
# set(\${STAGE_VAR} \"vertex\" PARENT_SCOPE)
# set(\${ENTRY_VAR} \"vertexMain\" PARENT_SCOPE)
# elseif(SHADER_NAME MATCHES \"\.frag$\" OR SHADER_NAME MATCHES \"_ps$\" OR SHADER_NAME MATCHES \"_fs$\" OR
# SHADER_NAME MATCHES \"Fragment\" OR SHADER_NAME MATCHES \"Pixel\")
# set(\${STAGE_VAR} \"fragment\" PARENT_SCOPE)
# set(\${ENTRY_VAR} \"fragmentMain\" PARENT_SCOPE)
# elseif(SHADER_NAME MATCHES \"\.comp$\" OR SHADER_NAME MATCHES \"_cs$\" OR SHADER_NAME MATCHES \"Compute\")
# set(\${STAGE_VAR} \"compute\" PARENT_SCOPE)
# set(\${ENTRY_VAR} \"computeMain\" PARENT_SCOPE)
# elseif(SHADER_NAME MATCHES \"\.geom$\" OR SHADER_NAME MATCHES \"_gs$\" OR SHADER_NAME MATCHES \"Geometry\")
# set(\${STAGE_VAR} \"geometry\" PARENT_SCOPE)
# set(\${ENTRY_VAR} \"geometryMain\" PARENT_SCOPE)
# else()
# # Default to vertex shader if uncertain
# set(\${STAGE_VAR} \"vertex\" PARENT_SCOPE)
# set(\${ENTRY_VAR} \"main\" PARENT_SCOPE)
# endif()
# endfunction()

# # Statistics
# set(COMPILE_SUCCESS_COUNT 0)
# set(COMPILE_FAIL_COUNT 0)
# list(LENGTH SLANG_SHADERS TOTAL_SHADER_COUNT)

# # Compile each shader
# foreach(SHADER_FILE \${SLANG_SHADERS})
# # Get relative path for output structure
# file(RELATIVE_PATH SHADER_REL_PATH \"\${SHADER_SOURCE_DIR}\" \"\${SHADER_FILE}\")
# get_filename_component(SHADER_REL_DIR \"\${SHADER_REL_PATH}\" DIRECTORY)
# get_filename_component(SHADER_NAME \"\${SHADER_FILE}\" NAME_WE)

# # Detect shader stage
# get_shader_stage(\"\${SHADER_FILE}\" SHADER_STAGE SHADER_ENTRY)

# # Create output subdirectory if needed
# if(NOT \"\${SHADER_REL_DIR}\" STREQUAL \"\")
# file(MAKE_DIRECTORY \"\${SHADER_OUTPUT_DIR}/\${SHADER_REL_DIR}\")
# endif()

# # Build output path
# set(OUTPUT_FILE \"\${SHADER_OUTPUT_DIR}/\${SHADER_REL_DIR}/\${SHADER_NAME}.\${SHADER_OUTPUT_EXT}\")

# # Build slangc command
# set(SLANGC_COMMAND
# \"\${SLANGC_PATH}\"
# -target \${SHADER_TARGET_API}
# -profile sm_6_0
# -stage \${SHADER_STAGE}
# -entry \${SHADER_ENTRY}
# -O3
# -matrix-layout-column-major
# \"\${SHADER_FILE}\"
# -o \"\${OUTPUT_FILE}\"
# )

# # Platform-specific options
# if(\"\${SHADER_TARGET_API}\" STREQUAL \"spirv\")
# list(APPEND SLANGC_COMMAND
# -capability spirv_1_5
# -emit-spirv-directly)
# elseif(\"\${SHADER_TARGET_API}\" STREQUAL \"metal\")
# list(APPEND SLANGC_COMMAND
# -target-version 2.4)
# endif()

# # Execute compilation
# message(STATUS \"Compiling: \${SHADER_NAME} (\${SHADER_STAGE} shader)\")

# execute_process(
# COMMAND \${SLANGC_COMMAND}
# RESULT_VARIABLE COMPILE_RESULT
# OUTPUT_VARIABLE COMPILE_OUTPUT
# ERROR_VARIABLE COMPILE_ERROR
# OUTPUT_STRIP_TRAILING_WHITESPACE
# ERROR_STRIP_TRAILING_WHITESPACE
# )

# if(COMPILE_RESULT EQUAL 0)
# message(STATUS \"  ✓ Success: \${OUTPUT_FILE}\")
# math(EXPR COMPILE_SUCCESS_COUNT \"\${COMPILE_SUCCESS_COUNT} + 1\")

# # Optionally show shader info
# if(COMPILE_OUTPUT)
# message(STATUS \"  Info: \${COMPILE_OUTPUT}\")
# endif()
# else()
# message(WARNING \"  ✗ Failed: \${SHADER_NAME}\")
# message(WARNING \"  Error: \${COMPILE_ERROR}\")
# math(EXPR COMPILE_FAIL_COUNT \"\${COMPILE_FAIL_COUNT} + 1\")
# endif()
# endforeach()

# # Summary
# message(STATUS \"========================================\")
# message(STATUS \"Shader Compilation Complete\")
# message(STATUS \"Total: \${TOTAL_SHADER_COUNT} shaders\")
# message(STATUS \"Success: \${COMPILE_SUCCESS_COUNT}\")
# message(STATUS \"Failed: \${COMPILE_FAIL_COUNT}\")
# message(STATUS \"========================================\")

# # Fail the install if any shaders failed to compile
# if(COMPILE_FAIL_COUNT GREATER 0)
# message(FATAL_ERROR \"Shader compilation failed for \${COMPILE_FAIL_COUNT} shader(s). Please fix the errors
# above.\")
# endif()
# "
# COMPONENT Shaders
# )

# # ---------- Optional: Create a convenience target for shader compilation only ----------
# add_custom_target(CompileShaders
# COMMAND ${CMAKE_COMMAND} --install ${CMAKE_BINARY_DIR} --component Shaders --config $<CONFIG>
# COMMENT "Compiling shaders with slangc"
# )

# set_property(TARGET CompileShaders PROPERTY FOLDER "Commands")

# # ---------- Optional: Add shader compilation as dependency to main targets ----------
# This ensures shaders are compiled when building the Client
# if(TARGET Client)
# add_dependencies(Client CompileShaders)
# endif()

# # ---------- Install configuration for different build types ----------
# if(CMAKE_CONFIGURATION_TYPES)
# # Multi-config generators (Visual Studio, Xcode)
# set(CMAKE_INSTALL_CONFIG_NAME "$<CONFIG>")
# else()
# # Single-config generators (Make, Ninja)
# if(NOT CMAKE_BUILD_TYPE)
# set(CMAKE_BUILD_TYPE "Release")
# endif()

# set(CMAKE_INSTALL_CONFIG_NAME "${CMAKE_BUILD_TYPE}")
# endif()

# # ---------- CPack configuration (optional, for creating installers) ----------
# set(CPACK_COMPONENTS_ALL ShaderCompiler Shaders)
# set(CPACK_COMPONENT_SHADERCOMPILER_DISPLAY_NAME "Shader Compiler Tools")
# set(CPACK_COMPONENT_SHADERS_DISPLAY_NAME "Compiled Shaders")
# set(CPACK_COMPONENT_SHADERS_DEPENDS ShaderCompiler)