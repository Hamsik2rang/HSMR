set(TARGET_NAME Profiler)

set(PROFILER_HEADERS
    Profiler.h
    GPUQuery.h
)

source_group("Public" FILES ${PROFILER_HEADERS})

set(PROFILER_SOURCES
    Profiler.cpp
    GPUQuery.cpp
)

source_group("Private" FILES ${PROFILER_SOURCES})

set(TOTAL_FILES
    ${PROFILER_HEADERS}
    ${PROFILER_SOURCES}
)

# Add ImPlot sources if available
if(IMPLOT_FOUND)
    list(APPEND TOTAL_FILES ${IMPLOT_SOURCES})
    source_group("ThirdParty\\ImPlot" FILES ${IMPLOT_SOURCES})
endif()

add_library(${TARGET_NAME}
    SHARED
    ${TOTAL_FILES}
)

# Link dependencies - Profiler sits between Engine and Editor layer
# Needs Editor for ImGui symbols (ImPlot depends on ImGui)
if(WIN32)
    target_link_libraries(${TARGET_NAME}
        PRIVATE
        Engine
        Editor
    )
else()
    target_link_libraries(${TARGET_NAME}
        PRIVATE
        Engine
        Editor
    )
endif()

target_include_directories(${TARGET_NAME}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC ${HS_SRC_DIR}
    PUBLIC ${HS_SRC_DIR}/Editor/ThirdParty/ImGui
)

# Add ImPlot include directory and define
if(IMPLOT_FOUND)
    target_include_directories(${TARGET_NAME} PUBLIC ${IMPLOT_INCLUDE_DIR})
    target_compile_definitions(${TARGET_NAME} PUBLIC HAS_IMPLOT=1)
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

    HS_PROFILER_API_EXPORT
    HS_API_IMPORT
    HS_EDITOR_API_IMPORT
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
