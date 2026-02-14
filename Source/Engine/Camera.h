//
//  Camera.h
//  HSMR
//
#ifndef __HS_CAMERA_H__
#define __HS_CAMERA_H__

#include "Precompile.h"
#include "Core/Math/Common.h"

HS_NS_BEGIN

// Camera class for scene viewing (based on EditorCamera)
class HS_API Camera
{
public:
    enum class EProjectionType
    {
        Perspective  = 0,
        Orthographic = 1,
    };

    Camera();
    ~Camera() = default;

    // Transform setters
    HS_FORCEINLINE void SetPosition(const glm::vec3& position)
    {
        _position  = position;
        _viewDirty = true;
    }

    HS_FORCEINLINE void SetRotation(const glm::vec3& rotation)
    {
        _rotation = rotation;
        updateDirectionVectors();
        _viewDirty = true;
    }

    HS_FORCEINLINE void SetTarget(const glm::vec3& target)
    {
        _frontDir  = glm::normalize(target - _position);
        _viewDirty = true;
    }

    HS_FORCEINLINE void SetMoveSpeed(float speed) { _moveSpeed = speed; }
    HS_FORCEINLINE float GetMoveSpeed() const { return _moveSpeed; }

    HS_FORCEINLINE float GetRotateSpeed() const { return _rotateSpeed; }

    // Transform getters
    HS_FORCEINLINE glm::vec3 GetPosition() const { return _position; }
    HS_FORCEINLINE glm::vec3 GetRotation() const { return _rotation; }

    // Direction vectors
    HS_FORCEINLINE glm::vec3 GetForward() const { return _frontDir; }
    HS_FORCEINLINE glm::vec3 GetUp() const { return _upDir; }
    HS_FORCEINLINE glm::vec3 GetRight() const { return glm::normalize(glm::cross(_upDir, _frontDir)); }

    // Camera manipulation helpers
    HS_FORCEINLINE void Move(const glm::vec3& offset)
    {
        _position += offset;
        _viewDirty = true;
    }

    HS_FORCEINLINE void Rotate(const glm::vec3& eulerAngles)
    {
        _rotation += eulerAngles;
        updateDirectionVectors();
        _viewDirty = true;
    }

    // Projection type
    HS_FORCEINLINE void SetProjectionType(EProjectionType type)
    {
        _projectionType  = type;
        _projectionDirty = true;
    }
    HS_FORCEINLINE EProjectionType GetProjectionType() const { return _projectionType; }

    // Perspective projection
    void SetPerspective(float fovY, float aspectRatio, float nearZ, float farZ);

    // Orthographic projection
    void SetOrthographic(float left, float right, float bottom, float top, float nearZ, float farZ);

    // Projection parameters
    HS_FORCEINLINE float GetFov() const { return _fovY; }
    HS_FORCEINLINE void SetFov(float fovY)
    {
        _fovY            = fovY;
        _projectionDirty = true;
    }

    HS_FORCEINLINE float GetAspectRatio() const { return _aspectRatio; }
    HS_FORCEINLINE void SetAspectRatio(float aspectRatio)
    {
        _aspectRatio     = aspectRatio;
        _projectionDirty = true;
    }

    HS_FORCEINLINE float GetNearZ() const { return _nearZ; }
    HS_FORCEINLINE void SetNearZ(float nearZ)
    {
        _nearZ           = nearZ;
        _projectionDirty = true;
    }

    HS_FORCEINLINE float GetFarZ() const { return _farZ; }
    HS_FORCEINLINE void SetFarZ(float farZ)
    {
        _farZ            = farZ;
        _projectionDirty = true;
    }

    // Update matrices (call before accessing matrices)
    void Update();

    // Matrix access (call Update() first)
    const glm::mat4& GetViewMatrix() const { return _viewMatrix; }
    const glm::mat4& GetProjectionMatrix() const { return _projectionMatrix; }
    const glm::mat4& GetViewProjectionMatrix() const { return _viewProjectionMatrix; }

    // Inverse matrices
    const glm::mat4& GetInverseViewMatrix() const { return _inverseViewMatrix; }
    const glm::mat4& GetInverseProjectionMatrix() const { return _inverseProjectionMatrix; }
    const glm::mat4& GetInverseViewProjectionMatrix() const { return _inverseViewProjectionMatrix; }

    // Coordinate conversion
    glm::vec3 ScreenToWorldPoint(const glm::vec3& screenPos) const;
    glm::vec3 ScreenToWorldDirection(const glm::vec3& screenDir) const;
    glm::vec2 WorldToScreenPoint(const glm::vec3& worldPos) const;

    // Orbit camera controls
    void Orbit(float yaw, float pitch, const glm::vec3& target);
    void Dolly(float distance, const glm::vec3& target);

private:
    void updateDirectionVectors();
    void updateViewMatrix();
    void updateProjectionMatrix();
    void updateViewProjectionMatrix();

    // Transform data
    glm::vec3 _position;
    glm::vec3 _rotation; // Euler angles (radians)
    glm::vec3 _frontDir;
    glm::vec3 _upDir;

    // Projection parameters
    EProjectionType _projectionType;

    // Perspective parameters
    float _fovY;
    float _aspectRatio;

    // Orthographic parameters
    float _left;
    float _right;
    float _bottom;
    float _top;

    // Common parameters
    float _nearZ;
    float _farZ;

    float _moveSpeed;
    float _rotateSpeed;

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

HS_NS_END

#endif // __HS_CAMERA_H__
