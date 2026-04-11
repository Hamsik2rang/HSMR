# Editor Panel Refactoring: From Ad-Hoc Windows to a Reusable Editor UI Layer

## Overview

This article documents the editor panel refactoring that was completed on the
`feature/panel-refactoring` branch.

It is written as a practical engineering note rather than a changelog. The goal
is to help a new developer or agent understand:

- what the old problems were
- what architectural decisions were made
- which shared helpers now exist
- how those helpers are expected to be used
- what tradeoffs were accepted to keep complexity low

If you only need the current rules, see:
- [EditorPanelArchitecture.md](/Users/yongsikim/Desktop/Dev/HSMR/Docs/EditorPanelArchitecture.md)

This article is the “why” and “how we got here”.

## The Problem We Were Solving

The editor originally grew in a very natural way:

- each panel was just an ImGui window
- panel-specific layout lived inside that panel
- wiring and ownership gradually accumulated in `EditorWindow`

That works for a small editor, but after enough features land, the cost starts
to show up in three places.

### 1. `EditorWindow` became the wiring bottleneck

Panel creation, visibility binding, update order, and cross-panel ownership were
all mixed together in one orchestration layer. That meant small panel changes
often required touching top-level window code.

### 2. Similar UI drifted apart

The same conceptual controls were implemented with different low-level ImGui
patterns:

- some property rows used `Columns()`
- others used `SameLine()`
- others used ad-hoc width math

That kind of drift is subtle, but it compounds quickly. Two `vec3` controls can
end up looking like unrelated widgets even though they represent the same type
of data.

### 3. New work became slower than it needed to be

Adding a panel, extending a panel, or restyling a panel often meant writing
more “panel-local” UI glue than actual editor behavior. Over time, the editor
risked becoming harder to evolve than the runtime systems it was inspecting.

## Refactoring Goals

The refactor aimed to solve those problems without turning the editor into a
large framework.

The goals were:

- keep `Panel` small
- keep `EditorWindow` as an orchestrator, not a god-object
- centralize repeated ImGui structure in narrow helpers
- make the UI more visually consistent
- reduce the amount of custom code needed for the next panel or dialog

Just as important, we wanted to **avoid over-abstraction**.

The rule of thumb throughout the refactor was:

- if a pattern is repeated and clearly reusable, extract it
- if a pattern is highly local or domain-specific, keep it in the panel

That is why the resulting structure is made of a few focused helpers rather
than one giant “editor UI utilities” layer.

## Architectural Outcome

The refactor ended up organizing the panel system into three levels.

### Level 1: Panel lifecycle and ownership

This is the structural layer:

- [Panel.h](/Users/yongsikim/Desktop/Dev/HSMR/Source/Editor/Panel/Panel.h)
- [EditorWindow.h](/Users/yongsikim/Desktop/Dev/HSMR/Source/Editor/Core/EditorWindow.h)
- [EditorWindow.cpp](/Users/yongsikim/Desktop/Dev/HSMR/Source/Editor/Core/EditorWindow.cpp)

`Panel` owns the minimum common contract:

- `Setup()`
- `Cleanup()`
- `Update()`
- `Draw()`
- stable `panelId`
- a shared visibility binding

`EditorWindow` owns:

- panel creation
- registration order
- lifecycle dispatch
- visibility binding

The important boundary is this:

- `EditorWindow` should know **that a panel exists**
- a panel should know **how to draw itself**

The top-level window should not have to micromanage each panel’s internal
layout.

### Level 2: Shared UI primitives

This is the visual consistency layer:

- [EditorPanelFrame.h](/Users/yongsikim/Desktop/Dev/HSMR/Source/Editor/Panel/EditorPanelFrame.h)
- [EditorFormLayout.h](/Users/yongsikim/Desktop/Dev/HSMR/Source/Editor/GUI/EditorFormLayout.h)
- [EditorListWidgets.h](/Users/yongsikim/Desktop/Dev/HSMR/Source/Editor/GUI/EditorListWidgets.h)
- [EditorTreeWidgets.h](/Users/yongsikim/Desktop/Dev/HSMR/Source/Editor/GUI/EditorTreeWidgets.h)
- [EditorDialogFrame.h](/Users/yongsikim/Desktop/Dev/HSMR/Source/Editor/GUI/EditorDialogFrame.h)
- [EditorFeedbackWidgets.h](/Users/yongsikim/Desktop/Dev/HSMR/Source/Editor/GUI/EditorFeedbackWidgets.h)

Each helper has a narrow purpose:

- `EditorPanelFrame`: panel window/frame structure
- `EditorFormLayout`: label/value property forms
- `EditorListWidgets`: selectable rows and card/list surface language
- `EditorTreeWidgets`: tree node conventions and tree-related feedback
- `EditorDialogFrame`: modal placement and footer layout
- `EditorFeedbackWidgets`: empty-state and low-emphasis text

### Level 3: Panel-specific editor behavior

This remains inside the panels themselves:

- what data is shown
- how the panel reacts to editor state
- what interactions are meaningful for that panel

For example:

