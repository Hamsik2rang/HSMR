//
//  MousePicker.cpp
//  HSMR
//
//  Created for lightweight prototyping framework
//
#include "MousePicker.h"
#include <cfloat>

HS_NS_BEGIN

Ray MousePicker::ScreenToWorldRay(Camera* camera, float screenX, float screenY) const
{
    if (!camera) return Ray();

    // Convert screen coordinates to NDC (-1 to 1)
    float ndcX = (2.0f * screenX / static_cast<float>(_screenWidth)) - 1.0f;
    float ndcY = 1.0f - (2.0f * screenY / static_cast<float>(_screenHeight));

    // Clip space coordinates for near and far planes
    glm::vec4 clipNear(ndcX, ndcY, -1.0f, 1.0f);
    glm::vec4 clipFar(ndcX, ndcY, 1.0f, 1.0f);

    // Transform to world space
    glm::mat4 invVP = camera->GetInverseViewProjectionMatrix();

    glm::vec4 worldNear = invVP * clipNear;
    glm::vec4 worldFar = invVP * clipFar;

    // Perspective divide
    if (worldNear.w != 0.0f) worldNear /= worldNear.w;
    if (worldFar.w != 0.0f) worldFar /= worldFar.w;

    Ray ray;
    ray.origin = glm::vec3(worldNear);
    ray.direction = glm::normalize(glm::vec3(worldFar - worldNear));

    return ray;
}

bool MousePicker::RayIntersectsAABB(const Ray& ray, const AABB& bounds, float& outDistance)
{
    // Slab method for ray-AABB intersection
    float tmin = 0.0f;
    float tmax = FLT_MAX;

    for (int i = 0; i < 3; ++i)
    {
        float invDir = 1.0f / ray.direction[i];

        float t1 = (bounds.min[i] - ray.origin[i]) * invDir;
        float t2 = (bounds.max[i] - ray.origin[i]) * invDir;

        if (invDir < 0.0f)
        {
            std::swap(t1, t2);
        }

        tmin = glm::max(tmin, t1);
        tmax = glm::min(tmax, t2);

        if (tmin > tmax)
        {
            return false;
        }
    }

    outDistance = tmin;
    return true;
}

bool MousePicker::RayIntersectsSphere(const Ray& ray, const glm::vec3& center, float radius, float& outDistance)
{
    glm::vec3 oc = ray.origin - center;

    float a = glm::dot(ray.direction, ray.direction);
    float b = 2.0f * glm::dot(oc, ray.direction);
    float c = glm::dot(oc, oc) - radius * radius;

    float discriminant = b * b - 4.0f * a * c;

    if (discriminant < 0.0f)
    {
        return false;
    }

    float sqrtD = sqrt(discriminant);
    float t1 = (-b - sqrtD) / (2.0f * a);
    float t2 = (-b + sqrtD) / (2.0f * a);

    // Return the closest positive intersection
    if (t1 > 0.0f)
    {
        outDistance = t1;
        return true;
    }
    if (t2 > 0.0f)
    {
        outDistance = t2;
        return true;
    }

    return false;
}

bool MousePicker::RayIntersectsPlane(const Ray& ray, const glm::vec3& planeNormal, const glm::vec3& planePoint, float& outDistance)
{
    float denom = glm::dot(planeNormal, ray.direction);

    if (abs(denom) < 1e-6f)
    {
        return false; // Ray is parallel to plane
    }

    float t = glm::dot(planePoint - ray.origin, planeNormal) / denom;

    if (t >= 0.0f)
    {
        outDistance = t;
        return true;
    }

    return false;
}

SceneObject* MousePicker::PickObject(Camera* camera, Scene* scene, float screenX, float screenY)
{
    float distance;
    return PickObject(camera, scene, screenX, screenY, distance);
}

SceneObject* MousePicker::PickObject(Camera* camera, Scene* scene, float screenX, float screenY, float& outDistance)
{
    if (!camera || !scene) return nullptr;

    // Generate ray from screen position
    _lastRay = ScreenToWorldRay(camera, screenX, screenY);

    SceneObject* closest = nullptr;
    float closestDist = FLT_MAX;

    // Test all objects in scene
    auto& objects = scene->GetObjects();
    for (auto& obj : objects)
    {
        if (!obj.IsVisible()) continue;

        float dist;
        AABB worldBounds = const_cast<SceneObject&>(obj).GetWorldBounds();

        if (RayIntersectsAABB(_lastRay, worldBounds, dist))
        {
            if (dist < closestDist && dist >= 0.0f)
            {
                closestDist = dist;
                closest = &obj;
            }
        }
    }

    outDistance = closestDist;
    return closest;
}

HS_NS_END
