# if(APPLE)
# set(TARGET_NAME Shader)

# set(TOTAL_FILES
# MSL/Basic.metal
# )

# source_group("MSL" FILES ${TOTAL_FILES})

# set_source_files_properties(${TOTAL_FILES} PROPERTIES LANGUAGE METAL)

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=macos-metal3.0")
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=macos-metal3.0")

# add_library(${TARGET_NAME} STATIC ${TOTAL_FILES})

# target_link_libraries(${TARGET_NAME} "-framework Metal -framework MetalPerformanceShaders")

# # set(TOTAL_FILES)

# # add_library(${TARGET_NAME} STATIC ${TOTAL_FILES})
# set_target_properties(${TARGET_NAME} PROPERTIES
# RUNTIME_OUTPUT_DIRECTORY ${HS_PROJECT_BINARY_DIR}
# LIBRARY_OUTPUT_DIRECTORY ${HS_PROJECT_BINARY_DIR}
# ARCHIVE_OUTPUT_DIRECTORY ${HS_PROJECT_BINARY_DIR}
# XCODE_ATTRIBUTE_MTL_ENABLE_DEBUG_INFO "INCLUDE_SOURCE"
# )
# else()
# endif()

set(TARGET_NAME Shader)

if(APPLE)
    # Metal 셰이더 파일들
    file(GLOB_RECURSE METAL_SHADERS "MSL/*.metal")

    # 출력 경로 설정
    set(METALLIB_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    set(METALLIB_FILE "${METALLIB_OUTPUT_DIR}/default.metallib")

    # Metal 컴파일러 찾기
    find_program(METAL_COMPILER xcrun)

    if(NOT METAL_COMPILER)
        message(FATAL_ERROR "xcrun not found - Metal compilation not available")
    endif()

    # .metallib 생성을 위한 커스텀 명령
    add_custom_command(
        OUTPUT ${METALLIB_FILE}
        COMMAND ${METAL_COMPILER} -sdk macosx metal
        -std=metal3.2
        -I${CMAKE_CURRENT_SOURCE_DIR}/MSL
        -c ${METAL_SHADERS}
        -o ${METALLIB_OUTPUT_DIR}/temp.air
        COMMAND ${METAL_COMPILER} -sdk macosx metallib
        ${METALLIB_OUTPUT_DIR}/temp.air
        -o ${METALLIB_FILE}
        DEPENDS ${METAL_SHADERS}
        COMMENT "Compiling Metal shaders to default.metallib"
        VERBATIM
    )

    # 커스텀 타겟 생성
    add_custom_target(${TARGET_NAME} ALL
        DEPENDS ${METALLIB_FILE}
        SOURCES ${METAL_SHADERS}
    )

    # 소스 그룹 설정 (IDE에서 보기 좋게)
    source_group("MSL" FILES ${METAL_SHADERS})

    # 빌드 속성 설정
    set_target_properties(${TARGET_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        XCODE_ATTRIBUTE_MTL_ENABLE_DEBUG_INFO "INCLUDE_SOURCE"
    )

    # 전역 변수로 metallib 경로 설정 (Client에서 사용)
    set(METALLIB_FILE ${METALLIB_FILE} PARENT_SCOPE)

    message(STATUS "Metal lib location: ${METALLIB_FILE}")

else() # Windows/Linux의 경우 HLSL 또는 GLSL 처리
    # TODO: 추후 구현
endif()