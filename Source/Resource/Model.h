#ifndef __HS_MODEL_H__
#define __HS_MODEL_H__

#include "Precompile.h"

#include "Resource/Mesh.h"
#include "Resource/Material.h"

HS_NS_BEGIN

class HS_RESOURCE_API Model
{
public:
    Model() = default;
    Model(Scoped<Mesh>&& mesh, Scoped<Material>&& material)
        : _mesh(std::move(mesh))
        , _material(std::move(material))
    {}
    HS_FORCEINLINE Mesh* GetMesh() const { return _mesh.get(); }
    HS_FORCEINLINE void SetMesh(Scoped<Mesh>&& mesh)
    {
        _mesh        = std::move(mesh);
        _isMeshDirty = true;
    }

    HS_FORCEINLINE Material* GetMaterial() const { return _material.get(); }
    HS_FORCEINLINE void SetMaterial(Scoped<Material>&& material)
    {
        _material        = std::move(material);
        _isMaterialDirty = true;
    }

    HS_FORCEINLINE void SetPosition(const glm::vec3& position)
    {
        _position         = position;
        _isTransformDirty = true;
    }

    HS_FORCEINLINE void SetRotation(const glm::vec3& rotation)
    {
        _rotation         = rotation;
        _isTransformDirty = true;
    }

    HS_FORCEINLINE void SetScale(const glm::vec3& scale)
    {
        _scale            = scale;
        _isTransformDirty = true;
    }

    HS_FORCEINLINE const glm::mat4& GetWorldMatrix() const { return _worldMatrix; }
    HS_FORCEINLINE const glm::mat4& GetInverseWorldMatrix() const { return _inverseWorldMatrix; }

    void Update(); // Mesh, Material, Transform 등 변경 사항 업데이트

private:
    bool _isMeshDirty = false;
    bool _isMaterialDirty = false;
    bool _isTransformDirty = false;

    Scoped<Mesh> _mesh;
    Scoped<Material> _material;

    glm::vec3 _position = glm::vec3(0.0f);
    glm::vec3 _rotation = glm::vec3(0.0f);
    glm::vec3 _scale    = glm::vec3(1.0f);

    glm::mat4 _worldMatrix        = glm::mat4(1.0f);
    glm::mat4 _inverseWorldMatrix = glm::mat4(1.0f);
};

HS_NS_END
#endif
