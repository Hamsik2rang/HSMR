//
//  MaterialComponent.h
//  HSMR
//

#pragma once

#include "Precompile.h"

HS_NS_BEGIN

class Material;

/**
 * @brief 머티리얼 컴포넌트
 *
 * 렌더링에 사용할 머티리얼을 참조합니다.
 */
struct HS_API MaterialComponent
{
    Material* material = nullptr;

    MaterialComponent() = default;
    MaterialComponent(Material* mat)
        : material(mat)
    {}
};

HS_NS_END