- `InspectorPanel` still owns material editing logic
- `ResourcePanel` still owns asset browsing behavior
- `ScenePanel` still owns viewport interaction and picking

The refactor did **not** try to erase panel identity. It only removed repeated
layout scaffolding.

## Before and After

### Before: ad-hoc property layout

A common old pattern was a mix of manual columns, `SameLine()`, and
panel-specific width logic.

```cpp
ImGui::Columns(2, nullptr, false);
ImGui::Text("Near");
ImGui::NextColumn();
ImGui::SetNextItemWidth(-1);
ImGui::DragFloat("##Near", &camera.nearPlane, 0.01f);
ImGui::Columns(1);
```

This is workable in isolation, but if every panel does it slightly differently,
alignment and spacing drift over time.

### After: shared form rows

The same kind of UI now uses the shared form layout:

```cpp
if (EditorFormLayout::Begin("CameraForm"))
{
    EditorFormLayout::DragFloatRow("Near", &camera.nearPlane, 0.01f);
    EditorFormLayout::DragFloatRow("Far", &camera.farPlane, 1.0f);
    EditorFormLayout::CheckboxRow("Primary", &camera.isPrimary);
    EditorFormLayout::End();
}
```

This is shorter, easier to read, and visually consistent with every other form
that uses the same helper.

## The Key Structural Shift: `EditorWindow` as Orchestrator

One of the most important changes was reducing the amount of panel-specific
wiring that lived in `EditorWindow`.

The old direction was effectively:

- create panel
- stash it in a specific member
- manually remember to update it
- manually remember to draw it
- manually keep visibility in sync

The refactor moved toward a registration model:

```cpp
void EditorWindow::registerPanel(Panel* panel, bool* visibilityBinding, Panel* parent)
{
    if (!panel)
    {
        return;
    }

    panel->BindVisibility(visibilityBinding);
    if (parent)
    {
        parent->InsertPanel(panel);
    }

    _registeredPanels.push_back(panel);
}
```

That looks small, but the design payoff is large:

- panel ownership is easier to reason about
- update/draw order is centralized
- visibility binding becomes part of setup, not a scattered concern
- adding a panel becomes less invasive

## Why Stable `panelId` Matters

Another lesson from the refactor is that editor windows need stable identities.

Two panels may render similar controls, but they must still be distinct at the
window system level. This matters for:

- docking layout stability
- saved UI state
- debugging
- future automation or tooling

This is why `SimpleInspectorPanel` and `InspectorPanel` must not silently share
the same internal id.

## Shared Helper Design Principles

We tried to keep every helper small enough that a new contributor can answer
“what is this helper for?” in one sentence.

### `EditorPanelFrame`

This helper standardizes the outer shell of a panel.

```cpp
EditorPanelWindowOptions panelOptions{};
panelOptions.pOpen = GetVisibilityBinding();
panelOptions.useMenuBar = true;

EditorPanelFrame::BeginStandardPanel("Game", panelOptions);
drawMenuBar(scene);

EditorPanelContentOptions contentOptions{};
contentOptions.id = "GameViewport";
contentOptions.padding = ImVec2(0.0f, 0.0f);
EditorPanelFrame::BeginPanelContent(contentOptions);

// panel body

EditorPanelFrame::EndPanelContent();
EditorPanelFrame::EndStandardPanel();
```

This avoids every panel reinventing:

- `Begin`/`End`
- child content areas
- menu bar enablement
- common panel flags

### `EditorFormLayout`

This helper is the main guardrail against property-row drift.

```cpp
if (EditorFormLayout::Begin("LightForm", 120.0f))
{
    EditorFormLayout::ColorEdit3Row("Color", glm::value_ptr(light.color));
    EditorFormLayout::DragFloatRow("Intensity", &light.intensity, 0.05f);
    EditorFormLayout::CheckboxRow("Cast Shadow", &light.castShadow);
    EditorFormLayout::End();
}
```

Once this existed, it became much easier to align:

- `Camera`
- `Light`
- `MeshRenderer`
- material parameter editors
- launcher dialogs

under one visual rule.

### `EditorListWidgets`

This helper exists because list/card panels were starting to invent their own
hover, active, text, and warning hierarchies.

The helper gives us a shared surface language for:

- `ResourcePanel` rows
- `ProjectLauncherWindow` recent project cards

This is one of the main ways the refactor prevents theme drift.

### `EditorTreeWidgets`

This helper captures the tree-specific details that are easy to get subtly
wrong:

- consistent node flags
- icon+label construction
- distinguishing selection clicks from arrow-toggle clicks
- consistent empty-state usage near tree UIs

That kept `HierarchyPanel` and the folder tree in `ResourcePanel` from evolving
two different interaction styles.

### `EditorDialogFrame`

Dialogs are another place where drift tends to show up early.

Without a helper, every modal ends up re-implementing:

- `OpenPopup`
- centering logic
- initial size
- footer button alignment

The helper keeps those concerns out of panel code.

