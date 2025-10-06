#ifndef __HS_EDITOR_CAMERA_H__
#define __HS_EDITOR_CAMERA_H__

#include "Precompile.h"
#include "Resource/Object.h"
#include "Core/Flag.h"

#include "Core/Math/Common.h"

#include "Core/Math/Vec2f.h"
#include "Core/Math/Vec3f.h"
#include "Core/Math/Vec4f.h"

#include "Core/Math/Mat4f.h"

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
	void SetPosition(const Vec3f& position);
	void SetRotation(const Vec3f& rotation); // 오일러 각
	void SetScale(const Vec3f& scale);

	Vec3f GetPosition() const { return _position; }
	Vec3f GetRotation() const { return _rotation; }
	Vec3f GetScale() const { return _scale; }

	// 카메라 방향 벡터 얻기
	Vec3f GetForward() const;
	Vec3f GetRight() const;
	Vec3f GetUp() const;

	// 카메라 조작 편의 함수들
	void LookAt(const Vec3f& target, const Vec3f& up = { 0.0f, 1.0f, 0.0f });
	void Move(const Vec3f& offset);
	void Rotate(const Vec3f& eulerAngles);

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
	bool IsInFrustum(const Vec3f& point, float radius = 0.0f) const;

	// 행렬 계산과 접근
	const Mat44f& GetViewMatrix();
	const Mat44f& GetProjectionMatrix();
	const Mat44f& GetViewProjectionMatrix();

	// 인버스 행렬
	const Mat44f& GetInverseViewMatrix();
	const Mat44f& GetInverseProjectionMatrix();
	const Mat44f& GetInverseViewProjectionMatrix();

	// 레이 캐스팅 기능
	Vec3f ScreenToWorldPoint(const Vec2f& screenPos, float depth) const;
	Vec3f ScreenToWorldDirection(const Vec2f& screenPos) const;
	Vec2f WorldToScreenPoint(const Vec3f& worldPos) const;

private:
	// 변환 업데이트
	void updateViewMatrix();
	void updateProjectionMatrix();
	void updateViewProjectionMatrix();
	void updateInverseMatrices();
	void updateFrustumPlanes();

	// 트랜스폼 데이터
	Vec3f _position;
	Vec3f _rotation; // 오일러 각 (라디안)
	Vec3f _scale;

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
	Mat44f _viewMatrix;
	Mat44f _projectionMatrix;
	Mat44f _viewProjectionMatrix;

	Mat44f _inverseViewMatrix;
	Mat44f _inverseProjectionMatrix;
	Mat44f _inverseViewProjectionMatrix;

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
