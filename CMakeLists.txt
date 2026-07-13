cmake_minimum_required(VERSION 3.22.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(ENGINE_NAME "HSMR")

project(${ENGINE_NAME} VERSION 0.1.0)

set(HS_ROOT_DIR ${HSMR_SOURCE_DIR})
message(STATUS "HS_ROOT_DIR: ${HS_ROOT_DIR}")

set(HS_EXECUTABLE_DIR "${HSMR_BINARY_DIR}/$<CONFIG>")
message(STATUS "HS_EXECUTABLE_DIR: ${HS_EXECUTABLE_DIR}")

set(HS_PROJECT_BINARY_DIR ${PROJECT_BINARY_DIR})
message(STATUS "HS_PROJECT_BINARY_DIR: ${PROJECT_BINARY_DIR}")

set(HS_SRC_DIR ${HS_ROOT_DIR}/Source)
message(STATUS "HS_SRC_DIR: ${HS_SRC_DIR}")

set(HS_ASSET_DIR ${HS_ROOT_DIR}/Assets)
message(STATUS "HS_ASSET_DIR: ${HS_ASSET_DIR}")

# set(HS_RES_SHADER_DIR ${HS_ASSET_DIR}/Shader)
# message(STATUS "HS_RES_SHADER_DIR: ${HS_RES_SHADER_DIR}")
set(HS_DEPS_DIR ${HS_ROOT_DIR}/Dependency)
message(STATUS "HS_DEPS_DIR: ${HS_DEPS_DIR}")

if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "arm64")
    set(HS_ARCH "arm64")
else()
    set(HS_ARCH "x64")
endif()

message(STATUS "Processor Architecture is ${HS_ARCH}")

if(APPLE)
    add_compile_definitions(__APPLE__)

    if(CMAKE_GENERATOR STREQUAL "Xcode")
        set(CMAKE_XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC NO)
    else()
        set(CMAKE_OBJC_FLAGS "-fobjc-arc")
        set(CMAKE_OBJCXX_FLAGS "-fobjc-arc")
    endif()

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -Wno-invalid-offsetof -Winline-new-delete -fno-rtti")

    set(HS_DEPS_LIB_DIR "${HS_DEPS_DIR}/lib/mac/$<CONFIG>")
    set(HS_DEPS_DLL_DIR "${HS_DEPS_DIR}/dll/mac/$<CONFIG>")
    set(HS_DEPS_BIN_DIR "${HS_DEPS_DIR}/bin/mac/")
    set(HS_DEPS_INCLUDE_DIR "${HS_DEPS_DIR}/include")
else()
    add_compile_definitions(__WINDOWS__)

    # Fix Unicode/UTF-8 encoding issues on Windows MSVC
    if(MSVC)
        add_compile_options(/utf-8)
    endif()

    set(HS_DEPS_LIB_DIR "${HS_DEPS_DIR}/lib/win/$<CONFIG>")
    set(HS_DEPS_DLL_DIR "${HS_DEPS_DIR}/dll/win/$<CONFIG>")
    set(HS_DEPS_BIN_DIR "${HS_DEPS_DIR}/bin/win")
    set(HS_DEPS_INCLUDE_DIR "${HS_DEPS_DIR}/include")
endif()

# =========================================
# Lightweight Prototyping Framework Options
# =========================================
option(HSMR_USE_SDL "Use SDL3 for platform abstraction" ON)
option(HSMR_ENABLE_IMGUI "Enable ImGui integration" ON)
option(HSMR_ENABLE_TRACY "Enable Tracy profiler" ON)
option(HSMR_SHADER_TRANSPILE_DEBUG "Dump runtime Slang transpile outputs" OFF)

# SDL3 Configuration
if(HSMR_USE_SDL)
    set(HS_SDL3_INCLUDE_DIR "${HS_DEPS_INCLUDE_DIR}/SDL3")

    if(WIN32)
        set(HS_SDL3_LIB "${HS_DEPS_LIB_DIR}/SDL3.lib")
        set(HS_SDL3_DLL "${HS_DEPS_DLL_DIR}/SDL3.dll")
    else()
        set(HS_SDL3_FRAMEWORK_DIR "${HS_DEPS_LIB_DIR}/SDL3.framework")
    endif()

    add_compile_definitions(__SDL__)
    message(STATUS "[SDL3] Enabled")
    message(STATUS "[SDL3] Include: ${HS_SDL3_INCLUDE_DIR}")

    if(WIN32)
        message(STATUS "[SDL3] Lib: ${HS_SDL3_LIB}")
    elseif(APPLE)
        message(STATUS "[SDL3] Framework Directory: ${HS_SDL3_FRAMEWORK_DIR}")
    endif()
endif()

if(HSMR_ENABLE_IMGUI)
    add_compile_definitions(HSMR_ENABLE_IMGUI)
    message(STATUS "[ImGui] Enabled")
endif()

