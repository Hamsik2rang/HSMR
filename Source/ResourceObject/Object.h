//
//  Object.h
//  HSMR
//
//  Created by Yongsik Im on 2/4/25.
//
#ifndef __HS_OBJECT_H__
#define __HS_OBJECT_H__

#include "Precompile.h"
#include "Core/Log.h"

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
        , _isValid(true)
    {}
    virtual ~Object()
    {
        _type = EType::UNKNOWN;
        _isValid = false;
    }

    Object::EType GetType() const { return _type; }
    bool IsValid() const { return _isValid; }
    
    const char* name;

protected:
    EType _type;
    bool _isValid;
};

HS_NS_END

#endif
