#ifndef __HS_RESOURCE_PROXY_H__
#define __HS_RESOURCE_PROXY_H__

#include "Precompile.h"
#include "Core/Hash.h"

HS_NS_BEGIN

class Object;

class HS_RESOURCE_API ObjectProxy
{
public:
    enum class EType
    {
        UNKNOWN,
        MESH,
        MATERIAL,
        IMAGE,
        SHADER,
    };

    ObjectProxy(EType type, uint64 gameObjectId)
        : _type(type)
        , _gameObjectId(gameObjectId)
        , _isValid(true)
        , _isDirty(true)
    {}

    virtual ~ObjectProxy()
    {
        _type = EType::UNKNOWN;
        _isValid = false;
    }

    EType GetType() const { return _type; }
    uint64 GetGameObjectId() const { return _gameObjectId; }
    bool IsValid() const { return _isValid; }
    bool IsDirty() const { return _isDirty; }
    
    void MarkClean() { _isDirty = false; }
    void MarkDirty() { _isDirty = true; }

    virtual void UpdateFromGameObject(const Object* gameObject) = 0;
    virtual void ReleaseRenderResources() = 0;

protected:
    EType _type;
    uint64 _gameObjectId;
    bool _isValid;
    bool _isDirty;
};

HS_NS_END

#endif