set(TARGET_NAME ShaderSystem)

set(TOTAL_FILES)

set(SHADER_SYSTEM_HEADERS
    ShaderSystemDefinition.h
    ShaderCompiler.h
    ShaderCache.h
)

source_group("Public" FILES ${SHADER_SYSTEM_HEADERS})
list(APPEND TOTAL_FILES ${SHADER_SYSTEM_HEADERS})

set(SHADER_SYSTEM_SOURCES
    Private/ShaderSystemDefinition.cpp
    Private/ShaderCompiler.cpp
    Private/ShaderCache.cpp
)

source_group("Private" FILES ${SHADER_SYSTEM_SOURCES})
list(APPEND TOTAL_FILES ${SHADER_SYSTEM_SOURCES})

add_library(${TARGET_NAME} SHARED ${TOTAL_FILES})

if(APPLE)
    target_link_libraries(${TARGET_NAME}
        PRIVATE
        Core
        ${HS_SLANG_DYLIB}
    )
else()
    target_link_libraries(${TARGET_NAME}
        PRIVATE
        Core
        ${HS_SLANG_LIB}
    )
endif()

target_include_directories(${TARGET_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${HS_SLANG_INCLUDE_DIR}
)

target_compile_definitions(${TARGET_NAME}
    PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:MinSizeRel>:_RELEASE>
    $<$<CONFIG:Release>:_RELEASE>
    $<$<CONFIG:RelWithDebInfo>:_RELWITHDEBINFO>
    $<$<BOOL:${HSMR_SHADER_TRANSPILE_DEBUG}>:HSMR_SHADER_TRANSPILE_DEBUG=1>
    $<$<BOOL:${HSMR_SHADER_TRANSPILE_DEBUG}>:HSMR_SHADER_TRANSPILE_DEBUG_ROOT="${HS_PROJECT_BINARY_DIR}/ShaderTranspileDebug">
    HS_SHADER_SYSTEM
    HS_SHADER_SYSTEM_API_EXPORT
    HS_CORE_API_IMPORT
)

install(TARGETS ${TARGET_NAME}
    ARCHIVE DESTINATION "${INSTALL_FOLDER}/dev/lib"
    LIBRARY DESTINATION "${INSTALL_FOLDER}"
    RUNTIME DESTINATION "${INSTALL_FOLDER}"
)

hs_auto_install(${TARGET_NAME})
