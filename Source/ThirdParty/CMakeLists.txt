# ThirdParty libraries for HSMR prototyping framework
# These are header/source files that get compiled into the modules that use them

# Check for ImPlot
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ImPlot/implot.h")
    set(IMPLOT_FOUND TRUE CACHE BOOL "ImPlot found" FORCE)
    set(IMPLOT_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ImPlot" CACHE PATH "ImPlot include directory" FORCE)
    set(IMPLOT_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/ImPlot/implot.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ImPlot/implot_items.cpp"
        CACHE STRING "ImPlot source files" FORCE)
    message(STATUS "ImPlot found at ${IMPLOT_INCLUDE_DIR}")
else()
    set(IMPLOT_FOUND FALSE CACHE BOOL "ImPlot not found" FORCE)
    message(STATUS "ImPlot not found")
endif()

# Check for ImGuizmo
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ImGuizmo/ImGuizmo.h")
    set(IMGUIZMO_FOUND TRUE CACHE BOOL "ImGuizmo found" FORCE)
    set(IMGUIZMO_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ImGuizmo" CACHE PATH "ImGuizmo include directory" FORCE)
    set(IMGUIZMO_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/ImGuizmo/ImGuizmo.cpp"
        CACHE STRING "ImGuizmo source files" FORCE)
    message(STATUS "ImGuizmo found at ${IMGUIZMO_INCLUDE_DIR}")
else()
    set(IMGUIZMO_FOUND FALSE CACHE BOOL "ImGuizmo not found" FORCE)
    message(STATUS "ImGuizmo not found")
endif()
