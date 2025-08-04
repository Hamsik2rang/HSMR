set(TARGET_NAME ShaderSystem)

set(USE_SHADERSYSTEM YES)

if(USE_SHADERSYSTEM)
    if(APPLE)
        set(SLANG "${HS_DEPS_DLL_DIR}/slang.dylib")
        set(SLANG_LIB "${HS_DEPS_LIB_DIR}/slang.a")
    else()
        set(SLANG "${HS_DEPS_DLL_DIR}/slang.dll")
        set(SLANG_LIB "${HS_DEPS_LIB_DIR}/slang.lib")
    endif()

    message(STATUS "SLANG DIR: ${SLANG}")

    if(APPLE)
        # TODO: Vulkan SDK 1.3.296.0 기준으로 추가
        set(SPIRV_CROSS_C "${HS_DEPS_LIB_DIR}/spirv-cross-c.a")
        set(SPIRV_CROSS_CORE "${HS_DEPS_LIB_DIR}/spirv-cross-core.a")
        set(SPIRV_CROSS_CPP "${HS_DEPS_LIB_DIR}/spirv-cross-cpp.a")
        set(SPIRV_CROSS_HLSL "${HS_DEPS_LIB_DIR}/spirv-cross-hlsl.a")
        set(SPIRV_CROSS_MSL "${HS_DEPS_LIB_DIR}/spirv-cross-msl.a")
        set(SPIRV_CROSS_REFLECT "${HS_DEPS_LIB_DIR}/spirv-cross-reflect.a")
        set(SPIRV_CROSS_UTIL "${HS_DEPS_LIB_DIR}/spirv-cross-util.a")
    else()
        set(SPIRV_CROSS_C "${HS_DEPS_LIB_DIR}/spirv-cross-c.lib")
        set(SPIRV_CROSS_CORE "${HS_DEPS_LIB_DIR}/spirv-cross-core.lib")
        set(SPIRV_CROSS_CPP "${HS_DEPS_LIB_DIR}/spirv-cross-cpp.lib")
        set(SPIRV_CROSS_HLSL "${HS_DEPS_LIB_DIR}/spirv-cross-hlsl.lib")
        set(SPIRV_CROSS_MSL "${HS_DEPS_LIB_DIR}/spirv-cross-msl.lib")
        set(SPIRV_CROSS_REFLECT "${HS_DEPS_LIB_DIR}/spirv-cross-reflect.lib")
        set(SPIRV_CROSS_UTIL "${HS_DEPS_LIB_DIR}/spirv-cross-util.lib")
    endif()

    set(SPIRV_CROSS ${SPIRV_CROSS_C} ${SPIRV_CROSS_CORE} ${SPIRV_CROSS_CPP} ${SPIRV_CROSS_HLSL} ${SPIRV_CROSS_MSL} ${SPIRV_CROSS_REFLECT} ${SPIRV_CROSS_UTIL})

    set(TOTAL_FILES)

    set(SHADERSYSTEM_HEADERS
        ShaderCrossCompiler.h
        ShaderCacheManager.h
    )

    source_group("Public" FILES ${SHADERSYSTEM_HEADERS})
    list(APPEND TOTAL_FILES ${SHADERSYSTEM_HEADERS})

    set(SHADERSYSTEM_SOURCES
        ShaderCrossCompiler.cpp
        ShaderCacheManager.cpp
    )

    source_group("Private" FILES ${SHADERSYSTEM_SOURCES})
    list(APPEND TOTAL_FILES ${SHADERSYSTEM_SOURCES})

    set(SHADERSYSTEM_SLANG_HEADERS
        Slang/SlangCompiler.h
        Slang/SlangReflection.h
    )

    source_group("Slang\\Public" FILES ${SHADERSYSTEM_SLANG_HEADERS})
    list(APPEND TOTAL_FILES ${SHADERSYSTEM_SLANG_HEADERS})

    set(SHADERSYSTEM_SLANG_SOURCES
        Slang/SlangCompiler.cpp
        Slang/SlangReflection.cpp
    )

    source_group("Slang\\Private" FILES ${SHADERSYSTEM_SLANG_SOURCES})
    list(APPEND TOTAL_FILES ${SHADERSYSTEM_SLANG_SOURCES})

    set(SHADERSYSTEM_SPIRV_HEADERS
        Spirv/SpirvCompiler.h
        Spirv/SpirvCrossHelper.h
    )

    source_group("Spirv\\Public" FILES ${SHADERSYSTEM_SPIRV_HEADERS})
    list(APPEND TOTAL_FILES ${SHADERSYSTEM_SPIRV_HEADERS})

    set(SHADERSYSTEM_SPIRV_SOURCES
        Spirv/SpirvCompiler.cpp
        Spirv/SpirvCrossHelper.cpp
    )

    source_group("Spirv\\Private" FILES ${SHADERSYSTEM_SPIRV_SOURCES})
    list(APPEND TOTAL_FILES ${SHADERSYSTEM_SPIRV_SOURCES})

    add_library(${TARGET_NAME} SHARED ${TOTAL_FILES})

    target_Link_libraries(${TARGET_NAME} PRIVATE Engine ${SLANG_LIB} ${SPIRV_CROSS})
endif()