//
//  MousePicker.h
//  HSMR
//
//  Created for lightweight prototyping framework
//
#ifndef __HS_APPLICATION_MOUSE_PICKER_H__
#define __HS_APPLICATION_MOUSE_PICKER_H__

#include "Precompile.h"
#include "Core/Math/Common.h"
#include "SceneObject.h"
#include "Camera.h"
#include "Scene.h"

HS_NS_BEGIN

// Ray structure for picking
struct HS_APPLICATION_API Ray
{
    glm::vec3 origin;
    glm::vec3 direction;

    Ray() : origin(0.0f), direction(0.0f, 0.0f, -1.0f) {}
    Ray(const glm::vec3& orig, const glm::vec3& dir)
        : origin(orig), direction(glm::normalize(dir)) {}

    // Get point along ray at distance t
    glm::vec3 GetPoint(float t) const
    {
        return origin + direction * t;
    }
};

// Mouse picker for ray-casting based object selection
class HS_APPLICATION_API MousePicker
{
public:
    MousePicker() = default;
    ~MousePicker() = default;

    // Set screen dimensions (call when window resizes)
    void SetScreenSize(uint32 width, uint32 height)
    {
        _screenWidth = width;
        _screenHeight = height;
    }

    // Convert screen coordinates to world ray
    Ray ScreenToWorldRay(Camera* camera, float screenX, float screenY) const;

    // Ray-AABB intersection test
    // Returns true if ray intersects AABB, outputs distance to intersection
    static bool RayIntersectsAABB(const Ray& ray, const AABB& bounds, float& outDistance);

    // Ray-sphere intersection test (for point-like objects)
    static bool RayIntersectsSphere(const Ray& ray, const glm::vec3& center, float radius, float& outDistance);

    // Ray-plane intersection test
    static bool RayIntersectsPlane(const Ray& ray, const glm::vec3& planeNormal, const glm::vec3& planePoint, float& outDistance);

    // Pick object from scene
    // Returns the closest object that the ray hits, or nullptr if none
    SceneObject* PickObject(Camera* camera, Scene* scene, float screenX, float screenY);

    // Pick with distance output
    SceneObject* PickObject(Camera* camera, Scene* scene, float screenX, float screenY, float& outDistance);

    // Get last ray (for debugging/visualization)
    const Ray& GetLastRay() const { return _lastRay; }

private:
    uint32 _screenWidth = 1920;
    uint32 _screenHeight = 1080;

    Ray _lastRay;
};

HS_NS_END

#endif // __HS_APPLICATION_MOUSE_PICKER_H__
