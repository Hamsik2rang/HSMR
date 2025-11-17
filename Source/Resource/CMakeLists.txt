set(TARGET_NAME Resource)

set(TOTAL_FILES)

if(APPLE)
    set(ASSIMP ${HS_DEPS_DLL_DIR}/libassimp.dylib)
    set(STB_INCLUDE ${HS_DEPS_INCLUDE_DIR}/stb)
else()
    set(ASSIMP ${HS_DEPS_LIB_DIR}/assimp-vc143-mt.lib)
    set(STB_INCLUDE ${HS_DEPS_INCLUDE_DIR}/stb)
endif()

set(OBJECT_HEADERS
    ResourceDefinition.h
    Image.h
    Material.h
    Mesh.h
    Shader.h
    Object.h
    ObjectManager.h
)

source_group("Public" FILES ${OBJECT_HEADERS})
list(APPEND TOTAL_FILES ${OBJECT_HEADERS})

set(OBJECT_SOURCES
    Private/ResourceDefinition.cpp
    Private/ObjectManager.cpp

    Private/Image.cpp
    Private/Material.cpp
    Private/Mesh.cpp
    Private/Shader.cpp
    Private/Object.cpp
)
source_group("Private" FILES ${OBJECT_SOURCES})
list(APPEND TOTAL_FILES ${OBJECT_SOURCES})

# Proxy system temporarily disabled for shader simplification
# set(PROXY_HEADERS
# Proxy/ImageProxy.h
# Proxy/MaterialProxy.h
# Proxy/MeshProxy.h
# Proxy/ShaderProxy.h
# Proxy/ObjectProxy.h
# )
#
# source_group("Proxy\\Public" FILES ${PROXY_HEADERS})
# list(APPEND TOTAL_FILES ${PROXY_HEADERS})
#
# set(PROXY_SOURCES
# Proxy/Private/ImageProxy.cpp
# Proxy/Private/MaterialProxy.cpp
# Proxy/Private/MeshProxy.cpp
# Proxy/Private/ShaderProxy.cpp
# Proxy/Private/ObjectProxy.cpp
# Proxy/Private/MaterialProxy.cpp
# )
# source_group("Proxy\\Private" FILES ${PROXY_SOURCES})
# list(APPEND TOTAL_FILES ${PROXY_SOURCES})
add_library(${TARGET_NAME} SHARED ${TOTAL_FILES})

add_dependencies(${TARGET_NAME} Platform Core RHI)

target_link_libraries(${TARGET_NAME} PRIVATE Platform Core RHI)

target_compile_definitions(${TARGET_NAME}
    PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:MinSizeRel>:_RELEASE>
    $<$<CONFIG:Release>:_RELEASE>
    $<$<CONFIG:RelWithDebInfo>:_RELWITHDEBINFO>
    HS_RESOURCE

    HS_PLATFORM_API_IMPORT
    HS_CORE_API_IMPORT
    HS_RHI_API_IMPORT

    HS_RESOURCE_API_EXPORT
)

target_Link_libraries(${TARGET_NAME} PRIVATE Platform Core RHI ${ASSIMP})

# Add STB include directory
target_include_directories(${TARGET_NAME} PRIVATE ${STB_INCLUDE})

if(APPLE)
    # On macOS, copy the .dylib file
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:${TARGET_NAME}>"
        "${INSTALL_FOLDER}/$<TARGET_FILE_NAME:${TARGET_NAME}>"
        COMMENT "Copying ${TARGET_NAME} library to install folder: ${INSTALL_FOLDER}"
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

install(TARGETS ${TARGET_NAME}
    ARCHIVE DESTINATION "${INSTALL_FOLDER}/dev/lib"
    LIBRARY DESTINATION "${INSTALL_FOLDER}"
    RUNTIME DESTINATION "${INSTALL_FOLDER}"
)