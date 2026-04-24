# Editor Panel Architecture

## Purpose

This document describes the current editor panel architecture, the intent behind the recent panel refactoring, and the expected usage patterns for future development.

It is written for:
- new contributors onboarding into the editor code
- future agents continuing editor work
- reviewers who need to understand why panel-related changes are structured the way they are

This document is intentionally practical. It focuses on how the system is expected to be used, not just how it currently happens to work.

## Why This Refactor Exists

The editor started as a set of independent ImGui windows with panel-specific wiring living mostly in `EditorWindow`. That approach worked while the editor was small, but it created three recurring problems as more functionality was added:

1. `EditorWindow` became the place where panel creation, setup, update ordering, visibility, and cross-panel wiring were all mixed together.
2. Panels began to diverge in UI structure. Similar controls were rendered with different spacing, label placement, and interaction patterns.
3. Adding a new panel or changing an existing one increasingly required touching unrelated wiring code, which made iteration slower and more error-prone.

The refactor is meant to solve those issues by making the panel layer more declarative:

- `EditorWindow` owns and registers panels, but does not micromanage panel internals.
- `Panel` provides the shared lifecycle and visibility model.
- shared UI helpers define consistent visual and interaction rules.
- individual panels focus on editor behavior and data, not on re-inventing layout code.

The goal is not "more abstraction for its own sake". The goal is:

- lower coupling
- fewer duplicated ImGui patterns
- stable UI conventions
- easier onboarding for humans and agents

## Core Structure

### `Panel`

File:
- [Panel.h](/Users/yongsikim/Desktop/Dev/HSMR/Source/Editor/Panel/Panel.h)

`Panel` is the base type for editor panels. It owns the minimum common contract:

- lifecycle
  - `Setup()`
  - `Cleanup()`
  - `Update()`
  - `Draw()`
- identity
  - `panelId`
- visibility binding
  - `BindVisibility(bool*)`
  - `GetVisibilityBinding()`
  - `IsVisible()`
- parent/child relationship helpers
  - `InsertPanel()`
  - `RemovePanel()`

Important design rule:

- `Panel` should stay small.
- It is not a god-object for editor state.
- Shared editor state belongs in `EditorContext`.
- Shared rendering/layout behavior belongs in dedicated UI helpers.

### `EditorWindow`

File:
- [EditorWindow.h](/Users/yongsikim/Desktop/Dev/HSMR/Source/Editor/Core/EditorWindow.h)
- [EditorWindow.cpp](/Users/yongsikim/Desktop/Dev/HSMR/Source/Editor/Core/EditorWindow.cpp)

`EditorWindow` is the orchestrator. Its job is to:

- create panel instances
- register them in a central order
- bind panel visibility to shared editor state
- run lifecycle methods
- update and draw panels in a consistent way

Key internal helpers:

- `registerPanel(...)`
- `cleanupPanels()`
- `setupPanels()`

Important design rule:

- if a new panel requires substantial custom wiring in `EditorWindow`, treat that as a smell.
- prefer binding shared state through `EditorContext` or shared helpers instead of adding more one-off cross-panel code.

## Shared UI Layers

The panel refactor is not only about object ownership. It is also about **UI consistency**.

There are now six shared helper layers that should be reused instead of hand-writing panel-local ImGui structure every time.

### 1. `EditorPanelFrame`

File:
- [EditorPanelFrame.h](/Users/yongsikim/Desktop/Dev/HSMR/Source/Editor/Panel/EditorPanelFrame.h)

Use this for:
- standard editor windows
- panel menu bars
- panel content child regions
- overlay-style auxiliary panels

This helper defines panel-level structure:
- top-level `Begin` / `End`
- optional menu bar
- optional child content area

Use it when:
- creating or updating any editor panel window

Do not:
- call raw `ImGui::Begin()` / `ImGui::End()` for a standard editor panel unless there is a specific reason not to

### 2. `EditorFormLayout`

