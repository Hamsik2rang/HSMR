//
//  EditorCamera.cpp
//  HSMR
//
#include "EditorCamera.h"

#include "RHI/RHIContext.h"

HS_NS_EDITOR_BEGIN

EditorCamera::EditorCamera()
    : _yaw(0.0f)
    , _pitch(0.0f)
    , _moveSpeed(5.0f)
    , _rotateSpeed(0.003f)
    , _orthoLeft(-10.0f)
    , _orthoRight(10.0f)
    , _orthoBottom(-10.0f)
    , _orthoTop(10.0f)
    , _viewDirty(true)
    , _projectionDirty(true)
{
    // Match Camera's default: position (0, 2, -5), looking at +Z in LH
    _transform.SetPosition(glm::vec3(0.0f, 2.0f, -5.0f));

    // CameraComponent defaults: fov=60 degrees, 16:9, near=0.1, far=1000
    // These match Camera's defaults

    // Compute initial quaternion from euler (pitch=0, yaw=0)
    updateDirectionFromEuler();

    // Force initial matrix computation
    Update();
}

void EditorCamera::Move(const glm::vec3& offset)
{
    _transform.Translate(offset);
    _viewDirty = true;
}

void EditorCamera::Rotate(float yawDelta, float pitchDelta)
{
    _yaw += yawDelta;
    _pitch += pitchDelta;

    // Clamp pitch to avoid gimbal lock
    _pitch = glm::clamp(_pitch, -glm::half_pi<float>() + 0.01f, glm::half_pi<float>() - 0.01f);

    updateDirectionFromEuler();
    _viewDirty = true;
}

void EditorCamera::Orbit(float yaw, float pitch, const glm::vec3& target)
{
    float distance = glm::length(_transform.position - target);

    _yaw += yaw;
    _pitch += pitch;

    // Clamp pitch to avoid gimbal lock
    _pitch = glm::clamp(_pitch, -glm::half_pi<float>() + 0.01f, glm::half_pi<float>() - 0.01f);

    // Calculate new position on sphere around target
    float x = target.x + distance * cos(_pitch) * sin(_yaw);
    float y = target.y + distance * sin(_pitch);
    float z = target.z + distance * cos(_pitch) * cos(_yaw);

    _transform.SetPosition(glm::vec3(x, y, z));

    updateDirectionFromEuler();
    _viewDirty = true;
}

void EditorCamera::Dolly(float distance, const glm::vec3& target)
{
    glm::vec3 direction = glm::normalize(_transform.position - target);
    float currentDist = glm::length(_transform.position - target);
    float newDist = glm::max(0.1f, currentDist + distance);
    _transform.SetPosition(target + direction * newDist);
    _viewDirty = true;
}

void EditorCamera::Update()
{
    if (_viewDirty)
    {
        updateViewMatrix();
    }
    if (_projectionDirty)
    {
        updateProjectionMatrix();
    }
    if (_viewDirty || _projectionDirty)
    {
        updateViewProjectionMatrix();
        _viewDirty = false;
        _projectionDirty = false;
    }
}

void EditorCamera::SetPosition(const glm::vec3& position)
{
    _transform.SetPosition(position);
    _viewDirty = true;
}

void EditorCamera::SetRotation(const glm::vec3& eulerRadians)
{
    _pitch = eulerRadians.x;
    _yaw = eulerRadians.y;

    // Clamp pitch
    _pitch = glm::clamp(_pitch, -glm::half_pi<float>() + 0.01f, glm::half_pi<float>() - 0.01f);

    updateDirectionFromEuler();
    _viewDirty = true;
}

glm::vec3 EditorCamera::GetRotation() const
{
    return glm::vec3(_pitch, _yaw, 0.0f);
}

void EditorCamera::SetPerspective(float fovY, float aspectRatio, float nearZ, float farZ)
{
    _camera.projectionType = CameraComponent::EProjectionType::Perspective;
    _camera.fov = glm::degrees(fovY);
    _camera.aspectRatio = aspectRatio;
    _camera.nearPlane = nearZ;
    _camera.farPlane = farZ;
    _projectionDirty = true;
}

void EditorCamera::SetOrthographic(float left, float right, float bottom, float top, float nearZ, float farZ)
{
    _camera.projectionType = CameraComponent::EProjectionType::Orthographic;
    _orthoLeft = left;
    _orthoRight = right;
    _orthoBottom = bottom;
    _orthoTop = top;
    _camera.nearPlane = nearZ;
    _camera.farPlane = farZ;
    _projectionDirty = true;
}

glm::vec3 EditorCamera::ScreenToWorldPoint(const glm::vec3& screenPos) const
{
    glm::vec4 screenHPos = glm::vec4(screenPos, 1.0f);
    glm::vec4 world = _inverseViewProjectionMatrix * screenHPos;
    if (world.w != 0.0f)
    {
        world /= world.w;
    }
    return glm::vec3(world);
}

glm::vec3 EditorCamera::ScreenToWorldDirection(const glm::vec3& screenDir) const
{
    glm::vec4 screenHDir = glm::vec4(screenDir, 0.0f);
    glm::vec4 world = _inverseViewMatrix * screenHDir;
    return glm::vec3(world);
}

glm::vec2 EditorCamera::WorldToScreenPoint(const glm::vec3& worldPos) const
{
    glm::vec4 clipSpacePos = _viewProjectionMatrix * glm::vec4(worldPos, 1.0f);
    if (clipSpacePos.w != 0.0f)
    {
        glm::vec3 ndcSpacePos = glm::vec3(clipSpacePos) / clipSpacePos.w;
        return glm::vec2(ndcSpacePos.x, ndcSpacePos.y);
    }
    return glm::vec2(0.0f);
}

void EditorCamera::updateDirectionFromEuler()
{
    // Build quaternion from yaw/pitch euler angles (LH convention)
    // Yaw rotates around Y, pitch rotates around X
    // Negate pitch because angleAxis(+angle, X) rotates +Z toward -Y,
    // but we want positive pitch to look UP (front.y > 0)
    glm::quat yawQuat = glm::angleAxis(_yaw, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::quat pitchQuat = glm::angleAxis(-_pitch, glm::vec3(1.0f, 0.0f, 0.0f));
    _transform.SetRotation(yawQuat * pitchQuat);
}

void EditorCamera::updateViewMatrix()
{
    glm::vec3 front = GetForward();
    glm::vec3 up = GetUp();

    _viewMatrix = glm::lookAtLH(_transform.position, _transform.position + front, up);
    _inverseViewMatrix = glm::inverse(_viewMatrix);
}

void EditorCamera::updateProjectionMatrix()
{
    if (_camera.projectionType == CameraComponent::EProjectionType::Perspective)
    {
        _projectionMatrix = glm::perspectiveLH(
            glm::radians(_camera.fov), _camera.aspectRatio,
            _camera.nearPlane, _camera.farPlane);
    }
    else
    {
        _projectionMatrix = glm::orthoLH(
            _orthoLeft, _orthoRight, _orthoBottom, _orthoTop,
            _camera.nearPlane, _camera.farPlane);
    }

    // Invert Y for Vulkan NDC
    if (RHIContext::Get()->GetCurrentPlatform() == ERHIPlatform::Vulkan)
    {
        _projectionMatrix[1][1] *= -1;
    }

    _inverseProjectionMatrix = glm::inverse(_projectionMatrix);
}

void EditorCamera::updateViewProjectionMatrix()
{
    _viewProjectionMatrix = _projectionMatrix * _viewMatrix;
    _inverseViewProjectionMatrix = glm::inverse(_viewProjectionMatrix);
}

HS_NS_EDITOR_END
