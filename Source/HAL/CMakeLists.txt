set(TARGET_NAME HAL)

set(TOTAL_FILES)

set(HAL_HEADERS
    HALDefinition.h
    FileSystem.h
    NativeWindow.h
    Simd.h
    SystemContext.h
    Timer.h
)
source_group("Public" FILES ${HAL_HEADERS})
list(APPEND TOTAL_FILES ${HAL_HEADERS})

set(HAL_SOURCES
    Private/Timer.cpp
    Private/FileSystem.cpp
    Private/NativeWindow.cpp
    Private/SystemContext.cpp
)
source_group("Private" FILES ${HAL_SOURCES})
list(APPEND TOTAL_FILES ${HAL_SOURCES})

if(WIN32)
    set(HAL_WIN_HEADERS
        Win/WinFileSystem.h
        Win/WinSystemContext.h
        Win/WinWindow.h
    )
    source_group("Win\\Public" FILES ${HAL_WIN_HEADERS})
    list(APPEND TOTAL_FILES ${HAL_WIN_HEADERS})

    set(HAL_WIN_SOURCES
        Win/Private/WinFileSystem.cpp
        Win/Private/WinSystemContext.cpp
        Win/Private/WinWindow.cpp
    )
    source_group("Win\\Private" FILES ${HAL_WIN_SOURCES})
    list(APPEND TOTAL_FILES ${HAL_WIN_SOURCES})
else() # MAC
    set(HAL_MAC_HEADERS
        Mac/AutoReleasePool.h
        Mac/MacFileSystem.h
        Mac/MacSystemContext.h
        Mac/MacWindow.h
    )
    source_group("Mac\\Public" FILES ${HAL_MAC_HEADERS})
    list(APPEND TOTAL_FILES ${HAL_MAC_HEADERS})

    set(HAL_MAC_HEADERS
        Mac/Private/AutoReleasePool.mm
        Mac/Private/MacFileSystem.mm
        Mac/Private/MacSystemContext.mm
        Mac/Private/MacWindow.mm
    )
    source_group("Mac\\Private" FILES ${HAL_MAC_HEADERS})
    list(APPEND TOTAL_FILES ${HAL_MAC_HEADERS})
endif()

if(HS_ARCH_X64)
    set(HAL_X64_HEADERS
        x64/SSESimd.h
    )

    source_group("x64\\Public" FILES ${HAL_X64_HEADERS})
    list(APPEND TOTAL_FILES ${HAL_X64_HEADERS})

    set(HAL_X64_SOURCES
        x64/Private/SSESimd.cpp
    )

    source_group("x64\\Private" FILES ${HAL_X64_SOURCES})
    list(APPEND TOTAL_FILES ${HAL_X64_SOURCES})
else()
    set(HAL_ARM64_HEADERS
        arm64/NeonSimd.h
    )

    source_group("arm64\\Public" FILES ${HAL_ARM64_HEADERS})
    list(APPEND TOTAL_FILES ${HAL_ARM64_HEADERS})

    set(HAL_ARM64_SOURCES
        arm64/Private/NeonSimd.cpp
    )

    source_group("x64\\Private" FILES ${HAL_ARM64_SOURCES})
    list(APPEND TOTAL_FILES ${HAL_ARM64_SOURCES})
endif()

add_library(${TARGET_NAME}
    STATIC
    ${TOTAL_FILES}
)

target_include_directories(${TARGET_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if(APPLE)
    target_link_libraries(${TARGET_NAME}
        PRIVATE
        "-framework Foundation -framework CoreFoundation -framework QuartzCore -framework AppKit -framework IOKit -framework Metal -framework MetalKit -framework GameController"
    )
endif()

target_compile_definitions(${TARGET_NAME}
    PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:MinSizeRel>:_RELEASE>
    $<$<CONFIG:Release>:_RELEASE>
    $<$<CONFIG:RelWithDebInfo>:_RELWITHDEBINFO>
    HS_HAL
    HS_API_EXPORT
)

install(TARGETS ${TARGET_NAME}
    ARCHIVE DESTINATION "${INSTALL_FOLDER}/dev/lib"
    LIBRARY DESTINATION "${INSTALL_FOLDER}"
    RUNTIME DESTINATION "${INSTALL_FOLDER}"
)