#pragma once

#include "Precompile.h"

#include "Core/Math/Common.h"

HS_NS_BEGIN

namespace CoordinateConvention
{
inline constexpr bool IsLeftHanded = true;

inline constexpr glm::vec3 WorldRight{1.0f, 0.0f, 0.0f};
inline constexpr glm::vec3 WorldUp{0.0f, 1.0f, 0.0f};

// Engine camera/view math uses +Z as forward in left-handed space.
inline constexpr glm::vec3 CameraForward{0.0f, 0.0f, 1.0f};

// Some legacy object/light helpers still author "forward" as -Z.
// Keep this named constant so the inconsistency is searchable and explicit.
inline constexpr glm::vec3 LegacyObjectForward{0.0f, 0.0f, -1.0f};
}

HS_NS_END
