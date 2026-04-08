//
//  CameraUtils.h
//  HSMR
//
//  Shared utility for building PerView data from TransformComponent + CameraComponent.
//  Both EditorCamera and ECS game cameras use this path.
//
#ifndef __HS_CAMERA_UTILS_H__
#define __HS_CAMERA_UTILS_H__

#include "Precompile.h"
#include "Core/Math/Common.h"
#include "Renderer/RenderDefinition.h"
#include "Scene/Components/TransformComponent.h"
#include "Scene/Components/CameraComponent.h"

HS_NS_BEGIN

struct CameraUtils
{
    // Compute view matrix from TransformComponent (LH convention)
    static glm::mat4 ComputeViewMatrixLH(const TransformComponent& transform)
    {
        glm::vec3 position = glm::vec3(transform.worldMatrix[3]);
        glm::vec3 front = glm::normalize(glm::mat3(transform.worldMatrix) * glm::vec3(0.0f, 0.0f, 1.0f));
        glm::vec3 up = glm::normalize(glm::mat3(transform.worldMatrix) * glm::vec3(0.0f, 1.0f, 0.0f));
        return glm::lookAtLH(position, position + front, up);
    }

    // Compute projection matrix from CameraComponent (LH convention, with Vulkan Y-flip)
    static glm::mat4 ComputeProjectionMatrixLH(const CameraComponent& camera, bool vulkanYFlip)
    {
        glm::mat4 proj;
        if (camera.projectionType == CameraComponent::EProjectionType::Perspective)
        {
            proj = glm::perspectiveLH(glm::radians(camera.fov), camera.aspectRatio,
                                      camera.nearPlane, camera.farPlane);
        }
        else
        {
            float halfWidth = camera.orthoSize * camera.aspectRatio;
            float halfHeight = camera.orthoSize;
            proj = glm::orthoLH(-halfWidth, halfWidth, -halfHeight, halfHeight,
                                camera.nearPlane, camera.farPlane);
        }

        if (vulkanYFlip)
        {
            proj[1][1] *= -1;
        }

        return proj;
    }

    // Build complete PerView data from components
    static PerView BuildPerViewData(const TransformComponent& transform,
                                    const CameraComponent& camera,
                                    bool vulkanYFlip)
    {
        glm::mat4 viewMatrix = ComputeViewMatrixLH(transform);
        glm::mat4 projectionMatrix = ComputeProjectionMatrixLH(camera, vulkanYFlip);
        glm::mat4 viewProjectionMatrix = projectionMatrix * viewMatrix;

        PerView perView{};
        perView.viewMatrix = viewMatrix;
        perView.projectionMatrix = projectionMatrix;
        perView.viewProjectionMatrix = viewProjectionMatrix;
        perView.inverseViewMatrix = glm::inverse(viewMatrix);
        perView.inverseProjectionMatrix = glm::inverse(projectionMatrix);
        perView.inverseViewProjectionMatrix = glm::inverse(viewProjectionMatrix);
        perView.cameraPosition = glm::vec3(transform.worldMatrix[3]);

        return perView;
    }
};

HS_NS_END

#endif // __HS_CAMERA_UTILS_H__
