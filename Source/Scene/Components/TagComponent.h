//
//  TagComponent.h
//  HSMR
//

#pragma once

#include "Precompile.h"
#include <string>

HS_NS_BEGIN

/**
 * @brief 태그/이름 컴포넌트
 *
 * Entity의 이름, 레이어, static 여부 등 메타데이터를 저장합니다.
 */
struct HS_SCENE_API TagComponent
{
    std::string name = "Entity";
    uint32 layer = 0;
    bool isStatic = false;
    bool isActive = true;

    TagComponent() = default;
    TagComponent(const std::string& entityName)
        : name(entityName)
    {}
    TagComponent(const char* entityName)
        : name(entityName)
    {}
};

HS_NS_END
