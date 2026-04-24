#ifndef __HS_EDITOR_CAMERA_H__
#define __HS_EDITOR_CAMERA_H__

#include "Precompile.h"

#include "Core/Math/CoordinateConvention.h"
#include "Core/Math/Common.h"
#include "Scene/Components/TransformComponent.h"
#include "Scene/Components/CameraComponent.h"

HS_NS_EDITOR_BEGIN

class HS_EDITOR_API EditorCamera
{
public:
    EditorCamera();
    ~EditorCamera() = default;

    // ===== Camera manipulation (editor input) =====

    void Move(const glm::vec3& offset);
    void Rotate(float yawDelta, float pitchDelta);
    void Orbit(float yaw, float pitch, const glm::vec3& target);
    void Dolly(float distance, const glm::vec3& target);

    // Update cached matrices (call before accessing matrices)
    void Update();

    // ===== Matrix access (cached) =====

    const glm::mat4& GetViewMatrix() const { return _viewMatrix; }
    const glm::mat4& GetProjectionMatrix() const { return _projectionMatrix; }
    const glm::mat4& GetViewProjectionMatrix() const { return _viewProjectionMatrix; }

    const glm::mat4& GetInverseViewMatrix() const { return _inverseViewMatrix; }
    const glm::mat4& GetInverseProjectionMatrix() const { return _inverseProjectionMatrix; }
    const glm::mat4& GetInverseViewProjectionMatrix() const { return _inverseViewProjectionMatrix; }

    // ===== Component access (ECS compatibility) =====

    const TransformComponent& GetTransform() const { return _transform; }
    const CameraComponent& GetCameraComponent() const { return _camera; }

    // ===== Transform convenience (Camera-compatible interface) =====

    void SetPosition(const glm::vec3& position);
    void SetRotation(const glm::vec3& eulerRadians);  // pitch(x), yaw(y), roll(z)

    HS_FORCEINLINE glm::vec3 GetPosition() const { return _transform.position; }
    glm::vec3 GetRotation() const;  // returns euler radians: pitch(x), yaw(y), 0

    // Direction vectors use the engine camera convention: +Z forward in LH space.
    HS_FORCEINLINE glm::vec3 GetForward() const { return glm::normalize(_transform.rotation * CoordinateConvention::CameraForward); }
    HS_FORCEINLINE glm::vec3 GetUp() const { return _transform.GetUp(); }
    HS_FORCEINLINE glm::vec3 GetRight() const { return _transform.GetRight(); }

    // ===== Projection parameters =====

    HS_FORCEINLINE void SetProjectionType(CameraComponent::EProjectionType type)
    {
        _camera.projectionType = type;
        _projectionDirty = true;
    }
    HS_FORCEINLINE CameraComponent::EProjectionType GetProjectionType() const { return _camera.projectionType; }

    void SetPerspective(float fovY, float aspectRatio, float nearZ, float farZ);
    void SetOrthographic(float left, float right, float bottom, float top, float nearZ, float farZ);

    HS_FORCEINLINE float GetFov() const { return glm::radians(_camera.fov); }  // returns radians
    HS_FORCEINLINE void SetFov(float fovRadians)
    {
        _camera.fov = glm::degrees(fovRadians);
        _projectionDirty = true;
    }

    HS_FORCEINLINE float GetAspectRatio() const { return _camera.aspectRatio; }
    HS_FORCEINLINE void SetAspectRatio(float aspectRatio)
    {
        _camera.aspectRatio = aspectRatio;
        _projectionDirty = true;
    }

    HS_FORCEINLINE float GetNearZ() const { return _camera.nearPlane; }
    HS_FORCEINLINE void SetNearZ(float nearZ)
    {
        _camera.nearPlane = nearZ;
        _projectionDirty = true;
    }

    HS_FORCEINLINE float GetFarZ() const { return _camera.farPlane; }
    HS_FORCEINLINE void SetFarZ(float farZ)
    {
        _camera.farPlane = farZ;
        _projectionDirty = true;
    }

    // ===== Editor-specific parameters =====

    HS_FORCEINLINE void SetMoveSpeed(float speed) { _moveSpeed = speed; }
    HS_FORCEINLINE float GetMoveSpeed() const { return _moveSpeed; }
    HS_FORCEINLINE float GetRotateSpeed() const { return _rotateSpeed; }

    // ===== Coordinate conversion =====

    glm::vec3 ScreenToWorldPoint(const glm::vec3& screenPos) const;
    glm::vec3 ScreenToWorldDirection(const glm::vec3& screenDir) const;
    glm::vec2 WorldToScreenPoint(const glm::vec3& worldPos) const;

private:
    void updateDirectionFromEuler();
    void updateViewMatrix();
    void updateProjectionMatrix();
    void updateViewProjectionMatrix();

    // ECS components (data source of truth)
    TransformComponent _transform;
    CameraComponent _camera;

    // Editor-specific state
    float _yaw;     // radians
    float _pitch;   // radians
    float _moveSpeed;
    float _rotateSpeed;

    // Orthographic parameters (not in CameraComponent)
    float _orthoLeft;
    float _orthoRight;
    float _orthoBottom;
    float _orthoTop;

    // Matrix cache
    glm::mat4 _viewMatrix;
    glm::mat4 _projectionMatrix;
    glm::mat4 _viewProjectionMatrix;

    glm::mat4 _inverseViewMatrix;
    glm::mat4 _inverseProjectionMatrix;
    glm::mat4 _inverseViewProjectionMatrix;

    // Dirty flags
    bool _viewDirty;
    bool _projectionDirty;
};

HS_NS_EDITOR_END

#endif // __HS_EDITOR_CAMERA_H__