```cpp
if (EditorDialogFrame::BeginCenteredModal("New Project", &_showNewProjectDialog, ImVec2(600, 300)))
{
    // dialog body

    EditorDialogFrame::BeginFooterButtons(2, 120.0f, 10.0f);
    if (ImGui::Button("Create", ImVec2(120.0f, 35.0f)))
    {
        // confirm
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(120.0f, 35.0f)))
    {
        _showNewProjectDialog = false;
    }

    EditorDialogFrame::EndModal();
}
```

### `EditorFeedbackWidgets`

This is intentionally tiny, but it matters.

Low-emphasis UI text appears everywhere:

- “No entity selected”
- “No recent projects”
- “GPU profiling coming soon.”
- version labels
- status placeholders

That kind of copy tends to sprawl and become inconsistent. A tiny shared helper
is enough to stop that drift without introducing heavy abstraction.

## What We Chose Not To Abstract

A good refactor is defined as much by what it leaves alone as by what it
extracts.

We intentionally did **not** try to create:

- one giant generic panel base class that understands every panel type
- one mega-widget library for every ImGui control
- panel-independent business logic that hides all editor behavior

Examples we kept local on purpose:

- `ScenePanel` viewport interaction
- profiler zone tree rendering details
- material inspector behavior
- resource browsing rules

These areas are still panel-specific enough that abstracting them further would
likely hurt readability.

## Practical Impact on Existing Panels

### `InspectorPanel`

The inspector benefited the most from shared form layout because it had the
largest concentration of property editors.

This is where the refactor directly reduced future inconsistency risk:

- `vec3` controls now follow shared spacing/alignment rules
- material parameter rows align with other property rows
- texture slots and reflected parameter rows sit in the same layout system

### `SimpleInspectorPanel`

This panel was important because it proved the helpers were reusable outside the
main inspector. It also helped enforce the rule that internal panel ids must be
stable and distinct.

### `ResourcePanel`

The resource panel now shares:

- list row surface rules
- tree behavior rules
- current-folder based creation semantics

This matters because resource browsers are otherwise very prone to “every row
looks slightly different” problems.

### `ProjectLauncherWindow`

Even though this is not a docked panel, it was folded into the same UI language.

That was intentional.

The launcher now shares:

- list/card color hierarchy
- dialog form structure
- feedback text treatment

This widened the benefit of the refactor beyond the main editor shell.

## Documentation Strategy

Two different docs now exist on purpose.

### 1. Architecture rules

- [EditorPanelArchitecture.md](/Users/yongsikim/Desktop/Dev/HSMR/Docs/EditorPanelArchitecture.md)

This is the concise “how to work in this codebase” document.

### 2. This article

- [EditorPanelRefactoringArticle.md](/Users/yongsikim/Desktop/Dev/HSMR/Docs/EditorPanelRefactoringArticle.md)

This is the deeper narrative document explaining design intent and tradeoffs.

The split is deliberate:

- rules docs should be short and searchable
- articles should explain reasoning without forcing every reader through the full story

## Guidance for Future Contributors

If you are adding or changing a panel, use this checklist.

1. Ask whether the panel’s outer structure should use `EditorPanelFrame`.
2. Ask whether the body is fundamentally:
   - a form
   - a list/card
   - a tree
   - a modal dialog
   - an empty/secondary feedback area
3. Reuse the corresponding helper first.
4. Only add a new helper if the pattern is already appearing in multiple places.
5. Keep helper scope narrow.

If you skip those steps, UI drift will come back.

## What “Done” Means for This Refactor

This refactor does **not** mean the editor UI is feature-complete.

It means the panel system is now in a healthier state:

- ownership is clearer
- visibility is more uniform
- repeated layout code is reduced
- consistency has a real enforcement path
- new panels have a better starting point

That is the kind of refactor that keeps paying dividends after the original
feature work is finished.

## Appendix: Minimal Patterns to Copy

### Standard panel shell

```cpp
if (!IsVisible())
{
    return;
}

EditorPanelWindowOptions options{};
options.pOpen = GetVisibilityBinding();

EditorPanelFrame::BeginStandardPanel("Example", options);
// draw panel body
EditorPanelFrame::EndStandardPanel();
```

### Property form

```cpp
if (EditorFormLayout::Begin("ExampleForm"))
{
    EditorFormLayout::DragFloatRow("Value", &_value, 0.01f);
    EditorFormLayout::CheckboxRow("Enabled", &_enabled);
    EditorFormLayout::End();
}
```

### Empty state

```cpp
EditorFeedbackWidgets::EmptyState(
    "No recent projects",
    "Create a new project or open an existing one.");
```

### Modal dialog

```cpp
if (EditorDialogFrame::BeginCenteredModal("Rename Asset", &_showRenameDialog, ImVec2(420, 0)))
{
    if (EditorFormLayout::Begin("RenameForm"))
    {
        EditorFormLayout::InputTextRow("Name", _renameBuffer, sizeof(_renameBuffer));
        EditorFormLayout::End();
    }

    EditorDialogFrame::BeginFooterButtons(2);
    if (ImGui::Button("OK", ImVec2(120.0f, 35.0f)))
    {
        // apply
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(120.0f, 35.0f)))
    {
        _showRenameDialog = false;
    }

    EditorDialogFrame::EndModal();
}
```

