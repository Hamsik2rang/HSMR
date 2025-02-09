set(TARGET_NAME Shader)

set(TOTAL_FILES
    MSL/Basic.metal
)

source_group("MSL" FILES ${TOTAL_FILES})

set_source_files_properties(${TOTAL_FILES} PROPERTIES LANGUAGE METAL)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=macos-metal3.0")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=macos-metal3.0")

add_library(${TARGET_NAME} STATIC ${TOTAL_FILES})

set_target_properties(${TARGET_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${HS_PROJECT_BINARY_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${HS_PROJECT_BINARY_DIR}
    ARCHIVE_OUTPUT_DIRECTORY ${HS_PROJECT_BINARY_DIR}
)

target_link_libraries(${TARGET_NAME} "-framework Metal -framework MetalPerformanceShaders")
