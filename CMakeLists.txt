cmake_minimum_required(VERSION 3.22.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(ENGINE_NAME "HSMR")

project(${ENGINE_NAME} VERSION 0.1.0)

set(HS_ROOT_DIR ${HSMR_SOURCE_DIR})
message(STATUS "HS_ROOT_DIR: ${HS_ROOT_DIR}")

set(HS_EXE_BIN_DIR "${HSMR_BINARY_DIR}$<CONFIG>")
message(STATUS "HS_EXE_BIN_DIR: ${HS_EXE_BIN_DIR}")

set(HS_PROJECT_BINARY_DIR ${PROJECT_BINARY_DIR})
message(STATUS "HS_PROJECT_BINARY_DIR: ${PROJECT_BINARY_DIR}")

set(HS_SRC_DIR ${HS_ROOT_DIR}/Source)
message(STATUS "HS_SRC_DIR: ${HS_SRC_DIR}")

set(HS_RES_DIR ${HS_ROOT_DIR}/Resource)
message(STATUS "HS_RES_DIR: ${HS_RES_DIR}")

# set(HS_RES_SHADER_DIR ${HS_RES_DIR}/Shader)
# message(STATUS "HS_RES_SHADER_DIR: ${HS_RES_SHADER_DIR}")
set(HS_DEPS_DIR ${HS_ROOT_DIR}/Dependency)
message(STATUS "HS_DEPS_DIR: ${HS_DEPS_DIR}")

if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "arm64")
    set(HS_ARCH "arm64")
    set(HS_ARCH_ARM64)
else()
    set(HS_ARCH "x64")
    set(HS_ARCH_X64)
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
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wno-invalid-offsetof -Winline-new-delete -fno-rtti")

    set(HS_DEPS_LIB_DIR "${HS_DEPS_DIR}/lib/mac/$<CONFIG>")
    set(HS_DEPS_DLL_DIR "${HS_DEPS_DIR}/dll/mac/$<CONFIG>")
    set(HS_DEPS_INCLUDE_DIR "${HS_DEPS_DIR}/include")
else()
    add_compile_definitions(__WINDOWS__)

    # Fix Unicode/UTF-8 encoding issues on Windows MSVC
    if(MSVC)
        add_compile_options(/utf-8)
    endif()

    set(HS_DEPS_LIB_DIR "${HS_DEPS_DIR}/lib/win/$<CONFIG>")
    set(HS_DEPS_DLL_DIR "${HS_DEPS_DIR}/dll/win/$<CONFIG>")
    set(HS_DEPS_INCLUDE_DIR "${HS_DEPS_DIR}/include")
endif()

if(HS_ARCH_ARM64)
    add_compile_definitions(__ARM64__)
else()
    add_compile_definitions(__X64__)
endif()

set(INSTALL_FOLDER "$<TARGET_FILE_DIR:Client>")

set(CMAKE_XCODE_GENERATE_SCHEME TRUE)

include_directories(Source)
include_directories(SYSTEM ${HS_DEPS_INCLUDE_DIR})

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(HS_HAL_DIR "Source/HAL")
set(HS_CORE_DIR "Source/Core")
set(HS_RESOURCE_OBJECT_DIR "Source/Object")
set(HS_SHADERSYSTEM_DIR "Source/ShaderSystem")
set(HS_RHI_DIR "Source/RHI")
set(HS_PHYSICS_DIR "Source/Physics")
set(HS_RENDERER_DIR "Source/Renderer")
set(HS_ANIMATION_DIR "Source/Animation")
set(HS_GEOMETRY_DIR "Source/Geometry")
set(HS_ECS_DIR "Source/ECS")
set(HS_ENGINE_DIR "Source/Engine")
set(HS_EDITOR_DIR "Source/Editor")
set(HS_CLIENT_DIR "Source/Client")
set(HS_SHADERSYSTEM_DIR "Source/ShaderSystem")

add_subdirectory(${HS_HAL_DIR})
add_subdirectory(${HS_CORE_DIR})

add_subdirectory(${HS_RESOURCE_OBJECT_DIR})
add_subdirectory(${HS_SHADERSYSTEM_DIR})

add_subdirectory(${HS_RHI_DIR})
add_subdirectory(${HS_GEOMETRY_DIR})
add_subdirectory(${HS_PHYSICS_DIR})

add_subdirectory(${HS_RENDERER_DIR})

# add_subdirectory(${HS_ANIMATION_DIR})
add_subdirectory(${HS_ECS_DIR})

add_subdirectory(${HS_ENGINE_DIR})

add_subdirectory(${HS_EDITOR_DIR})

add_subdirectory(${HS_CLIENT_DIR})
add_custom_target(
    RegenerateCMake
    COMMAND ${CMAKE_COMMAND} -S ${CMAKE_SOURCE_DIR} -B ${CMAKE_BINARY_DIR}
    COMMENT "Regenerating CMake..."
)

set_property(TARGET RegenerateCMake PROPERTY FOLDER "Commands")