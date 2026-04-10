#include "Renderer/CoordinateConventionValidation.h"

#include "Core/Log.h"
#include "Core/Math/CoordinateConvention.h"

#include "Renderer/CameraUtils.h"

#include "Scene/Components/CameraComponent.h"
#include "Scene/Components/TransformComponent.h"

HS_NS_BEGIN

namespace
{
bool s_hasValidatedCoordinateConvention = false;

bool nearlyEqual(float lhs, float rhs, float epsilon = 0.0001f)
{
    return glm::abs(lhs - rhs) <= epsilon;
}

bool nearlyEqualVec3(const glm::vec3& lhs, const glm::vec3& rhs, float epsilon = 0.0001f)
{
    return glm::all(glm::lessThanEqual(glm::abs(lhs - rhs), glm::vec3(epsilon)));
}

bool nearlyEqualMat4(const glm::mat4& lhs, const glm::mat4& rhs, float epsilon = 0.0001f)
{
    for (int row = 0; row < 4; ++row)
    {
        for (int col = 0; col < 4; ++col)
        {
            if (!nearlyEqual(lhs[row][col], rhs[row][col], epsilon))
            {
                return false;
            }
        }
    }

    return true;
}
}

void CoordinateConventionValidation::RunOnce()
{
    if (s_hasValidatedCoordinateConvention)
    {
        return;
    }
    s_hasValidatedCoordinateConvention = true;

    HS_ASSERT(CoordinateConvention::IsLeftHanded, "Renderer convention expects left-handed space");
    HS_ASSERT(
        nearlyEqualVec3(CoordinateConvention::WorldUp, glm::vec3(0.0f, 1.0f, 0.0f)),
        "Renderer convention expects +Y world up");
    HS_ASSERT(
        nearlyEqualVec3(CoordinateConvention::CameraForward, glm::vec3(0.0f, 0.0f, 1.0f)),
        "Renderer convention expects +Z camera forward");

    TransformComponent transform{};
    transform.worldMatrix = glm::mat4(1.0f);

    CameraComponent camera{};
    camera.isActive = true;

    const glm::vec3 forward = CameraUtils::ComputeCameraForwardWS(transform);
    HS_ASSERT(
        nearlyEqualVec3(forward, CoordinateConvention::CameraForward),
        "Identity camera transform must face +Z");

    const glm::mat4 view = CameraUtils::ComputeViewMatrixLH(transform);
    const glm::vec4 pointInFrontCS = view * glm::vec4(CoordinateConvention::CameraForward * 5.0f, 1.0f);
    HS_ASSERT(pointInFrontCS.z > 0.0f, "Point at +Z must be in front of the camera");

    const glm::mat4 projNoFlip = CameraUtils::ComputeProjectionMatrixLH(camera, false);
    const glm::mat4 projFlip = CameraUtils::ComputeProjectionMatrixLH(camera, true);
    glm::mat4 expectedFlipped = projNoFlip;
    expectedFlipped[1][1] *= -1.0f;
    HS_ASSERT(
        nearlyEqualMat4(projFlip, expectedFlipped),
        "Vulkan Y-flip must only invert the projection Y axis");

    HS_LOG(
        info,
        "[CoordinateConvention] LH=%d, WorldUp=(%.1f, %.1f, %.1f), CameraForward=(%.1f, %.1f, %.1f)",
        CoordinateConvention::IsLeftHanded ? 1 : 0,
        CoordinateConvention::WorldUp.x, CoordinateConvention::WorldUp.y, CoordinateConvention::WorldUp.z,
        CoordinateConvention::CameraForward.x, CoordinateConvention::CameraForward.y, CoordinateConvention::CameraForward.z);
}

HS_NS_END
