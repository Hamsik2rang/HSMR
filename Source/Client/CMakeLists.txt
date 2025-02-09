set(TARGET_NAME Client)

set(EXECUTABLE_NAME "HSMR")

set(COMMON_SOURCES
    main.mm
)

add_executable(${TARGET_NAME}
    ${COMMON_SOURCES}
)

list(APPEND OSX_FRAMEWORK "-framework OpenGL -framework Foundation -framework CoreFoundation -framework QuartzCore -framework AppKit -framework IOKit -framework Metal -framework GameController")

set_target_properties(${TARGET_NAME} PROPERTIES
    XCODE_ATTRIBUTE_INSTALL_PATH "/Client"
    OUTPUT_NAME ${EXECUTABLE_NAME}
    RUNTIME_OUTPUT_DIRECTORY ${HS_PROJECT_BINARY_DIR}
)

target_link_libraries(${TARGET_NAME}
    PUBLIC
    Engine
    Editor
)

target_compile_definitions(${TARGET_NAME} PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:MinSizeRel>:_RELEASE>
    $<$<CONFIG:Release>:_RELEASE>
    $<$<CONFIG:RelWithDebInfo>:_RELWITHDEBINFO>
    HS_CLIENT
    HS_EDITOR_API_IMPORT
    HS_EXECUTABLE_NAME="${EXECUTABLE_NAME}"
)

set(CMAKE_XCODE_GENERATE_SCHEME TRUE)
set_property(TARGET ${TARGET_NAME} PROPERTY XCODE_SCHEME_ENABLE_GPU_FRAME_CAPTURE_MODE Metal)

# set_property(TARGET ${TARGET_NAME} PROPERTY XCODE_GENERATE_DEBUGGING_SYMBOLS ON)
install(DIRECTORY ${HS_DEPS_DLL_DIR}/ DESTINATION "${INSTALL_FOLDER}")
install(DIRECTORY ${HS_RES_DIR}/ DESTINATION "${INSTALL_FOLDER}/Resource")
