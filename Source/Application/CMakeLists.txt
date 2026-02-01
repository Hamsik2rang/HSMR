set(TARGET_NAME Application)

set(APPLICATION_HEADERS
    Camera.h
    Scene.h
    SceneObject.h
    MousePicker.h
    PrototypeApplication.h
    GizmoController.h
)

source_group("Public" FILES ${APPLICATION_HEADERS})

set(APPLICATION_SOURCES
    Camera.cpp
    Scene.cpp
    SceneObject.cpp
    MousePicker.cpp
    PrototypeApplication.cpp
    GizmoController.cpp
)

source_group("Private" FILES ${APPLICATION_SOURCES})

set(TOTAL_FILES
    ${APPLICATION_HEADERS}
    ${APPLICATION_SOURCES}
)

# Add ImGuizmo sources if available
if(IMGUIZMO_FOUND)
    list(APPEND TOTAL_FILES ${IMGUIZMO_SOURCES})
    source_group("ThirdParty\\ImGuizmo" FILES ${IMGUIZMO_SOURCES})
endif()

add_library(${TARGET_NAME}
    SHARED
    ${TOTAL_FILES}
)

# Link dependencies
if(WIN32)
    target_link_libraries(${TARGET_NAME}
        PRIVATE
        Engine
        Editor
    )
else()
    target_link_libraries(${TARGET_NAME}
        PRIVATE
        "-Wl,-all_load" Engine
        Editor
    )
endif()

target_include_directories(${TARGET_NAME}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC ${HS_SRC_DIR}
    PUBLIC ${HS_SRC_DIR}/Editor/ThirdParty/ImGui
)

# Add ImGuizmo include directory and define
if(IMGUIZMO_FOUND)
    target_include_directories(${TARGET_NAME} PUBLIC ${IMGUIZMO_INCLUDE_DIR})
    target_compile_definitions(${TARGET_NAME} PUBLIC HAS_IMGUIZMO=1)
endif()

set_target_properties(${TARGET_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${HS_EXECUTABLE_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${HS_EXECUTABLE_DIR}
)

add_dependencies(${TARGET_NAME} Engine Editor)

target_compile_definitions(${TARGET_NAME}
    PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:MinSizeRel>:_RELEASE>
    $<$<CONFIG:Release>:_RELEASE>
    $<$<CONFIG:RelWithDebInfo>:_RELWITHDEBINFO>

    HS_APPLICATION_API_EXPORT
    HS_EDITOR_API_IMPORT
    HS_API_IMPORT
)

# Install rules
if(APPLE)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:${TARGET_NAME}>"
        "${INSTALL_FOLDER}/$<TARGET_FILE_NAME:${TARGET_NAME}>"
        COMMENT "Copying ${TARGET_NAME} library to install folder: ${INSTALL_FOLDER}"
    )
else()
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:${TARGET_NAME}>"
        "${INSTALL_FOLDER}/$<TARGET_FILE_NAME:${TARGET_NAME}>"
        COMMENT "Copying ${TARGET_NAME}.dll to install folder: ${INSTALL_FOLDER}"
    )
endif()

install(TARGETS ${TARGET_NAME}
    ARCHIVE DESTINATION "${INSTALL_FOLDER}/dev/lib"
    LIBRARY DESTINATION "${INSTALL_FOLDER}"
    RUNTIME DESTINATION "${INSTALL_FOLDER}")
