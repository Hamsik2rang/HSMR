#include "Engine/Camera.h"

HS_NS_BEGIN

Camera::Camera()
: _position(0.0f, 2.0f, 5.0f)
, _rotation(0.0f, 0.0f, 0.0f)
, _frontDir(0.0f, 0.0f, -1.0f)
, _upDir(0.0f, 1.0f, 0.0f)
, _projectionType(EProjectionType::PERSPECTIVE)
, _fovY(glm::radians(60.0f))
, _aspectRatio(16.0f / 9.0f)
, _nearZ(0.1f)
, _farZ(1000.0f)
, _left(-10.0f)
, _right(10.0f)
, _bottom(-10.0f)
, _top(10.0f)
, _moveSpeed(5.0f)
, _rotateSpeed(0.003f)
, _viewDirty(true)
, _projectionDirty(true)
{
    Update();
}

void Camera::SetPerspective(float fovY, float aspectRatio, float nearZ, float farZ)
{
    _projectionType = EProjectionType::PERSPECTIVE;
    _fovY = fovY;
    _aspectRatio = aspectRatio;
    _nearZ = nearZ;
    _farZ = farZ;
    _projectionDirty = true;
}

void Camera::SetOrthographic(float left, float right, float bottom, float top, float nearZ, float farZ)
{
    _projectionType = EProjectionType::ORTHOGRAPHIC;
    _left = left;
    _right = right;
    _bottom = bottom;
    _top = top;
    _nearZ = nearZ;
    _farZ = farZ;
    _projectionDirty = true;
}

void Camera::Update()
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

void Camera::updateDirectionVectors()
{
    // Calculate front vector from euler angles
    float pitch = _rotation.x;
    float yaw = _rotation.y;
    
    glm::vec3 front;
    front.x = cos(pitch) * sin(yaw);
    front.y = sin(pitch);
    front.z = cos(pitch) * cos(yaw);
    _frontDir = glm::normalize(front);
    
    // Recalculate up vector
    glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
    glm::vec3 right = glm::normalize(glm::cross(worldUp, _frontDir));
    _upDir = glm::normalize(glm::cross(_frontDir, right));
}

void Camera::updateViewMatrix()
{
    _viewMatrix = glm::lookAt(_position, _position + _frontDir, _upDir);
    _inverseViewMatrix = glm::inverse(_viewMatrix);
}

void Camera::updateProjectionMatrix()
{
    switch (_projectionType)
    {
        case EProjectionType::PERSPECTIVE:
            _projectionMatrix = glm::perspective(_fovY, _aspectRatio, _nearZ, _farZ);
            break;
        case EProjectionType::ORTHOGRAPHIC:
            _projectionMatrix = glm::ortho(_left, _right, _bottom, _top, _nearZ, _farZ);
            break;
    }
    _inverseProjectionMatrix = glm::inverse(_projectionMatrix);
}

void Camera::updateViewProjectionMatrix()
{
    _viewProjectionMatrix = _projectionMatrix * _viewMatrix;
    _inverseViewProjectionMatrix = glm::inverse(_viewProjectionMatrix);
}

glm::vec3 Camera::ScreenToWorldPoint(const glm::vec3& screenPos) const
{
    glm::vec4 screenHPos = glm::vec4(screenPos, 1.0f);
    glm::vec4 world = _inverseViewProjectionMatrix * screenHPos;
    if (world.w != 0.0f)
    {
        world /= world.w;
    }
    return glm::vec3(world);
}

glm::vec3 Camera::ScreenToWorldDirection(const glm::vec3& screenDir) const
{
    glm::vec4 screenHDir = glm::vec4(screenDir, 0.0f);
    glm::vec4 world = _inverseViewMatrix * screenHDir;
    return glm::vec3(world);
}

glm::vec2 Camera::WorldToScreenPoint(const glm::vec3& worldPos) const
{
    glm::vec4 clipSpacePos = _viewProjectionMatrix * glm::vec4(worldPos, 1.0f);
    if (clipSpacePos.w != 0.0f)
    {
        glm::vec3 ndcSpacePos = glm::vec3(clipSpacePos) / clipSpacePos.w;
        return glm::vec2(ndcSpacePos.x, ndcSpacePos.y);
    }
    return glm::vec2(0.0f);
}

void Camera::Orbit(float yaw, float pitch, const glm::vec3& target)
{
    // Calculate distance from target
    float distance = glm::length(_position - target);
    
    // Update rotation
    _rotation.y += yaw;
    _rotation.x += pitch;
    
    // Clamp pitch to avoid gimbal lock
    _rotation.x = glm::clamp(_rotation.x, -glm::half_pi<float>() + 0.01f, glm::half_pi<float>() - 0.01f);
    
    // Calculate new position
    float x = target.x + distance * cos(_rotation.x) * sin(_rotation.y);
    float y = target.y + distance * sin(_rotation.x);
    float z = target.z + distance * cos(_rotation.x) * cos(_rotation.y);
    
    _position = glm::vec3(x, y, z);
    _frontDir = glm::normalize(target - _position);
    
    // Recalculate up vector
    glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
    glm::vec3 right = glm::normalize(glm::cross(worldUp, _frontDir));
    _upDir = glm::normalize(glm::cross(_frontDir, right));
    
    _viewDirty = true;
}

void Camera::Dolly(float distance, const glm::vec3& target)
{
    glm::vec3 direction = glm::normalize(_position - target);
    float currentDist = glm::length(_position - target);
    float newDist = glm::max(0.1f, currentDist + distance);
    _position = target + direction * newDist;
    _viewDirty = true;
}



HS_NS_END

