//
//  Hash.h
//  Core
//
//  Created by Yongsik Im on 2/23/2025.
//

#ifndef __HS_HASH_H__
#define __HS_HASH_H__

#include "Precompile.h"

HS_NS_BEGIN
// ref from https://gist.github.com/badboy/6267743.

// 해당 탬플릿 구조체를 컾마일 타임 정의함으로써 객체에 대한 해시 값을 획득하는 방법을 정의합니다.
template <typename T>
struct Hasher
{
    template <typename U = T, typename std::enable_if<!std::is_enum<U>::value>::type* = nullptr>
    static uint32 Get(const U& key);

    template <typename U = T, typename std::enable_if<std::is_enum<U>::value>::type* = nullptr>
    static uint32 Get(const U& key)
    {
        return static_cast<uint32>(key);
    }
};

template <>
struct Hasher<uint64>
{
    static uint32 Get(const uint64& key)
    {
        uint64 hash = key;

        hash = (~hash) + (hash << 18); // key = (key << 18) - key - 1;
        hash = hash ^ (hash >> 31);
        hash = hash * 21; // key = (key + (key << 2)) + (key << 4);
        hash = hash ^ (hash >> 11);
        hash = hash + (hash << 6);
        hash = hash ^ (hash >> 22);
        return static_cast<uint32>(hash);
    }
};

template <>
struct Hasher<unsigned long>
{
    static uint32 Get(const unsigned long& key)
    {
        unsigned long hash = key;

        hash = (~hash) + (hash << 18); // key = (key << 18) - key - 1;
        hash = hash ^ (hash >> 31);
        hash = hash * 21; // key = (key + (key << 2)) + (key << 4);
        hash = hash ^ (hash >> 11);
        hash = hash + (hash << 6);
        hash = hash ^ (hash >> 22);
        return static_cast<uint32>(hash);
    }
};

template <>
struct Hasher<bool>
{
    static uint32 Get(const bool& key)
    {
        return static_cast<uint32>(key);
    }
};

template<>
struct Hasher<char>
{
    static uint32 Get(const char& key)
    {
        return static_cast<uint32>(key);
    }
};

template<>
struct Hasher<unsigned char>
{
    static uint32 Get(const unsigned char& key)
    {
        return static_cast<uint32>(key);
    }
};

template<>
struct Hasher<int16>
{
  static uint32 Get(const int16& key)
    {
      return static_cast<uint32>(key);
  }
};

template<>
struct Hasher<uint16>
{
    static uint32 Get(const uint16& key)
    {
        return static_cast<uint32>(key);
    }
};

HS_FORCEINLINE uint32 HashCombine(uint32 a, uint32 b, uint32 c)
{
    a += b;
    a -= b;
    a -= c;
    a ^= (c >> 13);
    b -= c;
    b -= a;
    b ^= (a << 8);
    c -= a;
    c -= b;
    c ^= (b >> 13);
    a -= b;
    a -= c;
    a ^= (c >> 12);
    b -= c;
    b -= a;
    b ^= (a << 16);
    c -= a;
    c -= b;
    c ^= (b >> 5);
    a -= b;
    a -= c;
    a ^= (c >> 3);
    b -= c;
    b -= a;
    b ^= (a << 10);
    c -= a;
    c -= b;
    c ^= (b >> 15);

    return c;
}

HS_FORCEINLINE uint32 HashCombine(uint32 a, uint32 c)
{
    // uint64 b = 0x9e3779b97f4a7c55;
    uint32 b = 0x9e3779b9;
    return HashCombine(a, b, c);
}

HS_FORCEINLINE uint32 PointerHash(const void* p, uint32 a)
{
    uint32 key = Hasher<uint64>::Get(reinterpret_cast<uint64>(p));
    return HashCombine(a, key);
}

HS_FORCEINLINE uint32 PointerHash(const void* p)
{
    uint32 hash = Hasher<uint64>::Get(reinterpret_cast<uint64>(p));
    return hash;
}

HS_NS_END

#endif /* __HS_HASH_H__ */
