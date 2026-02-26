# ThirdParty libraries for HSMR prototyping framework
# These are header/source files that get compiled into the modules that use them

# ImPlot
set(IMPLOT_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ImPlot" CACHE PATH "ImPlot include directory" FORCE)
set(IMPLOT_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/ImPlot/implot.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImPlot/implot_items.cpp"
    CACHE STRING "ImPlot source files" FORCE)

# ImGuizmo
set(IMGUIZMO_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ImGuizmo" CACHE PATH "ImGuizmo include directory" FORCE)
set(IMGUIZMO_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/ImGuizmo/ImGuizmo.cpp"
    CACHE STRING "ImGuizmo source files" FORCE)
