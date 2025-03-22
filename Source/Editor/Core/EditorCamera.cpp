//
//  Camera.cpp
//  HSMR
//
//  Created on 3/22/25.
//
#include "Engine/Core/Camera.h"
#include "Engine/Core/Log.h"
#include "Engine/Math/HSMath.h"
#include <cmath>

HS_NS_EDITOR_BEGIN

Camera::Camera(const std::string& name)
    : Object(EType::UNKNOWN)  // 실제 구현에서는 적절한 타입을 지정해야 합니다.
    , _position(0.0f, 0.0f, 0.0f)
    , _rotation(0.0f, 0.0f, 0.0f)
    , _scale(1.0f, 1.0f, 1.0f)
    , _projectionType(EProjectionType::PERSPECTIVE)
    , _fovY(Math::PI / 4.0f)  // 45도
    , _aspectRatio(16.0f / 9.0f)
    , _left(-1.0f)
    , _right(1.0f)
    , _bottom(-1.0f)
    , _top(1.0f)
    , _nearZ(0.1f)
    , _farZ(1000.0f)
    , _enableFrustumCulling(true)
    , _viewDirty(true)
    , _projectionDirty(true)
    , _viewProjectionDirty(true)
    , _inverseDirty(true)
    , _frustumDirty(true)
{
    this->name = name;
    
    // 단위 행렬로 초기화
    _viewMatrix = Matrix4x4::Identity;
    _projectionMatrix = Matrix4x4::Identity;
    _viewProjectionMatrix = Matrix4x4::Identity;
    _inverseViewMatrix = Matrix4x4::Identity;
    _inverseProjectionMatrix = Matrix4x4::Identity;
    _inverseViewProjectionMatrix = Matrix4x4::Identity;
}

Camera::~Camera()
{
    // 필요한 정리 작업
}

void Camera::SetPosition(const simd::float3& position)
{
    _position = position;
    _viewDirty = true;
    _viewProjectionDirty = true;
    _inverseDirty = true;
    _frustumDirty = true;
}

void Camera::Move(const simd::float3& offset)
{
    _position += offset;
    _viewDirty = true;
    _viewProjectionDirty = true;
    _inverseDirty = true;
    _frustumDirty = true;
}

void Camera::Rotate(const simd::float3& eulerAngles)
{
    _rotation += eulerAngles;
    _viewDirty = true;
    _viewProjectionDirty = true;
    _inverseDirty = true;
    _frustumDirty = true;
}

void Camera::SetPerspective(float fovY, float aspectRatio, float nearZ, float farZ)
{
    _projectionType = EProjectionType::PERSPECTIVE;
    _fovY = fovY;
    _aspectRatio = aspectRatio;
    _nearZ = nearZ;
    _farZ = farZ;
    
    _projectionDirty = true;
    _viewProjectionDirty = true;
    _inverseDirty = true;
    _frustumDirty = true;
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
    _viewProjectionDirty = true;
    _inverseDirty = true;
    _frustumDirty = true;
}

bool Camera::IsInFrustum(const simd::float3& point, float radius) const
{
    if (!_enableFrustumCulling)
        return true;
    
    // 프러스텀의 6개 평면을 이용한 구체 컬링
    for (int i = 0; i < 6; i++)
    {
        // 평면의 방정식 Ax + By + Cz + D = 0에서
        // 거리는 (Ax + By + Cz + D) / sqrt(A² + B² + C²)
        // 정규화된 평면이면 분모는 1
        const simd::float4& plane = _frustumPlanes[i];
        simd::float3 normal(plane.x, plane.y, plane.z);
        float distance = simd::dot(normal, point) + plane.w;
        
        // 구체가 평면의 외부에 완전히 있다면 보이지 않음
        if (distance < -radius)
            return false;
    }
    
    return true;
}

