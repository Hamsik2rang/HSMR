cmake_minimum_required(VERSION 3.29.0)

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

set(HS_DEPS_DIR ${HS_ROOT_DIR}/Dependency)
message(STATUS "HS_DEPS_DIR: ${HS_DEPS_DIR}")

if(NOT APPLE)
    message(FATAL_ERROR "Currently, HSMR is only support MacOS.")
endif()

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

message(STATUS "Processor Architecture is ${HS_ARCH}")

set(HS_DEPS_LIB_DIR ${HS_DEPS_DIR}/lib)
set(HS_DEPS_DLL_DIR ${HS_DEPS_DIR}/dll)
set(HS_DEPS_INCLUDE_DIR ${HS_DEPS_DIR}/include)

set(SDL3 ${HS_DEPS_DLL_DIR}/libSDL3.dylib)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-invalid-offsetof -Winline-new-delete -fno-rtti")

set(INSTALL_FOLDER "$<TARGET_FILE_DIR:Client>")

set(CMAKE_XCODE_GENERATE_SCHEME TRUE)

include_directories(Source)
include_directories(SYSTEM ${HS_DEPS_INCLUDE_DIR})

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(HS_ENGINE_DIR "Source/Engine")
set(HS_EDITOR_DIR "Source/Editor")
set(HS_CLIENT_DIR "Source/Client")

add_subdirectory(${HS_ENGINE_DIR})
add_subdirectory(${HS_EDITOR_DIR})
add_subdirectory(${HS_CLIENT_DIR})

add_custom_target(
    Rebuild CMake
    COMMAND ${CMAKE_COMMAND} -S ${CMAKE_SOURCE_DIR} -B ${CMAKE_BINARY_DIR}
    COMMENT "Rebuilding CMake..."
)