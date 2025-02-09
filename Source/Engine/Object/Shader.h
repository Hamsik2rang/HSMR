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

    Shader(EStage stage, const char* path, const char* entryName, bool isBuiltIn = true);
    Shader(EStage stage, const char* byteCocde, size_t byteSize, const char* entryName, bool isBuitIn = true);

    ~Shader() override;

    const char* GetByteCode() { return _byteCode; }
    const char* GetEntryName() { return _entryName; }
    size_t      GetByteSize() { return _byteSize; }
    EStage      GetStage() { return _stage; }

private:
    void loadShader();
    
    bool        _isBuiltIn;
    const char* _byteCode;
    const char* _entryName;
    size_t      _byteSize;
    EStage      _stage;
};

HS_NS_END

#endif /* Shader_h */