const simd::float4x4& Camera::GetViewMatrix()
{
    if (_viewDirty)
    {
        updateViewMatrix();
    }
    return _viewMatrix;
}

const simd::float4x4& Camera::GetProjectionMatrix()
{
    if (_projectionDirty)
    {
        updateProjectionMatrix();
    }
    return _projectionMatrix;
}

const simd::float4x4& Camera::GetViewProjectionMatrix()
{
    if (_viewDirty || _projectionDirty || _viewProjectionDirty)
    {
        if (_viewDirty)
            updateViewMatrix();
        
        if (_projectionDirty)
            updateProjectionMatrix();
        
        updateViewProjectionMatrix();
    }
    return _viewProjectionMatrix;
}

const simd::float4x4& Camera::GetInverseViewMatrix()
{
    if (_viewDirty || _inverseDirty)
    {
        if (_viewDirty)
            updateViewMatrix();
        
        updateInverseMatrices();
    }
    return _inverseViewMatrix;
}

const simd::float4x4& Camera::GetInverseProjectionMatrix()
{
    if (_projectionDirty || _inverseDirty)
    {
        if (_projectionDirty)
            updateProjectionMatrix();
        
        updateInverseMatrices();
    }
    return _inverseProjectionMatrix;
}

const simd::float4x4& Camera::GetInverseViewProjectionMatrix()
{
    if (_viewDirty || _projectionDirty || _viewProjectionDirty || _inverseDirty)
    {
        if (_viewDirty || _projectionDirty || _viewProjectionDirty)
            GetViewProjectionMatrix();
        
        updateInverseMatrices();
    }
    return _inverseViewProjectionMatrix;
}

simd::float3 Camera::ScreenToWorldPoint(const simd::float2& screenPos, float depth) const
{
    // 화면 좌표계를 NDC로 변환 (화면 좌표계는 좌상단이 0,0, 우하단이 width,height)
    // screenPos는 0~1 범위로 정규화된 좌표여야 함
    simd::float4 ndcPos(
        screenPos.x * 2.0f - 1.0f,  // -1 ~ 1
        (1.0f - screenPos.y) * 2.0f - 1.0f,  // -1 ~ 1 (Y축 뒤집기)
        depth,  // 0 ~ 1
        1.0f
    );
    
    // NDC 좌표에서 월드 좌표로 변환
    simd::float4 worldPos = simd::matrix_multiply(_inverseViewProjectionMatrix, ndcPos);
    
    // w로 나누어 동차 좌표 처리
    if (worldPos.w != 0.0f)
    {
        worldPos.x /= worldPos.w;
        worldPos.y /= worldPos.w;
        worldPos.z /= worldPos.w;
    }
    
    return simd::float3(worldPos.x, worldPos.y, worldPos.z);
}

simd::float3 Camera::ScreenToWorldDirection(const simd::float2& screenPos) const
{
    // 화면 좌표에서 가까운 평면과 먼 평면의 월드 좌표를 구함
    simd::float3 nearPoint = ScreenToWorldPoint(screenPos, 0.0f);
    simd::float3 farPoint = ScreenToWorldPoint(screenPos, 1.0f);
    
    // 방향 벡터 계산 및 정규화
    return simd::normalize(farPoint - nearPoint);
}

simd::float2 Camera::WorldToScreenPoint(const simd::float3& worldPos) const
{
    // 월드 좌표를 클립 공간으로 변환
    simd::float4 clipPos = simd::matrix_multiply(_viewProjectionMatrix, simd::float4(worldPos.x, worldPos.y, worldPos.z, 1.0f));
    
    // 동차 나눗셈
    if (clipPos.w != 0.0f)
    {
        clipPos.x /= clipPos.w;
        clipPos.y /= clipPos.w;
        clipPos.z /= clipPos.w;
    }
    
    // NDC 좌표를 화면 좌표로 변환 (0~1 범위)
    simd::float2 screenPos(
        (clipPos.x + 1.0f) * 0.5f,
        (1.0f - (clipPos.y + 1.0f) * 0.5f)  // Y축 뒤집기
    );
    
    return screenPos;
}

