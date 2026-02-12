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
    Private/ShaderCompiler.cpp
    Private/ShaderCache.cpp
)

source_group("Private" FILES ${SHADER_SYSTEM_SOURCES})
list(APPEND TOTAL_FILES ${SHADER_SYSTEM_SOURCES})

add_library(${TARGET_NAME} SHARED ${TOTAL_FILES})

if(APPLE)
    target_link_libraries(${TARGET_NAME}
        PRIVATE
        Core Platform
        ${HS_SLANG_DYLIB}
    )

    if(HSMR_USE_SDL)
        target_compile_options(${TARGET_NAME} PRIVATE -F${HS_DEPS_LIB_DIR})
        target_link_options(${TARGET_NAME} PRIVATE -F${HS_DEPS_LIB_DIR})
        target_link_libraries(${TARGET_NAME} PRIVATE "-framework SDL3")
    endif()
else()
    target_link_libraries(${TARGET_NAME}
        PRIVATE
        Core Platform
        ${HS_SLANG_LIB}
    )
endif()

if(WIN32)
    target_link_options(${TARGET_NAME}
        PRIVATE
        "/WHOLEARCHIVE:Core"
        "/WHOLEARCHIVE:Platform"
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
    HS_SHADER_SYSTEM
    HS_SHADER_SYSTEM_API_EXPORT
)

install(TARGETS ${TARGET_NAME}
    ARCHIVE DESTINATION "${INSTALL_FOLDER}/dev/lib"
    LIBRARY DESTINATION "${INSTALL_FOLDER}"
    RUNTIME DESTINATION "${INSTALL_FOLDER}"
)

hs_auto_install(${TARGET_NAME})