# Tracy Profiler Configuration
if(HSMR_ENABLE_TRACY)
    set(HS_TRACY_INCLUDE_DIR "${HS_DEPS_INCLUDE_DIR}/tracy")
    set(HS_TRACY_CLIENT_SRC "${HS_TRACY_INCLUDE_DIR}/TracyClient.cpp")

    add_library(TracyClient STATIC ${HS_TRACY_CLIENT_SRC})
    target_include_directories(TracyClient PUBLIC ${HS_TRACY_INCLUDE_DIR})
    target_compile_definitions(TracyClient PUBLIC
        TRACY_ENABLE
        TRACY_ON_DEMAND
        TRACY_NO_EXIT
    )

    # Windows-specific settings
    if(WIN32)
        target_link_libraries(TracyClient PUBLIC ws2_32 dbghelp)
    endif()

    # Disable some warnings for Tracy on MSVC
    if(MSVC)
        target_compile_options(TracyClient PRIVATE /wd4996 /wd4267)
    endif()

    set_property(TARGET TracyClient PROPERTY FOLDER "ThirdParty")
    message(STATUS "[Tracy] Enabled (v0.11.1)")
    message(STATUS "[Tracy] Include: ${HS_TRACY_INCLUDE_DIR}")
endif()

# Slang Configuration (shader hot-reload support)
set(HS_SLANG_INCLUDE_DIR "${HS_DEPS_INCLUDE_DIR}/slang")

if(WIN32)
    set(HS_SLANG_LIB "${HS_DEPS_LIB_DIR}/slang.lib")
else()
    set(HS_SLANG_DYLIB "${HS_DEPS_DLL_DIR}/libslang.dylib")
endif()

message(STATUS "[Slang] Include: ${HS_SLANG_INCLUDE_DIR}")

set(INSTALL_FOLDER "$<TARGET_FILE_DIR:Client>")

set(CMAKE_XCODE_GENERATE_SCHEME TRUE)

include_directories(Source)
include_directories(SYSTEM ${HS_DEPS_INCLUDE_DIR})

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(HS_CORE_DIR "Source/Core")
set(HS_PLATFORM_DIR "Source/Platform")
set(HS_RHI_DIR "Source/RHI")
set(HS_RESOURCE_DIR "Source/Resource")
set(HS_RENDERER_DIR "Source/Renderer")
set(HS_SCENE_DIR "Source/Scene")
set(HS_ENGINE_DIR "Source/Engine")
set(HS_SHADER_SYSTEM_DIR "Source/ShaderSystem")
set(HS_EDITOR_DIR "Source/Editor")
set(HS_CLIENT_DIR "Source/Client")
set(HS_SHADER_DIR "Shader")

# =========================================
# 라이브러리 빌드 후 자동 설치 함수 (변경 시에만)
# =========================================
function(hs_auto_install TARGET_NAME)
    get_target_property(TARGET_TYPE ${TARGET_NAME} TYPE)

    if(TARGET_TYPE STREQUAL "STATIC_LIBRARY")
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${INSTALL_FOLDER}/dev/lib"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:${TARGET_NAME}>"
            "${INSTALL_FOLDER}/dev/lib/$<TARGET_FILE_NAME:${TARGET_NAME}>"
            COMMENT "[AUTO-INSTALL] ${TARGET_NAME} (if changed)"
            VERBATIM
        )
    elseif(TARGET_TYPE STREQUAL "SHARED_LIBRARY")
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${INSTALL_FOLDER}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:${TARGET_NAME}>"
            "${INSTALL_FOLDER}/$<TARGET_FILE_NAME:${TARGET_NAME}>"
            COMMENT "[AUTO-INSTALL] ${TARGET_NAME} (if changed)"
            VERBATIM
        )
    endif()
endfunction()

add_subdirectory(${HS_PLATFORM_DIR})
add_subdirectory(${HS_CORE_DIR})
add_subdirectory(${HS_RHI_DIR})
add_subdirectory(${HS_SHADER_SYSTEM_DIR})
add_subdirectory(${HS_RESOURCE_DIR})
add_subdirectory(${HS_RENDERER_DIR})
add_subdirectory(${HS_SCENE_DIR})
add_subdirectory(${HS_ENGINE_DIR})
add_subdirectory(${HS_EDITOR_DIR})
add_subdirectory(${HS_CLIENT_DIR})
add_subdirectory(${HS_SHADER_DIR})

add_dependencies(Client Shader)

add_custom_target(
    RegenerateCMake
    COMMAND ${CMAKE_COMMAND} -S ${CMAKE_SOURCE_DIR} -B ${CMAKE_BINARY_DIR}
    COMMENT "Regenerating CMake..."
)

set_property(TARGET RegenerateCMake PROPERTY FOLDER "Commands")

add_custom_target(CleanInstall
    COMMAND ${CMAKE_COMMAND} -E echo "========================================="
    COMMAND ${CMAKE_COMMAND} -E echo "Cleaning install directory..."
    COMMAND ${CMAKE_COMMAND} -E echo "Path: ${INSTALL_FOLDER}"
    COMMAND ${CMAKE_COMMAND} -E echo "========================================="
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${INSTALL_FOLDER}/Assets"
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${INSTALL_FOLDER}"
    COMMAND ${CMAKE_COMMAND} -E echo "Clean complete!"
    COMMENT "[CLEAN] Removing all installed files"
    VERBATIM
)

set_property(TARGET CleanInstall PROPERTY FOLDER "Commands")
