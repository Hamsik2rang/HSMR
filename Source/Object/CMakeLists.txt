set(TARGET_NAME Object)

set(TOTAL_FILES)

if(APPLE)
    set(ASSIMP ${HS_DEPS_DLL_DIR}/libassimp.5.4.3.dylib)
else() # TODO
    set(ASSIMP ${HS_DEPS_LIB_DIR}/assimp-vc143-mt.lib)
endif()

set(OBJECT_HEADERS
    Image.h
    Material.h
    Mesh.h
    Object.h
    ObjectManager.h
)

source_group("Public" FILES ${OBJECT_HEADERS})
list(APPEND TOTAL_FILES ${OBJECT_HEADERS})

set(OBJECT_SOURCES
    Private/Image.cpp
    Private/Material.cpp
    Private/Mesh.cpp
    Private/Object.cpp
    Private/ObjectManager.cpp
)
source_group("Private" FILES ${OBJECT_SOURCES})
list(APPEND TOTAL_FILES ${OBJECT_SOURCES})

add_library(${TARGET_NAME} SHARED ${TOTAL_FILES})

add_dependencies(${TARGET_NAME} HAL Core RHI)

target_compile_definitions(${TARGET_NAME}
    PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:MinSizeRel>:_RELEASE>
    $<$<CONFIG:Release>:_RELEASE>
    $<$<CONFIG:RelWithDebInfo>:_RELWITHDEBINFO>
    HS_OBJECT

    HS_OBJECT_API_EXPORT
)

target_Link_libraries(${TARGET_NAME} PRIVATE HAL Core RHI ${ASSIMP})

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