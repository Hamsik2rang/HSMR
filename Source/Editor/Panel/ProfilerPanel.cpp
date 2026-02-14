//
//  ProfilerPanel.cpp
//  Editor
//
//  Dockable profiler panel implementation.
//

#include "Editor/Panel/ProfilerPanel.h"
#include "Editor/Core/EditorContext.h"

#include "Core/Profiler/ProfileDataCollector.h"
#include "Core/Profiler/Profiler.h"

#include "ImGui/imgui.h"

HS_NS_EDITOR_BEGIN

ProfilerPanel::ProfilerPanel(Window* window)
    : Panel(window)
{
}

ProfilerPanel::~ProfilerPanel()
{
}

bool ProfilerPanel::Setup()
{
    return true;
}

void ProfilerPanel::Cleanup()
{
}

void ProfilerPanel::Draw()
{
    auto& vis = EditorContext::Get().GetPanelVisibility();
    if (!vis.profiler)
    {
        return;
    }

    ImGui::Begin("Profiler", &vis.profiler);

    if (ImGui::BeginTabBar("ProfilerTabs"))
    {
        if (ImGui::BeginTabItem("CPU"))
        {
            _drawCPUTab();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("GPU"))
        {
            _drawGPUTab();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Memory"))
        {
            _drawMemoryTab();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::Separator();

    // Tracy status
#if defined(TRACY_ENABLE)
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Tracy: Enabled");
#else
    ImGui::TextDisabled("Tracy: Disabled");
#endif

    ImGui::End();
}

void ProfilerPanel::_drawCPUTab()
{
    const auto& zones = hs::ProfileDataCollector::Get().GetLastFrameZones();

    if (zones.empty())
    {
        ImGui::TextDisabled("No zone data collected");
        return;
    }

    // Calculate total frame time from root zones (depth == 0)
    float totalFrameMs = 0.0f;
    for (const auto& zone : zones)
    {
        if (zone.depth == 0)
        {
            totalFrameMs += zone.durationMs;
        }
    }
    if (totalFrameMs < 0.001f)
    {
        totalFrameMs = 1.0f;
    }

    ImGui::Text("Frame: %.2f ms", totalFrameMs);
    ImGui::Separator();

    // Render zone tree using depth to determine tree structure
    // Track which depth levels have open tree nodes
    int prevDepth = -1;
    int openNodes = 0;

    for (size_t i = 0; i < zones.size(); ++i)
    {
        const auto& zone = zones[i];

        // Close tree nodes if we went back up in depth
        while (prevDepth >= zone.depth && openNodes > 0)
        {
            ImGui::TreePop();
            openNodes--;
            prevDepth--;
        }

        // Check if this zone has children (next zone has greater depth)
        bool hasChildren = (i + 1 < zones.size() && zones[i + 1].depth > zone.depth);

        float fraction = zone.durationMs / totalFrameMs;
        float percent = fraction * 100.0f;

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth;
        if (!hasChildren)
        {
            flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        }
        flags |= ImGuiTreeNodeFlags_DefaultOpen;

        // Zone color indicator
        ImVec4 zoneColor = ImGui::ColorConvertU32ToFloat4(
            IM_COL32((zone.color >> 16) & 0xFF,
                     (zone.color >> 8) & 0xFF,
                     (zone.color) & 0xFF, 255));

        ImGui::PushStyleColor(ImGuiCol_Text, zoneColor);
        ImGui::Bullet();
        ImGui::PopStyleColor();
        ImGui::SameLine();

        bool nodeOpen = ImGui::TreeNodeEx(zone.name, flags);

        // Duration and percentage on the same line
        ImGui::SameLine(ImGui::GetWindowWidth() - 200);
        ImGui::Text("%.2f ms", zone.durationMs);
        ImGui::SameLine(ImGui::GetWindowWidth() - 120);

        // Draw mini bar
        _drawZoneBar(fraction, zone.color, 60.0f, ImGui::GetTextLineHeight());
        ImGui::SameLine();
        ImGui::Text("%4.1f%%", percent);

        if (hasChildren && nodeOpen)
        {
            openNodes++;
            prevDepth = zone.depth;
        }
        else if (!hasChildren)
        {
            prevDepth = zone.depth;
        }
    }

    // Close remaining open tree nodes
    while (openNodes > 0)
    {
        ImGui::TreePop();
        openNodes--;
    }
}

void ProfilerPanel::_drawGPUTab()
{
    ImGui::TextDisabled("GPU profiling coming soon.");
    ImGui::TextDisabled("(Phase 2)");
}

void ProfilerPanel::_drawMemoryTab()
{
    ImGui::TextDisabled("Memory profiling coming soon.");
    ImGui::TextDisabled("(Phase 3)");
}

void ProfilerPanel::_drawZoneBar(float fraction, uint32 color, float width, float height)
{
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();

    float barWidth = width * fraction;
    if (barWidth < 1.0f)
    {
        barWidth = 1.0f;
    }

    ImU32 bgColor = IM_COL32(40, 40, 40, 200);
    ImU32 barColor = IM_COL32((color >> 16) & 0xFF,
                               (color >> 8) & 0xFF,
                               (color) & 0xFF, 200);

    drawList->AddRectFilled(pos, ImVec2(pos.x + width, pos.y + height), bgColor);
    drawList->AddRectFilled(pos, ImVec2(pos.x + barWidth, pos.y + height), barColor);

    // Advance cursor
    ImGui::Dummy(ImVec2(width, height));
}

HS_NS_EDITOR_END
