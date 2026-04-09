# Editor GUI Guidelines

## Summary
- Editor panels should use shared panel/frame helpers instead of ad-hoc `ImGui::Begin`, `SameLine`, and `SetCursorPos` layout code.
- Global look-and-feel lives in `GUIContext`.
- Shared panel structure lives in `EditorPanelFrame`.
- Shared reusable widgets live in `EditorWidgets`.

## Panel Types
- Standard panel:
  - Use `EditorPanelFrame::BeginStandardPanel()`
  - Use default editor padding from `GUIContext`
- Viewport panel:
  - Use `BeginStandardPanel()` with menu bar enabled
  - Put the render texture inside `BeginPanelContent()` with zero padding
- Inspector-like panel:
  - Use standard panel frame
  - Use removable section headers from `EditorWidgets::BeginRemovableSectionHeader()`

## Layout Rules
- Do not use raw `SetCursorPosX/Y()` for normal alignment when a shared helper can express the layout.
- Do not push panel-specific `WindowPadding` on the outer window unless the panel is introducing a new common rule.
- For right-aligned controls in toolbars or menu bars, use `EditorWidgets::RightAlignNextItem()`.
- For right-aligned search fields, use `EditorWidgets::SearchFieldRightAligned()`.
- For split panes, use `EditorWidgets::DrawVerticalSplitter()` and keep min/max widths explicit.

## Icon Rules
- Use `EditorIcons` for semantic editor-wide icons.
- Use `MaterialSymbolsIcons` directly only for one-off panel-specific cases.
- Use icon button variants by context:
  - `IconButtonMenuBar()` for menu bars
  - `IconButtonHeader()` for section header actions
  - `IconButtonSmall()` for compact toolbar controls

## Expectations For New Panels
- A new panel should usually need:
  - one `BeginStandardPanel()`
  - optional `BeginPanelMenuBar()`
  - optional `BeginPanelContent()`
  - shared `EditorWidgets` controls
- If a new panel requires custom cursor-position math for routine layout, treat that as a missing shared helper and add the helper first.
