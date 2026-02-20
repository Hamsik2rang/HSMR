set(TARGET_NAME Platform)

set(TOTAL_FILES)

if(HSMR_USE_SDL)
    set(PLATFORM_SDL_HEADERS
        SDL/SDLFileDialog.h
        SDL/SDLFileSystem.h
    )
    source_group("SDL\\Public" FILES ${PLATFORM_SDL_HEADERS})
    list(APPEND TOTAL_FILES ${PLATFORM_SDL_HEADERS})

    set(PLATFORM_SDL_SOURCES
        SDL/Private/SDLFileDialog.cpp
        SDL/Private/SDLFileSystem.cpp
    )
    if(APPLE)
        list(APPEND PLATFORM_SDL_SOURCES SDL/Private/SDLFileDialog_Mac.mm)
    endif()
    source_group("SDL\\Private" FILES ${PLATFORM_SDL_SOURCES})
    list(APPEND TOTAL_FILES ${PLATFORM_SDL_SOURCES})
endif()

if(WIN32)
    set(PLATFORM_WIN_HEADERS

        # Win/WinFileSystem.h
        Win/WinSystemContext.h
        Win/WinInput.h
        Win/WinWindow.h
    )

    set(PLATFORM_WIN_SOURCES

        # Win/Private/WinFileSystem.cpp
        Win/Private/WinSystemContext.cpp
        Win/Private/WinInput.cpp
        Win/Private/WinWindow.cpp
    )

    if(NOT HSMR_USE_SDL)
        list(APPEND PLATFORM_WIN_HEADERS Win/WinFileDialog.h Win/Private/WinFileSystem.h)
        list(APPEND PLATFORM_WIN_SOURCES Win/Private/WinFileDialog.cpp Win/Private/WinFileSystem.cpp)
    endif()

    source_group("Win\\Public" FILES ${PLATFORM_WIN_HEADERS})
    list(APPEND TOTAL_FILES ${PLATFORM_WIN_HEADERS})

    source_group("Win\\Private" FILES ${PLATFORM_WIN_SOURCES})
    list(APPEND TOTAL_FILES ${PLATFORM_WIN_SOURCES})

else() # MAC
    set(PLATFORM_MAC_HEADERS
        Mac/AutoReleasePool.h

        # Mac/MacFileSystem.h
        Mac/MacSystemContext.h
        Mac/MacInput.h
        Mac/MacWindow.h
    )

    set(PLATFORM_MAC_SOURCES
        Mac/Private/AutoReleasePool.mm

        # Mac/Private/MacFileSystem.mm
        Mac/Private/MacSystemContext.mm
        Mac/Private/MacInput.mm
        Mac/Private/MacWindow.mm
    )

    if(NOT HSMR_USE_SDL)
        list(APPEND PLATFORM_MAC_HEADERS Mac/MacFileDialog.h Mac/MacFileSystem.h)
        list(APPEND PLATFORM_MAC_SOURCES Mac/Private/MacFileDialog.mm Mac/Private/MacFileSystem.mm)
    endif()

    source_group("Mac\\Public" FILES ${PLATFORM_MAC_HEADERS})
    list(APPEND TOTAL_FILES ${PLATFORM_MAC_HEADERS})

    source_group("Mac\\Private" FILES ${PLATFORM_MAC_SOURCES})
    list(APPEND TOTAL_FILES ${PLATFORM_MAC_SOURCES})
endif()

# Note: SDL-specific file (SDLPlatform.cpp) has been removed.
# SDL support is now integrated into MacWindow.mm and WinWindow.cpp via #ifdef __SDL__
if(HS_ARCH STREQUAL "x64")
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

# On Windows, Platform is WHOLEARCHIVE'd into Core.dll, so we don't create
# a dependency on Core here (would cause circular CMake dependency).
# Core headers are available via the global include_directories(Source).
if(NOT WIN32)
    add_dependencies(${TARGET_NAME} Core)
endif()

target_include_directories(${TARGET_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if(HSMR_USE_SDL)
    # SDL3 include directory
    target_include_directories(${TARGET_NAME}
        PRIVATE
        ${HS_SDL3_INCLUDE_DIR}
    )

    if(WIN32)
        # On Windows, Platform is merged into Core.dll via WHOLEARCHIVE.
        # SDL3 dependency is handled in Core/CMakeLists.txt.
        target_link_libraries(${TARGET_NAME}
            PRIVATE
            ${HS_SDL3_LIB}
        )
    elseif(APPLE)
        # Add framework search path for SDL3
        target_compile_options(${TARGET_NAME}
            PRIVATE
            -F${HS_DEPS_LIB_DIR}
        )

        target_link_options(${TARGET_NAME}
            PRIVATE
            -F${HS_DEPS_LIB_DIR}
        )

        target_link_libraries(${TARGET_NAME}
            PRIVATE
            Core
            "-framework Foundation"
            "-framework CoreFoundation"
            "-framework QuartzCore"
            "-framework AppKit"
            "-framework IOKit"
            "-framework Metal"
            "-framework MetalKit"
            "-framework GameController"
            "-framework SDL3"
        )
    endif()
else()
    if(APPLE)
        target_link_libraries(${TARGET_NAME}
            PRIVATE
            Core
            "-framework Foundation -framework CoreFoundation -framework QuartzCore -framework AppKit -framework IOKit -framework Metal -framework MetalKit -framework GameController"
        )
    endif()
    # On Windows without SDL, no additional link deps needed for Platform.
    # Platform will be WHOLEARCHIVE'd into Core.dll.
endif()

target_compile_definitions(${TARGET_NAME}
    PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:MinSizeRel>:_RELEASE>
    $<$<CONFIG:Release>:_RELEASE>
    $<$<CONFIG:RelWithDebInfo>:_RELWITHDEBINFO>
    HS_PLATFORM
    HS_API_EXPORT
    # On Windows, Platform is WHOLEARCHIVE'd into Core.dll, so Platform functions
    # that use HS_CORE_API need dllexport to be visible from Core.dll.
    $<$<PLATFORM_ID:Windows>:HS_CORE_API_EXPORT>
)

install(TARGETS ${TARGET_NAME}
    ARCHIVE DESTINATION "${INSTALL_FOLDER}/dev/lib"
    LIBRARY DESTINATION "${INSTALL_FOLDER}"
    RUNTIME DESTINATION "${INSTALL_FOLDER}"
)

hs_auto_install(${TARGET_NAME})
