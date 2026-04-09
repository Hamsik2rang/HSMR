# Coordinate System

HSMR rendering code uses a single engine-wide convention for camera/view math:

- Left-handed world space
- `+Y` is up
- Camera/view forward is `+Z`
- Camera/view matrices and projection helpers are left-handed
- Vulkan keeps the same camera convention and only flips projection `Y`

## Practical Rules

- "5 meters in front of the camera" means `cameraPosition + cameraForward * 5`
- "Behind the camera" means `cameraPosition - cameraForward * distance`
- Perspective and orthographic frustum near/far corners are built on the camera's local `+Z` axis

## Source Of Truth In Code

Use [CoordinateConvention.h](/Users/yongsikim/Desktop/Dev/HSMR/Source/Core/Math/CoordinateConvention.h) instead of hard-coded direction vectors:

- `CoordinateConvention::WorldUp`
- `CoordinateConvention::CameraForward`
- `CoordinateConvention::LegacyObjectForward`

`LegacyObjectForward` exists because some older object/light helpers still interpret forward as `-Z`. That path is intentionally named so the inconsistency is searchable and can be migrated safely. New camera/view/frustum code should use `CameraForward`.

## Validation Harness

The renderer runs a lightweight self-check during initialization through [CoordinateConventionValidation.cpp](/Users/yongsikim/Desktop/Dev/HSMR/Source/Renderer/Private/CoordinateConventionValidation.cpp). It validates:

- `WorldUp == +Y`
- `CameraForward == +Z`
- identity camera transform looks toward `+Z`
- Vulkan Y-flip only flips projection `Y`

If you touch camera math, frustum generation, or projection code, update the validation if the convention changes.
