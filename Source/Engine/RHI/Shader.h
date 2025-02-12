//
//  Shader.h
//  HSMR
//
//  Created by Yongsik Im on 2/6/25.
//

#ifndef __HS_SHADER_H__
#define __HS_SHADER_H__

#include "Precompile.h"

#include "Engine/RHI/RenderDefinition.h"

HS_NS_BEGIN

class Shader
{
public:
    Shader(EShaderStage stage, const char* path, const char* entryName, bool isBuiltIn = true);
    Shader(EShaderStage stage, const char* byteCocde, size_t byteSize, const char* entryName, bool isBuitIn = true);

    virtual ~Shader();

    const char*  GetByteCode() { return _byteCode; }
    const char*  GetEntryName() { return _entryName; }
    size_t       GetByteSize() { return _byteSize; }
    EShaderStage GetStage() { return _stage; }

private:
    void loadShader();

    bool         _isBuiltIn;
    const char*  _byteCode;
    const char*  _entryName;
    size_t       _byteSize;
    EShaderStage _stage;
};

HS_NS_END

#endif /* Shader_h */
