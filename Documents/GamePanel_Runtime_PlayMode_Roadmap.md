# Game Panel Runtime PlayMode Roadmap

> Created: 2026-04-09
> Purpose: Track planned follow-up work that should not be folded into Game Panel v1.

---

## 1. Why This Exists

`GamePanel` v1 is a display-only game view.

That is the right first step, but it is not the full editor gameplay workflow.
If runtime systems are mixed into v1 immediately, the scope becomes much larger and the renderer work becomes harder to stabilize.

This document exists to keep the follow-up path clear without forcing it into the first implementation.

---

## 2. Target End State

Long term, the editor should support:

- editing the scene in `ScenePanel`
- previewing the camera output in `GamePanel`
- entering a runtime play mode without mutating the authoring scene directly
- stopping play mode and returning safely to the authoring scene state

---

## 3. Planned Follow-Up Systems

### 3.1 Runtime scene instance

Needed capability:
- create a runtime copy or runtime-backed instance of the active scene
- keep authoring scene and runtime scene separate

Reason:
- play mode should not directly mutate editor-authoring state

### 3.2 Play mode state machine

Needed editor states:
- Edit
- Play
- Pause
- Step

Expected ownership:
- `EditorContext` or a dedicated editor runtime controller owns the state

### 3.3 Runtime update loop

Needed systems:
- runtime scene ticking
- physics stepping
- animation update
- script update
- audio update

This should be isolated from editor-only camera/gizmo update logic.

### 3.4 Input focus and routing

Needed behavior:
- when Game panel is focused in Play mode, input can drive the runtime world
- Scene panel editor camera controls must not steal that input

### 3.5 Runtime UI/HUD

Needed capability:
- runtime UI rendering in the Game panel
- optional editor controls layered around it, not inside it

### 3.6 Play-mode camera ownership

Needed rule:
- in Play mode, Game panel camera comes from runtime logic
- in Edit mode, Game panel camera comes from selected scene camera / auto camera selection

---

## 4. Suggested Order After v1

### Stage 1

- complete Game panel v1
- separate game renderer from editor overlay renderer
- make render-view ownership explicit

### Stage 2

- add editor play mode state
- build runtime scene instance creation / teardown

### Stage 3

- update runtime systems while in Play mode
- route input into runtime world when Game panel is focused

### Stage 4

- add Pause / Step
- add runtime HUD / game UI
- add better panel controls and diagnostics

---

## 5. Guardrails

To keep future work maintainable:

- do not make `GamePanel` depend on `EditorCamera`
- do not make runtime systems depend on editor overlay rendering
- do not use the authoring scene as the live runtime scene once Play mode exists
- do not couple play-mode input handling to `ScenePanel`

---

## 6. Nice-To-Have Features Later

These are useful, but not part of the immediate follow-up:

- aspect ratio presets
- resolution presets
- multiple simultaneous game cameras
- split-screen preview
- safe-area overlays
- runtime statistics overlay
- per-camera post-process controls
- record / capture tools

