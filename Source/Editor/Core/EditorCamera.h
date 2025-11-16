#ifndef __HS_EDITOR_CAMERA_H__
#define __HS_EDITOR_CAMERA_H__

#include "Precompile.h"

#include "Core/Flag.h"
#include "Core/Math/Common.h"

#include "Engine/Resource/Object.h"


HS_NS_EDITOR_BEGIN

// 카메라 클래스: 씬을 비추는 뷰 정의
class EditorCamera
{
public:
	enum class EProjectionType
	{
		PERSPECTIVE = 0,
		ORTHOGRAPHIC = 1,
	};

	enum class Dirty
	{
		POSITION,
		ROTATE,
		SCALE,
		VIEW,
		PROJECTION,
	};

	EditorCamera();
	~EditorCamera() = default;

	// 기본 변환 설정
	HS_FORCEINLINE void SetPosition(const glm::vec3& position) { _position = position; }
	HS_FORCEINLINE void SetRotation(const glm::vec3& rotation) { _rotation = rotation; }

	HS_FORCEINLINE glm::vec3 GetPosition() const { return _position; }
	HS_FORCEINLINE glm::vec3 GetRotation() const { return _rotation; }

	// 카메라 방향 벡터 얻기
	HS_FORCEINLINE glm::vec3 GetForward() const { return _frontDir; }
	HS_FORCEINLINE glm::vec3 GetUp() const { return  _upDir; }
	HS_FORCEINLINE glm::vec3 GetRight() const { return glm::normalize(glm::cross(_frontDir, _upDir)); }

	// 카메라 조작 편의 함수들
	HS_FORCEINLINE void Move(const glm::vec3& offset) { _position += offset; }
	HS_FORCEINLINE void Rotate(const glm::vec3& eulerAngles) { _rotation += eulerAngles; }

	// 투영 타입과 파라미터 설정
	HS_FORCEINLINE void SetProjectionType(EProjectionType type)
	{
		_projectionType = type;
		_projectionDirty = true;
	}
	HS_FORCEINLINE EProjectionType GetProjectionType() const { return _projectionType; }

	// 원근 투영 파라미터
	void SetPerspective(float fovY, float aspectRatio, float nearZ, float farZ);

	// 직교 투영 파라미터
	void SetOrthographic(float left, float right, float bottom, float top, float nearZ, float farZ);

	HS_FORCEINLINE float GetFov() const { return _fovY; }
	HS_FORCEINLINE void  SetFov(float fovY)
	{
		_fovY = fovY;
		_projectionDirty = true;
	}
	HS_FORCEINLINE float GetAspectRatio() const { return _aspectRatio; }
	HS_FORCEINLINE void SetAspectRatio(float aspectRatio)
	{
		_aspectRatio = aspectRatio;
		_projectionDirty = true;
	}
	// 공통 파라미터
	HS_FORCEINLINE float GetNearZ() const { return _nearZ; }
	HS_FORCEINLINE void SetNearZ(float nearZ)
	{
		_nearZ = nearZ;
		_projectionDirty = true;
	}
	HS_FORCEINLINE float GetFarZ() const { return _farZ; }
	HS_FORCEINLINE void SetFarZ(float farZ)
	{
		_farZ = farZ;
		_projectionDirty = true;
	}

	// TODO: 컬링 기능
	//HS_FORCEINLINE void SetFrustumCulling(bool enable) { _enableFrustumCulling = enable; }
	//HS_FORCEINLINE bool IsFrustumCulling() const { return _enableFrustumCulling; }
	//HS_FORCEINLINE bool IsInFrustum(const glm::vec3& point, float radius = 0.0f) const;

	// 행렬 계산과 접근
	HS_FORCEINLINE const glm::mat4& GetViewMatrix() { return _viewMatrix; }
	HS_FORCEINLINE const glm::mat4& GetProjectionMatrix() { return _projectionMatrix; }
	HS_FORCEINLINE const glm::mat4& GetViewProjectionMatrix() { return _viewProjectionMatrix; }

	// 인버스 행렬
	HS_FORCEINLINE const glm::mat4& GetInverseViewMatrix() { return _inverseViewMatrix; }
	HS_FORCEINLINE const glm::mat4& GetInverseProjectionMatrix() { return _inverseProjectionMatrix; }
	HS_FORCEINLINE const glm::mat4& GetInverseViewProjectionMatrix() { return _inverseViewProjectionMatrix; }

	glm::vec3 ScreenToWorldPoint(const glm::vec3& screenPos) const;
	glm::vec3 ScreenToWorldDirection(const glm::vec3& screenDir) const;
	glm::vec2 WorldToScreenPoint(const glm::vec3& worldPos) const;

private:
	// 변환 업데이트
	void updateViewMatrix();
	void updateProjectionMatrix();
	void updateViewProjectionMatrix();
	//void updateFrustumPlanes();

	// 트랜스폼 데이터
	glm::vec3 _position;
	glm::vec3 _rotation; // 오일러 각 (라디안)
	glm::vec3 _frontDir;
	glm::vec3 _upDir;

	// 투영 파라미터
	EProjectionType _projectionType;

	// 원근 투영 파라미터
	float _fovY;
	float _aspectRatio;

	// 직교 투영 파라미터
	float _left;
	float _right;
	float _bottom;
	float _top;

	// 공통 파라미터
	float _nearZ;
	float _farZ;

	// 행렬 캐싱
	glm::mat4 _viewMatrix;
	glm::mat4 _projectionMatrix;
	glm::mat4 _viewProjectionMatrix;

	glm::mat4 _inverseViewMatrix;
	glm::mat4 _inverseProjectionMatrix;
	glm::mat4 _inverseViewProjectionMatrix;

	// TODO: 프러스텀 컬링 데이터
	//bool _enableFrustumCulling;
	//glm::vec4 _frustumPlanes[6]; // 왼쪽, 오른쪽, 위, 아래, 가까운면, 먼면

	// 캐시 상태 플래그
	bool _viewDirty;
	bool _projectionDirty;
};

HS_NS_EDITOR_END

#endif // __HS_EDITOR_CAMERA_H__
