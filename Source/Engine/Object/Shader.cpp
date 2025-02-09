//
//  Shader.cpp
//  Engine
//
//  Created by Yongsik Im on 2/6/25.
//

#include "Engine/Object/Shader.h"

HS_NS_BEGIN

Shader::Shader(Shader::EStage stage, const char* path, const char* entryName, bool isBuiltIn)
    : Object(EType::SHADER)
    , _byteCode(nullptr)
    , _byteSize(0)
    , _stage(stage)
    , _entryName(entryName)
    , _isBuiltIn(isBuiltIn)
{
    loadShader();
    
}

Shader::Shader(Shader::EStage stage, const char* byteCode, size_t byteSize, const char* entryName, bool isBuiltIn)
    : Object(EType::SHADER)
    , _byteCode(byteCode)
    , _byteSize(byteSize)
    , _stage(stage)
    , _entryName(entryName)
    , _isBuiltIn(isBuiltIn)
{
    loadShader();
}

void Shader::loadShader()
{
    
}

HS_NS_END
