//
//  TypeId.h
//  Core
//

#ifndef __HS_CORE_TYPEID_H__
#define __HS_CORE_TYPEID_H__

#include "Precompile.h"
#include "Core/Hash.h"

HS_NS_BEGIN

using TypeId = uint64;

HS_NS_END

#define HS_GENERATE_TYPEID(ClassName)                                                       \
    static constexpr const char* GetStaticTypeName() { return #ClassName; }                 \
    static constexpr hs::TypeId GetStaticTypeId() { return hs::StringHash64(#ClassName); }

#define HS_GENERATE_REFLECTION(ClassName)                                                   \
    HS_GENERATE_TYPEID(ClassName)                                                           \
    const char* GetTypeName() const override { return GetStaticTypeName(); }                \
    hs::TypeId GetTypeId() const override { return GetStaticTypeId(); }

#endif
