//
//  ResourceHandle.h
//  Engine
//
//  Created by Yongsik Im on 2/9/25.
//
#ifndef __HS_RESOURCE_HANDLE_H__
#define __HS_RESOURCE_HANDLE_H__

#include "Precompile.h"

#include "Engine/RHI/RHIDefinition.h"

HS_NS_BEGIN

class Texture : public RHIHandle
{
public:
	~Texture() override;

	const TextureInfo info;
protected:
	Texture(const TextureInfo& info);
};

class Sampler : public RHIHandle
{
public:
	~Sampler() override;

	const SamplerInfo info;
protected:
	Sampler(const SamplerInfo& info);
};

class Buffer : public RHIHandle
{
public:
	~Buffer() override;

	const BufferInfo info;

	void* byte;
	size_t byteSize;
protected:
	Buffer(const BufferInfo& info);
};

class Shader : public RHIHandle
{
public:
	~Shader() override;

	const ShaderInfo	info;

protected:
	Shader(const ShaderInfo& info) noexcept;
};

class ResourceLayout : public RHIHandle
{
public:
	~ResourceLayout() override;

	std::vector<ResourceBinding> bindings;

protected:
	ResourceLayout();
};

class ResourceSet : public RHIHandle
{
public:
	~ResourceSet() override;

	std::vector<ResourceLayout*> layouts;

protected:
	ResourceSet();
};

class ResourceSetPool : public RHIHandle
{
public:
	~ResourceSetPool() override;

protected:
	ResourceSetPool();
};

template <>
struct Hasher<Texture>
{
	static uint32 Get(const Texture& key)
	{
		return Hasher<TextureInfo>::Get(key.info);
	}
};

template <>
struct Hasher<Texture*>
{
	static uint32 Get(const Texture*& key)
	{
		return Hasher<TextureInfo>::Get(key->info);
	}
};

template <>
struct Hasher<Sampler>
{
	static uint32 Get(const Sampler& key)
	{
		return Hasher<SamplerInfo>::Get(key.info);
	}
};

template <>
struct Hasher<Sampler*>
{
	static uint32 Get(const Sampler*& key)
	{
		return Hasher<SamplerInfo>::Get(key->info);
	}
};

template <>
struct Hasher<Buffer>
{
	static uint32 Get(const Buffer& key)
	{
		return Hasher<BufferInfo>::Get(key.info);
	}
};

template <>
struct Hasher<Buffer*>
{
	static uint32 Get(const Buffer*& key)
	{
		return Hasher<BufferInfo>::Get(key->info);
	}
};



HS_NS_END

#endif /* __HS_RESOURCE_HANDLE_H__ */