File:
- [EditorFormLayout.h](/Users/yongsikim/Desktop/Dev/HSMR/Source/Editor/GUI/EditorFormLayout.h)

Use this for:
- inspector-style forms
- labeled value editing
- dialog forms

This helper defines the common two-column pattern:
- left: label
- right: editor/control

It is the preferred layout for:
- `InspectorPanel`
- modal forms such as project/material creation dialogs
- any property editor where label/value consistency matters

Use it when:
- the UI is fundamentally “a list of named properties”

Do not:
- mix ad-hoc `Columns()`, `SameLine()`, and arbitrary item widths if the control is conceptually a form field

### 3. `EditorListWidgets`

File:
- [EditorListWidgets.h](/Users/yongsikim/Desktop/Dev/HSMR/Source/Editor/GUI/EditorListWidgets.h)

Use this for:
- selectable rows
- list/card surface colors
- primary/secondary text color hierarchy
- warning-tinted list/card backgrounds

This helper defines the shared visual language for:
- `ResourcePanel` list rows
- `ProjectLauncherWindow` recent project cards

Use it when:
- a panel presents rows or cards that are selectable, hoverable, or theme-aware

### 4. `EditorTreeWidgets`

File:
- [EditorTreeWidgets.h](/Users/yongsikim/Desktop/Dev/HSMR/Source/Editor/GUI/EditorTreeWidgets.h)

Use this for:
- tree row label construction
- tree node default flags
- selection-click behavior
- panel-local search bars
- empty state text

This helper aligns tree behavior between:
- `HierarchyPanel`
- `ResourcePanel` folder tree

Use it when:
- a panel renders nested data with tree nodes

### 5. `EditorDialogFrame`

File:
- [EditorDialogFrame.h](/Users/yongsikim/Desktop/Dev/HSMR/Source/Editor/GUI/EditorDialogFrame.h)

Use this for:
- centered modal dialogs
- shared popup sizing/placement
- standard footer button alignment

Use it when:
- a panel or launcher window needs a modal dialog and would otherwise repeat popup centering and footer math

### 6. `EditorFeedbackWidgets`

File:
- [EditorFeedbackWidgets.h](/Users/yongsikim/Desktop/Dev/HSMR/Source/Editor/GUI/EditorFeedbackWidgets.h)

Use this for:
- empty-state text
- secondary/supporting text

Use it when:
- a panel needs a low-emphasis status line, placeholder, or “no data” message

## Current Expected Panel Authoring Style

When adding a new panel, prefer this order of construction:

1. Inherit from `Panel`.
2. Give the panel a stable `panelId`.
3. Let `EditorWindow::setupPanels()` create and register it.
4. Use `EditorPanelFrame` for the top-level window and content structure.
5. Use one of the specialized helpers for body UI:
   - `EditorFormLayout` for forms
   - `EditorListWidgets` for rows/cards
   - `EditorTreeWidgets` for trees
   - `EditorDialogFrame` for modal dialogs
   - `EditorFeedbackWidgets` for empty/secondary text
6. Store shared state in `EditorContext`, not in ad-hoc panel-to-panel references unless the dependency is truly local and explicit.

## Example: Minimal Standard Panel

```cpp
void ExamplePanel::Draw()
{
    if (!IsVisible())
    {
        return;
    }

    EditorPanelWindowOptions options{};
    options.pOpen = GetVisibilityBinding();

    EditorPanelFrame::BeginStandardPanel("Example", options);

    if (EditorFormLayout::Begin("ExampleForm"))
    {
        EditorFormLayout::DragFloatRow("Speed", &_speed, 0.01f);
        EditorFormLayout::CheckboxRow("Enabled", &_enabled);
        EditorFormLayout::End();
    }

    EditorPanelFrame::EndStandardPanel();
}
```

That is the preferred baseline shape.

## Visibility Rules

Panel visibility should come from a shared source, not panel-local booleans.

Use:
- `Panel::BindVisibility(...)`
- `Panel::GetVisibilityBinding()`
- `Panel::IsVisible()`

