//
//  Shader.h
//  HSMR
//
//  Created by Yongsik Im on 2/6/25.
//

#ifndef __HS_SHADER_H__
#define __HS_SHADER_H__

#include "Precompile.h"

#include "Engine/Object/Object.h"

HS_NS_BEGIN

class Shader : public Object
{
public:
    enum class EStage
    {
        VERTEX,
        GEOMETRY,
        DOMAIN,
        HULL,
        FRAGMENT,

        COMPUTE
    };

    Shader(EStage stage)
        : Object(Object::EType::SHADER)
        , _byteCode(nullptr)
        , _byteSize(0)
        , _stage(stage)
    {}
    
    Shader(EStage stage, const char* path);
    Shader(EStage stage, const char* byteCocde, size_t byteSize);
    
    ~Shader() override;

    const char* GetByteCode() { return _byteCode; }
    size_t GetByteSize() { return _byteSize; }
    EStage GetStage() { return _stage; }

private:
    const char* _byteCode;
    size_t _byteSize;
    EStage _stage;
};

HS_NS_END

#endif /* Shader_h */
