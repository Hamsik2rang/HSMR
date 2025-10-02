//
//  Camera.h
//  HSMR
//
//  Created on 3/22/25.
//
#ifndef __HS_EDITOR_CAMERA_H__
#define __HS_EDITOR_CAMERA_H__

#include "Precompile.h"
#include "Resource/Object.h"
#include "Core/Flag.h"

#include "glm/glm.hpp"

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
	~EditorCamera();

	// 기본 변환 설정
	void SetPosition(const glm::vec3& position);
	void SetRotation(const glm::vec3& rotation); // 오일러 각
	void SetScale(const glm::vec3& scale);

	glm::vec3 GetPosition() const { return _position; }
	glm::vec3 GetRotation() const { return _rotation; }
	glm::vec3 GetScale() const { return _scale; }

	// 카메라 방향 벡터 얻기
	glm::vec3 GetForward() const;
	glm::vec3 GetRight() const;
	glm::vec3 GetUp() const;

	// 카메라 조작 편의 함수들
	void LookAt(const glm::vec3& target, const glm::vec3& up = { 0.0f, 1.0f, 0.0f });
	void Move(const glm::vec3& offset);
	void Rotate(const glm::vec3& eulerAngles);

	// 투영 타입과 파라미터 설정
	void SetProjectionType(EProjectionType type)
	{
		_projectionType = type;
		_projectionDirty = true;
	}
	EProjectionType GetProjectionType() const { return _projectionType; }

	// 원근 투영 파라미터
	void  SetPerspective(float fovY, float aspectRatio, float nearZ, float farZ);
	float GetFov() const { return _fovY; }
	void  SetFov(float fovY)
	{
		_fovY = fovY;
		_projectionDirty = true;
	}
	float GetAspectRatio() const { return _aspectRatio; }
	void  SetAspectRatio(float aspectRatio)
	{
		_aspectRatio = aspectRatio;
		_projectionDirty = true;
	}

	// 직교 투영 파라미터
	void SetOrthographic(float left, float right, float bottom, float top, float nearZ, float farZ);

	// 공통 파라미터
	float GetNearZ() const { return _nearZ; }
	void  SetNearZ(float nearZ)
	{
		_nearZ = nearZ;
		_projectionDirty = true;
	}
	float GetFarZ() const { return _farZ; }
	void  SetFarZ(float farZ)
	{
		_farZ = farZ;
		_projectionDirty = true;
	}

	// 컬링 기능
	void SetFrustumCulling(bool enable) { _enableFrustumCulling = enable; }
	bool IsFrustumCulling() const { return _enableFrustumCulling; }
	bool IsInFrustum(const glm::vec3& point, float radius = 0.0f) const;

	// 행렬 계산과 접근
	const glm::mat4& GetViewMatrix();
	const glm::mat4& GetProjectionMatrix();
	const glm::mat4& GetViewProjectionMatrix();

	// 인버스 행렬
	const glm::mat4& GetInverseViewMatrix();
	const glm::mat4& GetInverseProjectionMatrix();
	const glm::mat4& GetInverseViewProjectionMatrix();

	// 레이 캐스팅 기능
	glm::vec3 ScreenToWorldPoint(const glm::vec2& screenPos, float depth) const;
	glm::vec3 ScreenToWorldDirection(const glm::vec2& screenPos) const;
	glm::vec2 WorldToScreenPoint(const glm::vec3& worldPos) const;

private:
	// 변환 업데이트
	void updateViewMatrix();
	void updateProjectionMatrix();
	void updateViewProjectionMatrix();
	void updateInverseMatrices();
	void updateFrustumPlanes();

	// 트랜스폼 데이터
	glm::vec3 _position;
	glm::vec3 _rotation; // 오일러 각 (라디안)
	glm::vec3 _scale;

	// 투영 파라미터
	EProjectionType _projectionType;

	// 원근 투영 파라미터
	float _fovY;        // 라디안 단위
	float _aspectRatio; // 가로 / 세로

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

	// 프러스텀 컬링 데이터
	bool    _enableFrustumCulling;
	glm::vec4 _frustumPlanes[6]; // 왼쪽, 오른쪽, 위, 아래, 가까운면, 먼면

	// 캐시 상태 플래그
	bool _viewDirty;
	bool _projectionDirty;
	bool _viewProjectionDirty;
	bool _inverseDirty;
	bool _frustumDirty;
};

HS_NS_EDITOR_END

#endif // __HS_EDITOR_CAMERA_H__
