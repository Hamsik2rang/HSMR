cmake_minimum_required(VERSION 3.22.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(ENGINE_NAME "HSMR")

project(${ENGINE_NAME} VERSION 0.1.0)

set(HS_ROOT_DIR ${HSMR_SOURCE_DIR})
message(STATUS "HS_ROOT_DIR: ${HS_ROOT_DIR}")

set(HS_BIN_DIR ${HSMR_BINARY_DIR})
message(STATUS "HS_BIN_DIR: ${HS_BIN_DIR}")

set(HS_PROJECT_BINARY_DIR ${PROJECT_BINARY_DIR})

set(HS_SRC_DIR ${HS_ROOT_DIR}/Source)
message(STATUS "HS_SRC_DIR: ${HS_SRC_DIR}")

set(HS_RES_DIR ${HS_ROOT_DIR}/Resource)
message(STATUS "HS_RES_DIR: ${HS_RES_DIR}")

set(HS_RES_SHADER_DIR ${HS_RES_DIR}/Shader)
message(STATUS "HS_RES_SHADER_DIR: ${HS_RES_SHADER_DIR}")

set(HS_DEPS_DIR ${HS_ROOT_DIR}/Dependency)
message(STATUS "HS_DEPS_DIR: ${HS_DEPS_DIR}")

if(APPLE)
    add_compile_definitions(__APPLE__)

    # if(NOT APPLE)
    # message(FATAL_ERROR "Currently, HSMR is only support MacOS.")
    # endif()
    if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "arm64")
        set(HS_ARCH "arm64")
    else()
        set(HS_ARCH "x86_64")
    endif()

    if(CMAKE_GENERATOR STREQUAL "Xcode")
        set(CMAKE_XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES)
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
    add_compile_definitions(__WIN32__)

    set(HS_DEPS_LIB_DIR "${HS_DEPS_DIR}/lib/win/$<CONFIG>")
    set(HS_DEPS_DLL_DIR "${HS_DEPS_DIR}/dll/win/$<CONFIG>")
    set(HS_DEPS_INCLUDE_DIR "${HS_DEPS_DIR}/include")
endif()

message(STATUS "Processor Architecture is ${HS_ARCH}")

if(APPLE)
    set(ASSIMP ${HS_DEPS_DLL_DIR}/libassimp.5.4.3.dylib)
else() # TODO
    set(ASSIMP)
endif()

if(WIN32)
    set(VULKAN ${HS_DEPS_LIB_DIR}/vulkan-1.lib)
    set(SPIRV ${HS_DEPS_LIB_DIR}/SPIRV.lib)
else()
    set(VULKAN)
    set(SPIRV)
endif()

set(INSTALL_FOLDER "$<TARGET_FILE_DIR:Client>")

set(CMAKE_XCODE_GENERATE_SCHEME TRUE)

include_directories(Source)
include_directories(SYSTEM ${HS_DEPS_INCLUDE_DIR})

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(HS_ENGINE_DIR "Source/Engine")
set(HS_EDITOR_DIR "Source/Editor")
set(HS_CLIENT_DIR "Source/Client")

add_subdirectory(${HS_RES_SHADER_DIR})
add_subdirectory(${HS_ENGINE_DIR})
add_subdirectory(${HS_EDITOR_DIR})
add_subdirectory(${HS_CLIENT_DIR})

add_custom_target(
    RegenerateCMake
    COMMAND ${CMAKE_COMMAND} -S ${CMAKE_SOURCE_DIR} -B ${CMAKE_BINARY_DIR}
    COMMENT "Regenerating CMake..."
)