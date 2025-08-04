set(TARGET_NAME Client)

set(EXECUTABLE_NAME "HSMR")

set(TOTAL_FILES
    main.cpp
)

if(APPLE)
    list(APPEND OSX_FRAMEWORK "-framework OpenGL -framework Foundation -framework CoreFoundation -framework QuartzCore -framework AppKit -framework IOKit -framework Metal -framework GameController")
    add_executable(${TARGET_NAME}
        ${TOTAL_FILES}
    )
    set_target_properties(${TARGET_NAME} PROPERTIES
        XCODE_ATTRIBUTE_INSTALL_PATH "/Client"
        OUTPUT_NAME ${EXECUTABLE_NAME}
        RUNTIME_OUTPUT_DIRECTORY ${HS_PROJECT_BINARY_DIR}
    )

    target_link_libraries(${TARGET_NAME}
        PRIVATE
        Engine
        Editor
        ${OSX_FRAMEWORK}
    )
else()
    add_executable(${TARGET_NAME}
        ${TOTAL_FILES}
    )
    set_target_properties(${TARGET_NAME} PROPERTIES
        OUTPUT_NAME ${EXECUTABLE_NAME}
        RUNTIME_OUTPUT_DIRECTORY ${HS_PROJECT_BINARY_DIR})

    target_link_libraries(${TARGET_NAME}
        PRIVATE
        Engine
        Editor
    )
endif()

add_dependencies(${TARGET_NAME} Engine Editor)

target_compile_definitions(${TARGET_NAME} PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:MinSizeRel>:_RELEASE>
    $<$<CONFIG:Release>:_RELEASE>
    $<$<CONFIG:RelWithDebInfo>:_RELWITHDEBINFO>
    HS_CLIENT
    HS_EDITOR_API_IMPORT
    HS_EXECUTABLE_NAME="${EXECUTABLE_NAME}"
)

if(APPLE)
    set(CMAKE_XCODE_GENERATE_SCHEME TRUE)
    set_property(TARGET ${TARGET_NAME} PROPERTY XCODE_SCHEME_ENABLE_GPU_FRAME_CAPTURE_MODE Metal)
    set_property(TARGET ${TARGET_NAME} PROPERTY XCODE_ATTRIBUTE_MTL_ENABLE_DEBUG_INFO "INCLUDE_SOURCE")
endif()

# set_property(TARGET ${TARGET_NAME} PROPERTY XCODE_GENERATE_DEBUGGING_SYMBOLS ON)
message(STATUS "HS_DEPS_DLL_DIR : ${HS_DEPS_DLL_DIR}")
message(STATUS "HS_DEPS_LIB_DIR : ${HS_DEPS_LIB_DIR}")
message(STATUS "INSTALL_FOLDER: ${INSTALL_FOLDER}")

install(DIRECTORY "${HS_DEPS_DLL_DIR}/" DESTINATION "${INSTALL_FOLDER}")
install(DIRECTORY "${HS_DEPS_LIB_DIR}/" DESTINATION "${INSTALL_FOLDER}")
install(DIRECTORY "${HS_RES_DIR}/" DESTINATION "${INSTALL_FOLDER}/Resource")

# install(FILES "${METALLIB_FILE}/" DESTINATION "${INSTALL_FOLDER}/Resource/Shader")