//
//  Object.h
//  HSMR
//
//  Created by Yongsik Im on 2/4/25.
//
#ifndef __HS_OBJECT_H__
#define __HS_OBJECT_H__

#include "Precompile.h"
#include "Engine/Core/Log.h"

HS_NS_BEGIN

class Referencable
{
public:
    int Retain()
    {
        return ++_refCount;
    }

    int Release()
    {
        int newRef = _refCount - 1;
        HS_ASSERT(newRef >= 0, "Over released!");
        _refCount--;

        return newRef;
    }

private:
    int _refCount = 0;
};

class Object : public Referencable
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

    Object(EType type)
        : _type(type)
    {}
    virtual ~Object() = default;

    Object::EType GetType() const { return _type; }

    const char* name;

protected:
    EType _type;
};

HS_NS_END

#endif
