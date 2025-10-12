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
	: _position(0.0f, 1.5f, 5.0f)
	, _rotation(0.0f, 0.0f, 0.0f)
	, _front(0.0f, 0.0f, -1.0f)
	, _projectionType(EProjectionType::PERSPECTIVE)
	, _fovY(glm::radians(60.0f))
	, _aspectRatio(16.0f / 9.0f)
	, _nearZ(0.1f)
	, _farZ(1000.0f)
	, _left(-10.0f)
	, _right(10.0f)
	, _bottom(-10.0f)
	, _top(10.0f)
	, _viewDirty(true)
	, _projectionDirty(true)
{
	updateViewMatrix();
	updateProjectionMatrix();
	updateViewProjectionMatrix();
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

glm::vec3 EditorCamera::ScreenToWorldPoint(const glm::vec3& screenPos) const
{
	glm::vec4 screenHPos= glm::vec4(screenPos, 1.0f);
	glm::vec4 world = _inverseViewProjectionMatrix * screenHPos;

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
	glm::vec3 ndcSpacePos = clipSpacePos / clipSpacePos.w;

	return glm::vec2(ndcSpacePos.x, ndcSpacePos.y);
}

void EditorCamera::updateViewMatrix()
{
	if (_viewDirty)
	{
		_viewMatrix = glm::lookAt(_position, _position + _front, GetUp());
		_inverseViewMatrix = glm::inverse(_viewMatrix);
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
		_inverseProjectionMatrix = glm::inverse(_projectionMatrix);
		_projectionDirty = false;
	}
}

void EditorCamera::updateViewProjectionMatrix()
{
	if (_viewDirty || _projectionDirty)
	{
		_viewProjectionMatrix = _projectionMatrix * _viewMatrix;
		_inverseViewProjectionMatrix = glm::inverse(_viewProjectionMatrix);
	}
}

HS_NS_EDITOR_END