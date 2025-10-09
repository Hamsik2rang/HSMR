//
//  EditorCamera.cpp
//  HSMR
//
//  Created on 10/03/25.
//
#include "Core/Log.h"
#include <cmath>
#include "EditorCamera.h"

HS_NS_EDITOR_BEGIN


EditorCamera::EditorCamera()
{
	_position = { 0.0f, 1.5f, 5.0f };
	_rotation = { 0.0f, 0.0f, 0.0f };
	_front = { 0.0f, 0.0f, -1.0f };
}

void EditorCamera::SetPerspective(float fovY, float aspectRatio, float nearZ, float farZ)
{
	_projectionType = EProjectionType::PERSPECTIVE;
	_fovY = fovY;
	_aspectRatio = aspectRatio;
	_nearZ = nearZ;
	_farZ = farZ;

	_projectionDirty = true;
}

void EditorCamera::SetOrthographic(float left, float right, float bottom, float top, float nearZ, float farZ)
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

HS_FORCEINLINE glm::vec3 EditorCamera::ScreenToWorldPoint(const glm::vec2& screenPos, float depth) const
{
	return glm::vec3();
}

HS_FORCEINLINE glm::vec3 EditorCamera::ScreenToWorldDirection(const glm::vec2& screenPos) const
{
	return glm::vec3();
}

HS_FORCEINLINE glm::vec2 EditorCamera::WorldToScreenPoint(const glm::vec3& worldPos) const
{
	return glm::vec2();
}

void EditorCamera::updateViewMatrix()
{
	if (_viewDirty)
	{
		_viewMatrix = glm::lookAt(_position, _position + _front, GetUp());
	}
}

void EditorCamera::updateProjectionMatrix()
{
	if (_projectionDirty)
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
		_projectionDirty = false;
	}
}

void EditorCamera::updateViewProjectionMatrix()
{
	if (_viewDirty || _projectionDirty)
	{
		_viewProjectionMatrix = _projectionMatrix * _viewMatrix;
	}
}

void EditorCamera::updateInverseMatrices()
{
	_inverseViewMatrix = glm::inverse(_viewMatrix);
	_inverseProjectionMatrix = glm::inverse(_projectionMatrix);
	_inverseViewProjectionMatrix = glm::inverse(_viewProjectionMatrix);
}

HS_NS_EDITOR_END