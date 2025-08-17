set(TARGET_NAME Physics)

set(TOTAL_FILES)

set(PHYSICS_HEADERS
    PhysicsDefinition.h
)

source_group("Public" FILES ${PHYSICS_HEADERS})
list(APPEND TOTAL_FILES ${PHYSICS_HEADERS})

set(PHYSICS_SOURCES
    Private/PhysicsDefinition.cpp
)
source_group("Private" FILES ${PHYSICS_SOURCES})
list(APPEND TOTAL_FILES ${PHYSICS_SOURCES})

add_library(${TARGET_NAME} SHARED ${TOTAL_FILES})

add_dependencies(${TARGET_NAME} HAL Core Object)

target_include_directories(${TARGET_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(${TARGET_NAME}
    PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:MinSizeRel>:_RELEASE>
    $<$<CONFIG:Release>:_RELEASE>
    $<$<CONFIG:RelWithDebInfo>:_RELWITHDEBINFO>
    HS_PHYSICS

    HS_PHYSICS_API_EXPORT

    HS_OBJECT_API_IMPORT
)

target_Link_libraries(${TARGET_NAME} PRIVATE HAL Core Object)

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