void Camera::updateViewMatrix()
{
    // 회전 행렬 계산 (오일러 각도에서)
    float cosX = std::cos(_rotation.x);
    float sinX = std::sin(_rotation.x);
    float cosY = std::cos(_rotation.y);
    float sinY = std::sin(_rotation.y);
    float cosZ = std::cos(_rotation.z);
    float sinZ = std::sin(_rotation.z);
    
    // 회전 행렬 (Z * X * Y 순서로 적용)
    simd::float4x4 rotMatrix = {
        simd::float4(cosY * cosZ, sinX * sinY * cosZ + cosX * sinZ, -cosX * sinY * cosZ + sinX * sinZ, 0.0f),
        simd::float4(-cosY * sinZ, -sinX * sinY * sinZ + cosX * cosZ, cosX * sinY * sinZ + sinX * cosZ, 0.0f),
        simd::float4(sinY, -sinX * cosY, cosX * cosY, 0.0f),
        simd::float4(0.0f, 0.0f, 0.0f, 1.0f)
    };
    
    // 이동 행렬
    simd::float4x4 translationMatrix = simd::matrix_identity_float4x4;
    translationMatrix.columns[3] = simd::float4(-_position.x, -_position.y, -_position.z, 1.0f);
    
    // 뷰 행렬 = 회전 행렬 * 이동 행렬
    _viewMatrix = simd::matrix_multiply(rotMatrix, translationMatrix);
    _viewDirty = false;
    
    // 의존성 있는 캐시 갱신 플래그 설정
    _viewProjectionDirty = true;
    _inverseDirty = true;
    _frustumDirty = true;
}

void Camera::updateProjectionMatrix()
{
    if (_projectionType == EProjectionType::PERSPECTIVE)
    {
        // 원근 투영 행렬 생성 (Metal 좌표계에 맞춤)
        float tanHalfFovY = std::tan(_fovY * 0.5f);
        
        _projectionMatrix = simd::matrix_identity_float4x4;
        _projectionMatrix.columns[0].x = 1.0f / (tanHalfFovY * _aspectRatio);
        _projectionMatrix.columns[1].y = 1.0f / tanHalfFovY;
        _projectionMatrix.columns[2].z = _farZ / (_farZ - _nearZ);
        _projectionMatrix.columns[2].w = 1.0f;
        _projectionMatrix.columns[3].z = -(_nearZ * _farZ) / (_farZ - _nearZ);
        _projectionMatrix.columns[3].w = 0.0f;
    }
    else  // ORTHOGRAPHIC
    {
        // 직교 투영 행렬 생성
        float width = _right - _left;
        float height = _top - _bottom;
        float depth = _farZ - _nearZ;
        
        _projectionMatrix = simd::matrix_identity_float4x4;
        _projectionMatrix.columns[0].x = 2.0f / width;
        _projectionMatrix.columns[1].y = 2.0f / height;
        _projectionMatrix.columns[2].z = 1.0f / depth;
        _projectionMatrix.columns[3].x = -(_right + _left) / width;
        _projectionMatrix.columns[3].y = -(_top + _bottom) / height;
        _projectionMatrix.columns[3].z = -_nearZ / depth;
    }
    
    _projectionDirty = false;
    
    // 의존성 있는 캐시 갱신 플래그 설정
    _viewProjectionDirty = true;
    _inverseDirty = true;
    _frustumDirty = true;
}

void Camera::updateViewProjectionMatrix()
{
    _viewProjectionMatrix = simd::matrix_multiply(_projectionMatrix, _viewMatrix);
    _viewProjectionDirty = false;
    
    // 프러스텀 평면 갱신
    if (_frustumDirty)
    {
        updateFrustumPlanes();
    }
}

