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
HS_FORCEINLINE HS_API size_t Hash64(uint64 key)
{
	key = (~key) + (key << 18);
	key = key ^ (key >> 31);
	key = key * 21;
	key = key ^ (key >> 11);
	key = key + (key << 6);
	key = key ^ (key >> 22);
	return static_cast<size_t>(key);
}

HS_FORCEINLINE HS_API uint64 HashCombine64(uint64 a, uint64 b, uint64 c)
{
	a += b; a -= b; a -= c; a ^= (c >> 13); b -= c; b -= a; b ^= (a << 8);
	c -= a; c -= b; c ^= (b >> 13); a -= b; a -= c; a ^= (c >> 12); b -= c;
	b -= a; b ^= (a << 16); c -= a; c -= b; c ^= (b >> 5); a -= b; a -= c;
	a ^= (c >> 3); b -= c; b -= a; b ^= (a << 10); c -= a; c -= b; c ^= (b >> 15);

	return c;
}

HS_FORCEINLINE HS_API uint64 HashCombine64(uint64 a, uint64 c)
{
	uint64 b = 0x9e3779b97f4a7c55;
	return HashCombine64(a, b, c);
}

HS_FORCEINLINE HS_API uint32 HashCombine(uint32 a, uint32 b, uint32 c)
{
	a += b; a -= b; a -= c; a ^= (c >> 13); b -= c; b -= a; b ^= (a << 8);
	c -= a; c -= b; c ^= (b >> 13); a -= b; a -= c; a ^= (c >> 12); b -= c;
	b -= a; b ^= (a << 16); c -= a; c -= b; c ^= (b >> 5); a -= b; a -= c;
	a ^= (c >> 3); b -= c; b -= a; b ^= (a << 10); c -= a; c -= b; c ^= (b >> 15);

	return c;
}

HS_FORCEINLINE HS_API uint32 HashCombine(uint32 a, uint32 c)
{
	uint32 b = 0x9e3779b9;
	return HashCombine(a, b, c);
}

HS_FORCEINLINE HS_API size_t PointerHash(const void* p, size_t a)
{
	size_t key = Hash64(reinterpret_cast<uint64>(p));
	return HashCombine64(a, key);
}

HS_FORCEINLINE HS_API size_t PointerHash(const void* p)
{
	return Hash64(reinterpret_cast<uint64>(p));
}

// 64bit FNV-1a hash function
HS_FORCEINLINE HS_API uint64 StringHash64(const std::string& str)
{
	uint64 hash = 14695981039346656037ULL;
	for (char c : str)
	{
		hash ^= static_cast<uint64>(c);
		hash *= 1099511628211ULL;
	}
	return hash;
}

// 32bit FNV-1a hash function
HS_FORCEINLINE HS_API uint32 StringHash(const std::string& str)
{
	uint32 hash = 2166136261U;
	for (char c : str)
	{
		hash ^= static_cast<uint32>(c);
		hash *= 16777619U;
	}
	return hash;
}

HS_NS_END

#endif /* __HS_HASH_H__ */
