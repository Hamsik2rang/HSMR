set(TARGET_NAME Core)

set(TOTAL_FILES)

set(CORE_HEADERS
    Color.h
    Event.h
    Exception.h
    Flag.h
    Hash.h
    Log.h
    TypeId.h
    Object.h
    SystemContext.h
)

source_group("Public" FILES ${CORE_HEADERS})
list(APPEND TOTAL_FILES ${CORE_HEADERS})

set(CORE_SOURCES
    Private/Color.cpp
    Private/Event.cpp
    Private/Exception.cpp
    Private/Flag.cpp
    Private/Hash.cpp
    Private/Log.cpp
    Private/Object.cpp
    Private/SystemContext.cpp
)

source_group("Private" FILES ${CORE_SOURCES})
list(APPEND TOTAL_FILES ${CORE_SOURCES})

set(CORE_NATIVE_HEADERS
    Native/NativeWindow.h
    Native/NativeEvent.h
)
source_group("Native\\Public" FILES ${CORE_NATIVE_HEADERS})
list(APPEND TOTAL_FILES ${CORE_NATIVE_HEADERS})

set(CORE_NATIVE_SOURCES
    Native/Private/NativeWindow.cpp
    Native/Private/NativeEvent.cpp
)
source_group("Native\\Private" FILES ${CORE_NATIVE_SOURCES})
list(APPEND TOTAL_FILES ${CORE_NATIVE_SOURCES})

set(CORE_HAL_HEADERS
    HAL/CommandLine.h
    HAL/FileDialog.h
    HAL/FileSystem.h
    HAL/Input.h
    HAL/Timer.h
)

source_group("HAL\\Public" FILES ${CORE_HAL_HEADERS})
list(APPEND TOTAL_FILES ${CORE_HAL_HEADERS})

set(CORE_HAL_SOURCES
    HAL/Private/CommandLine.cpp
    HAL/Private/FileSystem.cpp
    HAL/Private/Input.cpp
    HAL/Private/Timer.cpp
)

source_group("HAL\\Private" FILES ${CORE_HAL_SOURCES})
list(APPEND TOTAL_FILES ${CORE_HAL_SOURCES})

set(CORE_MATH_HEADERS
    Math/Common.h
)

source_group("Math\\Public" FILES ${CORE_MATH_HEADERS})
list(APPEND TOTAL_FILES ${CORE_MATH_HEADERS})

set(CORE_MATH_SOURCES
    Math/Private/Common.cpp
)

source_group("Math\\Private" FILES ${CORE_MATH_SOURCES})
list(APPEND TOTAL_FILES ${CORE_MATH_SOURCES})

set(CORE_UTILITY_HEADERS
    Utility/StringUtility.h
)

source_group("Utility\\Public" FILES ${CORE_UTILITY_HEADERS})
list(APPEND TOTAL_FILES ${CORE_UTILITY_HEADERS})

set(CORE_UTILITY_SOURCES
    Utility/Private/StringUtility.cpp
)

source_group("Utility\\Private" FILES ${CORE_UTILITY_SOURCES})
list(APPEND TOTAL_FILES ${CORE_UTILITY_SOURCES})

# Memory management
set(CORE_MEMORY_HEADERS
    Memory/MemoryPool.h
)

source_group("Memory\\Public" FILES ${CORE_MEMORY_HEADERS})
list(APPEND TOTAL_FILES ${CORE_MEMORY_HEADERS})

set(CORE_MEMORY_SOURCES
    Memory/Private/MemoryPool.cpp
)

source_group("Memory\\Private" FILES ${CORE_MEMORY_SOURCES})
list(APPEND TOTAL_FILES ${CORE_MEMORY_SOURCES})

set(CORE_PROFILER_HEADERS
    Profiler/Profiler.h
    Profiler/ProfilerMacros.h
    Profiler/ProfileDataCollector.h
    Profiler/GPUQuery.h
)

source_group("Profiler\\Public" FILES ${CORE_PROFILER_HEADERS})
list(APPEND TOTAL_FILES ${CORE_PROFILER_HEADERS})

set(CORE_PROFILER_SOURCES
    Profiler/Private/Profiler.cpp
    Profiler/Private/ProfileDataCollector.cpp
    Profiler/Private/GPUQuery.cpp
)
source_group("Profiler\\Private" FILES ${CORE_PROFILER_SOURCES})
list(APPEND TOTAL_FILES ${CORE_PROFILER_SOURCES})

add_library(${TARGET_NAME} SHARED ${TOTAL_FILES})

if(APPLE)
    list(APPEND CORE_FRAMEWORKS
        "-framework Foundation"
        "-framework AppKit"
        "-framework IOKit"
    )

    # Add SDL3 framework support
    if(HSMR_USE_SDL)
        target_compile_options(${TARGET_NAME}
            PRIVATE
            -F${HS_DEPS_LIB_DIR}
        )

        target_link_options(${TARGET_NAME}
            PRIVATE
            -F${HS_DEPS_LIB_DIR}
        )

        list(APPEND CORE_FRAMEWORKS "-framework SDL3")
    endif()

    target_link_libraries(${TARGET_NAME}
        PRIVATE
        ${CORE_FRAMEWORKS}
    )

    # Platform implements Core's *Internal() functions (CreateNativeWindowInternal, etc.)
    # We defer symbol resolution to runtime to avoid Core <-> Platform circular dependency.
    target_link_options(${TARGET_NAME}
        PRIVATE
        "-undefined" "dynamic_lookup"
    )
elseif(WIN32)
    # On Windows, DLLs cannot defer symbol resolution like macOS (-undefined dynamic_lookup).
    # Platform implements Core's initializePlatform(), finalizePlatform(), and *Internal()
    # functions. We merge Platform (STATIC) into Core.dll via WHOLEARCHIVE so all symbols
    # are resolved within the same link step.
    target_link_libraries(${TARGET_NAME}
        PRIVATE
        Platform
    )
    target_link_options(${TARGET_NAME}
        PRIVATE
        "/WHOLEARCHIVE:Platform"
    )

    # Pull in Platform's dependencies
    if(HSMR_USE_SDL)
        target_link_libraries(${TARGET_NAME} PRIVATE ${HS_SDL3_LIB})
    endif()
endif()

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
    HS_CORE
    HS_CORE_API_EXPORT
)

# Link Tracy if enabled
if(HSMR_ENABLE_TRACY)
    target_link_libraries(${TARGET_NAME} PUBLIC TracyClient)
endif()

install(TARGETS ${TARGET_NAME}
    ARCHIVE DESTINATION "${INSTALL_FOLDER}/dev/lib"
    LIBRARY DESTINATION "${INSTALL_FOLDER}"
    RUNTIME DESTINATION "${INSTALL_FOLDER}"
)

hs_auto_install(${TARGET_NAME})