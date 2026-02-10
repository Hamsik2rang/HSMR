//
//  MeshComponent.h
//  HSMR
//

#pragma once

#include "Precompile.h"

HS_NS_BEGIN

class Mesh;

/**
 * @brief 메시 렌더링 컴포넌트
 *
 * 렌더링할 메시와 서브메시 인덱스를 참조합니다.
 */
struct HS_API MeshComponent
{
    Mesh* mesh = nullptr;
    uint32 submeshIndex = 0;
    bool castShadow = true;
    bool receiveShadow = true;

    MeshComponent() = default;
    MeshComponent(Mesh* meshPtr, uint32 subIdx = 0)
        : mesh(meshPtr)
        , submeshIndex(subIdx)
    {}
};

HS_NS_END
