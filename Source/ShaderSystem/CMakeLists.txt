set(TARGET_NAME ShaderSystem)

option(USE_SHADERSYSTEM ON)

if(USE_SHADERSYSTEM)
    if(APPLE)
        set(SLANG "${HS_DEPS_DLL_DIR}/libslang.dylib")
        set(SLANG_LIB "${SLANG}")
    else()
        set(SLANG "${HS_DEPS_DLL_DIR}/slang.dll")
        set(SLANG_LIB "${HS_DEPS_LIB_DIR}/slang.lib")
    endif()

    message(STATUS "SLANG DIR: ${SLANG}")

    if(APPLE)
        # TODO: Vulkan SDK 1.3.296.0 기준으로 추가
        set(SPIRV_CROSS_C "${HS_DEPS_LIB_DIR}/libspirv-cross-c.a")
        set(SPIRV_CROSS_CORE "${HS_DEPS_LIB_DIR}/libspirv-cross-core.a")
        set(SPIRV_CROSS_CPP "${HS_DEPS_LIB_DIR}/libspirv-cross-cpp.a")
        set(SPIRV_CROSS_GLSL "${HS_DEPS_LIB_DIR}/libspirv-cross-glsl.a")
        set(SPIRV_CROSS_HLSL "${HS_DEPS_LIB_DIR}/libspirv-cross-hlsl.a")
        set(SPIRV_CROSS_MSL "${HS_DEPS_LIB_DIR}/libspirv-cross-msl.a")
        set(SPIRV_CROSS_REFLECT "${HS_DEPS_LIB_DIR}/libspirv-cross-reflect.a")
        set(SPIRV_CROSS_UTIL "${HS_DEPS_LIB_DIR}/libspirv-cross-util.a")
    else()
        set(SPIRV_CROSS_C "${HS_DEPS_LIB_DIR}/spirv-cross-c.lib")
        set(SPIRV_CROSS_CORE "${HS_DEPS_LIB_DIR}/spirv-cross-core.lib")
        set(SPIRV_CROSS_CPP "${HS_DEPS_LIB_DIR}/spirv-cross-cpp.lib")
        set(SPIRV_CROSS_GLSL "${HS_DEPS_LIB_DIR}/spirv-cross-glsl.lib")
        set(SPIRV_CROSS_HLSL "${HS_DEPS_LIB_DIR}/spirv-cross-hlsl.lib")
        set(SPIRV_CROSS_MSL "${HS_DEPS_LIB_DIR}/spirv-cross-msl.lib")
        set(SPIRV_CROSS_REFLECT "${HS_DEPS_LIB_DIR}/spirv-cross-reflect.lib")
        set(SPIRV_CROSS_UTIL "${HS_DEPS_LIB_DIR}/spirv-cross-util.lib")
    endif()

    set(SPIRV_CROSS ${SPIRV_CROSS_C} ${SPIRV_CROSS_CORE} ${SPIRV_CROSS_CPP} ${SPIRV_CROSS_GLSL} ${SPIRV_CROSS_HLSL} ${SPIRV_CROSS_MSL} ${SPIRV_CROSS_REFLECT} ${SPIRV_CROSS_UTIL})

    set(TOTAL_FILES)

    set(SHADERSYSTEM_HEADERS
        ShaderCrossCompiler.h
        ShaderCacheManager.h
    )

    source_group("Public" FILES ${SHADERSYSTEM_HEADERS})
    list(APPEND TOTAL_FILES ${SHADERSYSTEM_HEADERS})

    set(SHADERSYSTEM_SOURCES
        Private/ShaderCrossCompiler.cpp
        Private/ShaderCacheManager.cpp
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
        Slang/Private/SlangCompiler.cpp
        Slang/Private/SlangReflection.cpp
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
        Spirv/Private/SpirvCompiler.cpp
        Spirv/Private/SpirvCrossHelper.cpp
    )

    source_group("Spirv\\Private" FILES ${SHADERSYSTEM_SPIRV_SOURCES})
    list(APPEND TOTAL_FILES ${SHADERSYSTEM_SPIRV_SOURCES})

    add_library(${TARGET_NAME} SHARED ${TOTAL_FILES})

    add_dependencies(${TARGET_NAME} HAL Core)

    target_Link_libraries(${TARGET_NAME} PRIVATE Core ${SLANG_LIB} ${SPIRV_CROSS})

    if(APPLE)
        # On macOS, copy the .dylib file
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:${TARGET_NAME}>"
            "${INSTALL_FOLDER}/$<TARGET_FILE_NAME:${TARGET_NAME}>"
            COMMENT "Copying ${TARGET_NAME} library to install folder: ${INSTALL_FOLDER}"
        )
        
        # Copy Slang dylib to install folder
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SLANG}"
            "${INSTALL_FOLDER}/libslang.dylib"
            COMMENT "Copying libslang.dylib to install folder: ${INSTALL_FOLDER}"
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

    target_compile_definitions(${TARGET_NAME}
        PRIVATE
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<CONFIG:MinSizeRel>:_RELEASE>
        $<$<CONFIG:Release>:_RELEASE>
        $<$<CONFIG:RelWithDebInfo>:_RELWITHDEBINFO>
        HS_SHADERSYSTEM
        HS_SHADERSYSTEM_API_EXPORT
        HS_API_EXPORT
    )
endif()