Do not:
- invent new per-panel visibility booleans if the panel is part of the normal editor docking/menu system

Important implication:

- menu items, panel instances, and layout restoration should all refer to the same visibility state

## Identity Rules

Every panel should have a stable identity.

Use:
- constructor-time `panelId`

Why:
- stable docking/layout behavior
- easier debugging
- clearer code search
- less chance of two windows accidentally sharing the same internal identity

Panels with different purposes should not share the same id, even if one inherits from another.

Example:
- `Inspector`
- `Control Panel`

These must remain distinct.

## `EditorContext` vs Panel State

Use `EditorContext` for:
- active scene
- selected entity
- selected asset path
- current asset folder
- shared panel visibility state

Keep state inside a panel only when it is:
- purely local UI state
- not meaningful to other systems
- not part of editor-wide behavior

Examples of valid panel-local state:
- search text buffer
- rename buffer
- split width
- temporary popup selection state

Examples of state that should live in shared context:
- selected entity
- selected asset
- active scene
- current folder used across panels

## Refactor Goals for Future Work

When continuing panel cleanup, optimize for:

### 1. Lower code duplication

If two panels do the same visual thing, prefer extracting a helper rather than repeating raw ImGui code.

### 2. Lower behavioral drift

Similar controls should not slowly become visually inconsistent.

Examples of drift this refactor is specifically trying to prevent:
- one `vec3` control having extra right padding while another does not
- one list using hover colors from style while another hardcodes them
- one tree toggling selection on arrow click while another does not
- one popup form using label-left/value-right while another stacks labels above fields

### 3. Keep helpers narrow

Avoid giant “do everything” editor UI utilities.

Prefer small helpers with a clear domain:
- frame
- form
- list
- tree
- dialog
- feedback text

This keeps complexity lower and makes it easier for future contributors to know where to add things.

## What Not To Do

Avoid these patterns unless there is a strong reason:

- adding new panel-specific layout code when an existing helper already fits
- storing editor-global state inside a panel instance
- coupling panel behavior through raw pointer chains when `EditorContext` or shared services are sufficient
- using raw `ImGui::Columns()` for inspector/property rows
- hardcoding theme colors in panel-local code when shared widget helpers already define a surface/text hierarchy

## Recommended Workflow for New Contributors

If you are modifying editor panels:

1. Identify the panel type:
   - form
   - list/card
   - tree
   - overlay
2. Reuse the matching helper first.
3. Only add a new helper if the pattern is truly reusable across multiple panels.
4. If a new helper is added, document its intended scope here or in an adjacent comment.
5. Validate with an `Editor` Debug build.

Suggested local build command:

```bash
xcodebuild -project HSMR.xcodeproj -scheme Editor build -configuration Debug -derivedDataPath DerivedData CLANG_MODULE_CACHE_PATH=ModuleCache
```

## Current Refactor Status

As of this document:

- panel registration and lifecycle are centralized in `EditorWindow`
- panel visibility is bindable and shared
- panel frame structure is standardized
- inspector-style forms use a shared layout helper
- list/card visual language is shared
- tree interaction rules are shared
- project launcher dialog forms now also use the shared form layout

This does **not** mean the refactor is complete.

Remaining likely areas for future cleanup:
- more consistent context menu helper patterns
- continued removal of one-off ImGui row code from large panels
- stronger separation between editor behavior and rendering-specific details inside panel implementations

## If You Are an Agent

Before changing panel UI code:

1. Check whether the change belongs in:
   - `EditorPanelFrame`
   - `EditorFormLayout`
   - `EditorListWidgets`
   - `EditorTreeWidgets`
   - `EditorDialogFrame`
   - `EditorFeedbackWidgets`
2. Prefer using or extending an existing helper rather than introducing panel-local layout logic.
3. Preserve stable `panelId` values.
4. Avoid moving shared editor state out of `EditorContext` unless there is a deliberate architectural reason.
5. Keep helpers small and composable.

When in doubt, bias toward:
- less duplication
- lower coupling
- more consistent UI semantics
