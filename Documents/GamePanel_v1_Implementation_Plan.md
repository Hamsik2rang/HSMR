# Game Panel v1 Implementation Plan

> Created: 2026-04-09
> Purpose: Define the first implementation scope for the editor Game panel before code changes begin.

---

## 1. Goal

Add a `GamePanel` to the editor that shows how the current scene would look in the shipped game.

This panel must be different from `ScenePanel`:

- `ScenePanel` is an editor view driven by `EditorCamera`
- `GamePanel` is a game view driven by a scene camera
- `ScenePanel` shows editor-only overlays such as grid and debug lines
- `GamePanel` must render only the game view

---

## 2. Required Behavior

### 2.1 Panel responsibilities

`ScenePanel`
- Uses editor camera controls
- Shows editor overlays
- Keeps transform gizmo / picking workflow

`GamePanel`
- Does not use editor camera controls
- Uses a scene camera selected from the active scene
- Does not show grid / debug overlay / editor gizmos

### 2.2 Camera selection

The Game panel must support multiple scene cameras.

Rules:
- Default camera = highest priority active camera in the scene
- If there is a manual camera selection in the Game panel, use that camera
- If the selected camera becomes invalid, fall back to the default camera

### 2.3 Camera priority

`CameraComponent` needs a new integer field:

- `int priority = 0`

Selection order:
1. `isActive == true`
2. higher `priority`
3. `isPrimary == true`
4. stable fallback such as entity id / creation order

---

## 3. Design Direction

### 3.1 Renderer split

The current renderer should be split into:

- game rendering path
- editor overlay rendering path

Recommended structure:

- `ForwardRenderer` becomes the base game renderer
- grid/debug/editor-only overlay logic moves into an editor-only layer
- `ScenePanel` uses `game renderer + editor overlay renderer`
- `GamePanel` uses `game renderer only`

This avoids duplicating the core rendering pipeline while keeping editor-only features out of the game view.

### 3.2 View selection split

The current flow overrides `RenderSceneSnapshot.views[0]` with the editor camera view.
That should be replaced.

Recommended direction:

- `RenderSceneSnapshot` contains scene content only
- the actual render camera is passed separately as `RenderViewSnapshot`

That gives us:

- Scene panel render with editor camera
- Game panel render with selected scene camera
- no cross-panel view stomping

### 3.3 Render target split

Scene and Game panels need separate render targets.

Reason:
- panel sizes may differ
- both panels may be visible at the same time
- each panel must render independently per frame

Recommended ownership:

- Editor window owns one per-frame RT set for Scene panel
- Editor window owns one per-frame RT set for Game panel

---

## 4. Implementation Scope

### Phase 1: Camera data and selection

- Add `priority` to `CameraComponent`
- Add helper to find best active game camera in a scene
- Add helper to build a `RenderViewSnapshot` from a scene camera entity

### Phase 2: Renderer structure cleanup

- Move `GridPass` and `DebugPass` out of the base game renderer path
- Keep game rendering reusable by both Scene and Game panels
- Keep editor overlay rendering available only to Scene panel

### Phase 3: Game panel UI

- Add `GamePanel`
- Add panel menu bar
- Add camera selection UI
- Add auto/manual camera mode

### Phase 4: Editor window integration

- Allocate game-panel render targets
- Render scene view and game view separately in the same frame
- Route the correct texture to each panel

### Phase 5: Stabilization

- Handle camera deletion / deactivation
- Keep aspect ratio correct
- Make fallback behavior deterministic

---

## 5. Explicit Non-Goals For v1

The following are intentionally excluded from v1:

- Play / Pause / Stop runtime simulation
- separate runtime world cloning
- input routing into gameplay
- physics stepping for play mode
- script runtime update
- audio listener / game audio playback
- runtime HUD / UI rendering
- split-screen / multi-camera viewport composition

These are tracked in a separate roadmap document.

---

## 6. Acceptance Criteria

Game Panel v1 is complete when:

- a dockable `GamePanel` exists
- it renders without grid and debug lines
- it shows the highest-priority active scene camera by default
- the user can choose another scene camera from the panel menu bar
- Scene panel still shows editor camera view with editor overlays
- both panels can be visible simultaneously without interfering with each other

---

## 7. Risks

### 7.1 View ownership risk

If we keep mutating `RenderSceneSnapshot.views[0]`, Scene and Game rendering will fight over the same view data.

Mitigation:
- separate scene content snapshot from selected render view

### 7.2 Renderer coupling risk

If grid/debug remain inside the main renderer, Game panel will keep inheriting editor-only behavior.

Mitigation:
- move editor-only passes into a distinct overlay path

### 7.3 Scope creep risk

Game panel often tempts us to implement play mode too early.

Mitigation:
- ship v1 as a display-only game view first
- keep runtime-play features in a separate roadmap

