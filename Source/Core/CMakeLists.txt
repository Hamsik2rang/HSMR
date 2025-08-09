set(TARGET_NAME Core)

set(TOTAL_FILES)

set(CORE_HEADERS
    Application.h
    Color.h
    EngineContext.h
    Event.h
    Exception.h
    Flag.h
    Hash.h
    Log.h
    String.h
    Swapchain.h
    Window.h
)

source_group("Public" FILES ${CORE_HEADERS})
list(APPEND TOTAL_FILES ${CORE_HEADERS})

set(CORE_SOURCES
    Private/Application.cpp
    Private/Color.cpp
    Private/EngineContext.cpp
    Private/Event.cpp
    Private/Exception.cpp
    Private/Flag.cpp
    Private/Hash.cpp
    Private/Log.cpp
    Private/String.cpp
    Private/Swapchain.cpp
    Private/Window.cpp
)

source_group("Private" FILES ${CORE_SOURCES})
list(APPEND TOTAL_FILES ${CORE_SOURCES})

set(CORE_MATH_HEADERS
    Math/Mat4f.h
    Math/Common.h
    Math/Vec2f.h
    Math/Vec3f.h
    Math/Vec4f.h
)

source_group("Math\\Public" FILES ${CORE_MATH_HEADERS})
list(APPEND TOTAL_FILES ${CORE_MATH_HEADERS})

set(CORE_MATH_SOURCES
    Math/Private/Mat4f.cpp
    Math/Private/Common.cpp
    Math/Private/Vec2f.cpp
    Math/Private/Vec3f.cpp
    Math/Private/Vec4f.cpp
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

add_library(${TARGET_NAME} STATIC ${TOTAL_FILES})

target_include_directories(${TARGET_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${TARGET_NAME} PRIVATE HAL)

target_compile_definitions(${TARGET_NAME}
    PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:MinSizeRel>:_RELEASE>
    $<$<CONFIG:Release>:_RELEASE>
    $<$<CONFIG:RelWithDebInfo>:_RELWITHDEBINFO>
    HS_CORE
    HS_API_EXPORT
)

install(TARGETS ${TARGET_NAME}
    ARCHIVE DESTINATION "${INSTALL_FOLDER}/dev/lib"
    LIBRARY DESTINATION "${INSTALL_FOLDER}"
    RUNTIME DESTINATION "${INSTALL_FOLDER}"
)