void Camera::updateInverseMatrices()
{
    // 역행렬 계산
    _inverseViewMatrix = simd::matrix_invert(_viewMatrix);
    _inverseProjectionMatrix = simd::matrix_invert(_projectionMatrix);
    _inverseViewProjectionMatrix = simd::matrix_invert(_viewProjectionMatrix);
    
    _inverseDirty = false;
}

void Camera::updateFrustumPlanes()
{
    // 뷰-투영 행렬에서 프러스텀 평면 추출
    const simd::float4& row0 = _viewProjectionMatrix.columns[0];
    const simd::float4& row1 = _viewProjectionMatrix.columns[1];
    const simd::float4& row2 = _viewProjectionMatrix.columns[2];
    const simd::float4& row3 = _viewProjectionMatrix.columns[3];
    
    // 왼쪽 평면 (A,B,C,D 순서)
    _frustumPlanes[0] = row3 + row0;
    
    // 오른쪽 평면
    _frustumPlanes[1] = row3 - row0;
    
    // 아래쪽 평면
    _frustumPlanes[2] = row3 + row1;
    
    // 위쪽 평면
    _frustumPlanes[3] = row3 - row1;
    
    // 가까운 평면
    _frustumPlanes[4] = row3 + row2;
    
    // 먼 평면
    _frustumPlanes[5] = row3 - row2;
    
    // 평면 정규화
    for (int i = 0; i < 6; i++)
    {
        simd::float3 normal(
            _frustumPlanes[i].x,
            _frustumPlanes[i].y,
            _frustumPlanes[i].z
        );
        float length = simd::length(normal);
        
        if (length > 0.0f)
        {
            _frustumPlanes[i] = _frustumPlanes[i] / length;
        }
    }
    
    _frustumDirty = false;
}
    _viewProjectionDirty = true;
    _inverseDirty = true;
    _frustumDirty = true;
}

void Camera::SetRotation(const simd::float3& rotation)
{
    _rotation = rotation;
    _viewDirty = true;
    _viewProjectionDirty = true;
    _inverseDirty = true;
    _frustumDirty = true;
}

void Camera::SetScale(const simd::float3& scale)
{
    _scale = scale;
    _viewDirty = true;
    _viewProjectionDirty = true;
    _inverseDirty = true;
    _frustumDirty = true;
}

simd::float3 Camera::GetForward() const
{
    // 오일러 각도에서 전방 벡터 계산
    float yaw = _rotation.y;
    float pitch = _rotation.x;
    
    float cosYaw = std::cos(yaw);
    float sinYaw = std::sin(yaw);
    float cosPitch = std::cos(pitch);
    float sinPitch = std::sin(pitch);
    
    return simd::normalize(simd::float3(
        sinYaw * cosPitch,
        sinPitch,
        cosYaw * cosPitch
    ));
}

simd::float3 Camera::GetRight() const
{
    // 오일러 각도에서 우측 벡터 계산
    float yaw = _rotation.y;
    
    return simd::normalize(simd::float3(
        std::cos(yaw),
        0.0f,
        -std::sin(yaw)
    ));
}

simd::float3 Camera::GetUp() const
{
    // 전방과 우측 벡터의 외적으로 상향 벡터 계산
    return simd::normalize(simd::cross(GetForward(), GetRight()));
}

void Camera::LookAt(const simd::float3& target, const simd::float3& up)
{
    // 대상을 바라보는 방향 계산
    simd::float3 direction = simd::normalize(target - _position);
    
    // Y축 회전 (Yaw)
    float yaw = std::atan2(direction.x, direction.z);
    
    // X축 회전 (Pitch)
    float pitch = std::asin(direction.y);
    
    // 새로운 회전 설정 (Z축 회전은 유지)
    _rotation = simd::float3(pitch, yaw, _rotation.z);
    
    _viewDirty = true;

HS_NS_EDITOR_END
