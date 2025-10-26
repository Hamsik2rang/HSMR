set(TARGET_NAME Platform)

set(TOTAL_FILES)

if(WIN32)
    set(PLATFORM_WIN_HEADERS
        Win/WinFileSystem.h
        Win/WinSystemContext.h
        Win/WinWindow.h
        Win/WinInput.h
    )
    source_group("Win\\Public" FILES ${PLATFORM_WIN_HEADERS})
    list(APPEND TOTAL_FILES ${PLATFORM_WIN_HEADERS})

    set(PLATFORM_WIN_SOURCES
        Win/Private/WinFileSystem.cpp
        Win/Private/WinSystemContext.cpp
        Win/Private/WinWindow.cpp
        Win/Private/WinInput.cpp
    )
    source_group("Win\\Private" FILES ${PLATFORM_WIN_SOURCES})
    list(APPEND TOTAL_FILES ${PLATFORM_WIN_SOURCES})
else() # MAC
    set(PLATFORM_MAC_HEADERS
        Mac/AutoReleasePool.h
        Mac/MacFileSystem.h
        Mac/MacSystemContext.h
        Mac/MacWindow.h
        Mac/MacInput.h
    )
    source_group("Mac\\Public" FILES ${PLATFORM_MAC_HEADERS})
    list(APPEND TOTAL_FILES ${PLATFORM_MAC_HEADERS})

    set(PLATFORM_MAC_HEADERS
        Mac/Private/AutoReleasePool.mm
        Mac/Private/MacFileSystem.mm
        Mac/Private/MacSystemContext.mm
        Mac/Private/MacWindow.mm
        Mac/Private/MacInput.mm
    )
    source_group("Mac\\Private" FILES ${PLATFORM_MAC_HEADERS})
    list(APPEND TOTAL_FILES ${PLATFORM_MAC_HEADERS})
endif()

if(HS_ARCH_X64)
    set(PLATFORM_X64_HEADERS
        x64/SSESimd.h
    )

    source_group("x64\\Public" FILES ${PLATFORM_X64_HEADERS})
    list(APPEND TOTAL_FILES ${PLATFORM_X64_HEADERS})

    set(PLATFORM_X64_SOURCES
        x64/Private/SSESimd.cpp
    )

    source_group("x64\\Private" FILES ${PLATFORM_X64_SOURCES})
    list(APPEND TOTAL_FILES ${PLATFORM_X64_SOURCES})
else()
    set(PLATFORM_ARM64_HEADERS
        Arm64/NeonSimd.h
    )

    source_group("Arm64\\Public" FILES ${PLATFORM_ARM64_HEADERS})
    list(APPEND TOTAL_FILES ${PLATFORM_ARM64_HEADERS})

    set(PLATFORM_ARM64_SOURCES
        Arm64/Private/NeonSimd.cpp
    )

    source_group("Arm64\\Private" FILES ${PLATFORM_ARM64_SOURCES})
    list(APPEND TOTAL_FILES ${PLATFORM_ARM64_SOURCES})
endif()

add_library(${TARGET_NAME}
    STATIC
    ${TOTAL_FILES}
)

add_dependencies(${TARGET_NAME} Core)

target_include_directories(${TARGET_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if(APPLE)
    target_link_libraries(${TARGET_NAME}
        PRIVATE
        Core
        "-framework Foundation -framework CoreFoundation -framework QuartzCore -framework AppKit -framework IOKit -framework Metal -framework MetalKit -framework GameController"
    )
else()
    target_link_libraries(${TARGET_NAME}
        PRIVATE
        Core
    )
endif()

target_compile_definitions(${TARGET_NAME}
    PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:MinSizeRel>:_RELEASE>
    $<$<CONFIG:Release>:_RELEASE>
    $<$<CONFIG:RelWithDebInfo>:_RELWITHDEBINFO>
    HS_PLATFORM
)

install(TARGETS ${TARGET_NAME}
    ARCHIVE DESTINATION "${INSTALL_FOLDER}/dev/lib"
    LIBRARY DESTINATION "${INSTALL_FOLDER}"
    RUNTIME DESTINATION "${INSTALL_FOLDER}"
)
