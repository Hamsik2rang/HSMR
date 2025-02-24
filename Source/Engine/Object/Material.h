//
//  Material.h
//  HSMR
//
//  Created by Yongsik Im on 2/5/25.
//
#ifndef __HS_MATERIAL_H__
#define __HS_MATERIAL_H__

#include "Precompile.h"

#include "Engine/Object/Object.h"
#include "Engine/Renderer/RenderDefinition.h"

#include "MSL/BuiltInMaterialLayout.h"

HS_NS_BEGIN

class Shader;

struct BuiltInMaterialLayout
{

    
};

class Material : public Object
{
public:
    Material() : Object(EType::MATERIAL) {}
    ~Material() override;
    
    void SetShader(Shader* shader);
    void SetShaderParameter(const char* name);
    
private:
    Shader* _shader;

};


HS_NS_END


#endif
