//
//  Mesh.h
//  HSMR
//
//  Created by Yongsik Im on 2/5/25.
//
#ifndef __HS_MESH_H__
#define __HS_MESH_H__

#include "Precompile.h"

#include "Engine/Object/Object.h"

#include "glm/glm.hpp"

HS_NS_BEGIN

class Mesh : public Object
{
public:
    Mesh() : Object(EType::MESH) {}
    ~Mesh() override;
    
    HS_FORCEINLINE void AddSubMesh(Mesh* subMesh) { _subMeshes.push_back(subMesh); }
    HS_FORCEINLINE
    
    HS_FORCEINLINE void SetPosition(std::vector<float>&& position) { _position = std::move(position); }
    HS_FORCEINLINE const std::vector<float>& GetPosition() const { return _position; }
    
private:
    std::vector<float> _position;
    std::vector<float> _texcoord[8];
    std::vector<float> _normal;
    std::vector<float> _color;
    std::vector<float> _tangent;
    std::vector<float> _bitangent;
    //... TODO: Animation
    
    std::vector<Mesh*> _subMeshes;
};

HS_NS_END

#endif
