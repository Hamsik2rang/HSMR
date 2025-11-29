//
//  RHIDefinition
//  HSMR
//
//  Created by Yongsik Im on 2/6/25.
//
#ifndef __HS_RHI_DEFINITION_H__
#define __HS_RHI_DEFINITION_H__

#include "Precompile.h"

#include "Core/Log.h"
#include "Core/Hash.h"

#include <vector>
#include <string>

namespace hs { struct NativeWindow; }
namespace hs { class Swapchain; }

HS_NS_BEGIN

class HS_API RHIHandle
{
public:
	enum class EType
	{
		SWAPCHAIN,
		BUFFER,
		TEXTURE,
		SAMPLER,
		SHADER,
		RESOURCE_LAYOUT,
		RESOURCE_SET,
		RESOURCE_SET_POOL,
		RENDER_PASS,
		FRAMEBUFFER,
		GRAPHICS_PIPELINE,
		COMPUTE_PIPELINE,
		COMMAND_QUEUE,
		COMMAND_POOL,
		COMMAND_BUFFER,
	};

	RHIHandle() = delete;
	RHIHandle(RHIHandle::EType type, const char* name)
		: _type(type)
		, name(name)
	{}

	// RAII: Virtual destructor calls Release() automatically
	virtual ~RHIHandle() 
	{
		//// Only cleanup if we're the last reference
		//if (_refs == 1)
		//{
		//	delete this;
		//}
	}

	// Copy constructor - increase reference count
	RHIHandle(const RHIHandle& other) = delete;  // Disable copy to prevent issues
	
	// Move constructor - transfer ownership
	RHIHandle(RHIHandle&& other) noexcept
		: _type(other._type)
		, name(other.name)
		, _refs(other._refs)
		, _hash(other._hash)
	{
		other._refs = 0;  // Moved-from object should not trigger destruction
	}

	// Move assignment
	RHIHandle& operator=(RHIHandle&& other) noexcept
	{
		if (this != &other)
		{
			// Release current resource
			if (_refs == 1)
			{
				delete this;
			}
			
			// Transfer from other
			_type = other._type;
			name = other.name;
			_refs = other._refs;
			_hash = other._hash;
			other._refs = 0;
		}
		return *this;
	}

	HS_FORCEINLINE RHIHandle::EType GetType() const { return _type; }
	HS_FORCEINLINE uint32           GetHash() const { return _hash; }
	HS_FORCEINLINE void GetName(const char* name) { this->name = name; }
	
	// For external reference counting when needed
	HS_FORCEINLINE int Retain()
	{
		return ++_refs;
	}

	HS_FORCEINLINE int Release()
	{
		HS_ASSERT(_refs > 0, "Over Released!");
		
		if (--_refs == 0)
		{
			delete this;
			return 0;
		}
		return _refs;
	}

	HS_FORCEINLINE int GetRefCount() const { return _refs; }
	HS_FORCEINLINE bool IsValid() const { return _refs > 0; }

	const char* name;

protected:
	// Pure virtual method for platform-specific resource cleanup
	//virtual void OnDestroy() = 0;

	EType _type;
	int    _refs = 1;      // Start with 1 reference
	uint32 _hash = 0;
};

enum class ERHIPlatform
{
	INVALID = 0,
	VULKAN,
	METAL,
	//DIRECTX12,
	//OPENGL,
	//OPENGL_ES,
	//WEBGPU,
	VIRTUAL,
};

enum class EVertexFormat
{
    INVALID, 
    
	FLOAT,
	FLOAT2,
	FLOAT3,
	FLOAT4,

	HALF,
	HALF2,
	HALF3,
	HALF4,

	MAT2x2,
	MAT2x3,
	MAT2x4,

	MAT3x2,
	MAT3x3,
	MAT3x4,

	MAT4x2,
	MAT4x3,
	MAT4x4
};

enum class EPixelFormat
{
	INVALID = 0,

	R8_UNORM = 10,
	RG8_UNORM = 30,
	R8G8B8A8_UNORM = 70,
	R8G8B8A8_SRGB = 71,
	B8G8A8R8_UNORM = 80,
	B8G8A8R8_SRGB = 81,

	DEPTH32 = 252,
	STENCIL8 = 253,
	DEPTH24_STENCIL8 = 255,
	DEPTH32_STENCIL8 = 260,
};

enum class ETextureType
{
	INVALID = 0,

	TEX_1D,
	TEX_1D_ARRAY,
	TEX_2D,
	TEX_2D_ARRAY,
	TEX_CUBE,
};

enum class ETextureUsage : uint16
{
	UNKNOWN = 0x0000,
	STATIC = 0x0001,
	STAGING = 0x0002,
	SAMPLED = 0x0004,
	STORAGE = 0x0008,
	COLOR_ATTACHMENT = 0x0010,
	DEPTH_STENCIL_ATTACHMENT = 0x0020,
	TRANSIENT_ATTACHMENT = 0x0040,
	INPUT_ATTACHMENT = 0x0080,
};

HS_FORCEINLINE ETextureUsage operator|(ETextureUsage lhs, ETextureUsage rhs)
{
	return static_cast<ETextureUsage>(static_cast<uint32>(lhs) | static_cast<uint32>(rhs));
}

HS_FORCEINLINE ETextureUsage operator|=(ETextureUsage& lhs, ETextureUsage rhs)
{
	lhs = lhs | rhs;
	return lhs;
}

HS_FORCEINLINE ETextureUsage operator&(ETextureUsage lhs, ETextureUsage rhs)
{
	return static_cast<ETextureUsage>(static_cast<uint32>(lhs) & static_cast<uint32>(rhs));
}

HS_FORCEINLINE ETextureUsage operator&=(ETextureUsage& lhs, ETextureUsage rhs)
{
	lhs =  lhs & rhs;
	return lhs;
}

HS_FORCEINLINE bool operator!=(ETextureUsage lhs, uint16 rhs)
{
	return (false == (static_cast<uint16>(lhs) == rhs));
}

struct TextureInfo
{
	EPixelFormat  format = EPixelFormat::R8G8B8A8_UNORM;
	ETextureType  type = ETextureType::TEX_2D;
	ETextureUsage usage = ETextureUsage::UNKNOWN;
	struct
	{
		uint32 width = 0;
		uint32 height = 0;
		uint32 depth = 1;
	} extent;

	uint32 mipLevel = 1;
	uint32 arrayLength = 1;
	size_t byteSize = 0;

	bool isCompressed = false;
	bool isSwapchainTexture = false;
	Swapchain* swapchain = nullptr;
	bool isDepthStencilBuffer = false;
	bool useGenerateMipmap = false;
};

enum class EFilterMode
{
	INVALID = 0,

	NEAREST,
	LINEAR
};

enum class EAddressMode
{
	INVALID = 0,

	REPEAT,
	MIRRORED_REPEAT,
	CLAMP_TO_EDGE,
	CLAMP_TO_BORDER,
	MIRROR_CLAMP_TO_EDGE
};

struct SamplerInfo
{
	ETextureType type;
	EFilterMode  minFilter;
	EFilterMode  magFilter;
	EFilterMode mipmapMode;
	EAddressMode addressU;
	EAddressMode addressV;
	EAddressMode addressW;

	bool isPixelCoordinate = false;
};

struct RHIBuffer;

enum class EBufferUsage
{
	INVALID = 0,

	UNIFORM = 0x00000010,
	STORAGE_BUFFER = 0x00000020,
	INDEX = 0x00000040,
	VERTEX = 0x00000080,
	TEXEL = 0x00000004,
};

HS_FORCEINLINE EBufferUsage operator|(EBufferUsage lhs, EBufferUsage rhs)
{
	return static_cast<EBufferUsage>(static_cast<uint32>(lhs) | static_cast<uint32>(rhs));
}

HS_FORCEINLINE EBufferUsage operator|=(EBufferUsage& lhs, EBufferUsage rhs)
{
	lhs = static_cast<EBufferUsage>(static_cast<uint32>(lhs) | static_cast<uint32>(rhs));
	return lhs;
}

HS_FORCEINLINE EBufferUsage operator&(EBufferUsage lhs, EBufferUsage rhs)
{
	return static_cast<EBufferUsage>(static_cast<uint32>(lhs) & static_cast<uint32>(rhs));
}

HS_FORCEINLINE EBufferUsage operator&=(EBufferUsage& lhs, uint32 rhs)
{
	lhs = static_cast<EBufferUsage>(static_cast<uint32>(lhs) & rhs);
	return lhs;
}

enum class EBufferMemoryOption
{
	INVALID = 0,

	NOTHING,
	MAPPED,
	STATIC,
	DYNAMIC
};

struct BufferInfo
{
	EBufferUsage        usage;
	EBufferMemoryOption memoryOption;
};

struct RHITexture;
struct RHIRenderPass;
struct RHIResourceLayout;

struct RenderTexture
{
	EPixelFormat format;
	uint32       width;
	uint32       height;

	std::vector<RHITexture*> colorBuffers;
	RHITexture* depthStencilBuffer;
};

struct SwapchainInfo
{
	bool useDepth;
	bool useStencil;
	bool useMSAA;
	bool enableVSync = true;

	const NativeWindow* nativeWindow;
};

enum class EStoreAction
{
	INVALID = 0,

	DONT_CARE,
	STORE,
};

enum class ELoadAction
{
	INVALID = 0,

	DONT_CARE,
	LOAD,
	CLEAR,
};

struct ClearValue
{
	ClearValue() = default;
	ClearValue(float r, float g, float b, float a)
		: color{ r, g, b, a }
	{}
	ClearValue(float depth, float stencil)
		: depthStencil{ depth, stencil }
	{}

	union
	{
		float color[4]{};
		struct
		{
			float depth;
			float stencil;
		}depthStencil;
	};
};

struct Area
{
	Area() : x(0), y(0), width(1), height(1) {}
	Area(uint32 x, uint32 y, uint32 width, uint32 height)
		: x(x), y(y), width(width), height(height)
	{}

	uint32 x;
	uint32 y;
	uint32 width;
	uint32 height;
};

struct Attachment
{
	EPixelFormat	format;
	ELoadAction		loadAction;
	EStoreAction	storeAction;
	ClearValue		clearValue;
	uint8			sampleCount;
	bool			isDepthStencil = false;
};

struct RenderPassInfo
{
	std::vector<Attachment> colorAttachments;
	//std::vector<Attachment> resolveColorAttachments; // TODO: Resolve Color Attachments
	Attachment              depthStencilAttachment;
	uint8					colorAttachmentCount;

	bool useDepthStencilAttachment = false;
	bool isSwapchainRenderPass = false;
};

class RHIRenderPass;

struct FramebufferInfo
{
	RHIRenderPass* renderPass;
	std::vector<RHITexture*> colorBuffers;
	RHITexture* depthStencilBuffer;
	RHITexture* resolveBuffer;

	uint32 width = 1;
	uint32 height = 1;

	bool isSwapchainFramebuffer = false;
};

enum class EShaderParameterType
{
	T_BOOL = 0,
	T_CHAR,
	T_INT8,
	T_UINT8,
	T_INT16,
	T_UINT16,
	T_INT32,
	T_UINT32,
	T_INT64,
	T_UINT64,

	T_HALF,
	T_FLOAT,
	T_DOUBLE,

	T_VEC2,
	T_VEC3,
	T_VEC4,

	T_MAT22,
	T_MAT33,
	T_MAT44,

	T_STRUCT // Constant Buffer
	//,..
};

struct ShaderParameterValue
{
	EShaderParameterType type;

	void* data = nullptr;
	uint8 offset;
	uint8 align;
};

struct Viewport
{
	float x = 0;
	float y = 0;
	float width = 0;
	float height = 0;
	float zNear = 0.0f;
	float zFar = 1.0f;
};

enum class ERHIVertexElementType
{
	Float,
	Float2,
	Float3,
	Float4,
	Int,
	Int2,
	Int3,
	Int4,
	UInt,
	UInt2,
	UInt3,
	UInt4
};

struct VertexAttribute
{
	ERHIVertexElementType type;
	uint32 offset;
	uint32 size;
	std::string name;
	uint32 location;
};

enum class EShaderLanguage
{
	INVALID = 0,
	SPIRV,
	MSL,
	HLSL,
	//GLSL,
};;

#ifdef DOMAIN
#pragma push_macro("DOMAIN")
#undef DOMAIN
#endif

enum class EShaderStage
{
	NONE = 0x00000000,
	VERTEX = 0x00000001,
	DOMAIN = 0x00000002,
	HULL = 0x00000004,
	GEOMETRY = 0x00000008,
	FRAGMENT = 0x00000010,

	COMPUTE = 0x00000020
};

#ifdef DOMAIN
#pragma pop_macro("DOMAIN")
#endif

HS_FORCEINLINE EShaderStage operator|(EShaderStage lhs, EShaderStage rhs)
{
	return static_cast<EShaderStage>(static_cast<uint32>(lhs) | static_cast<uint32>(rhs));
}

HS_FORCEINLINE EShaderStage operator|=(EShaderStage& lhs, EShaderStage rhs)
{
	lhs = static_cast<EShaderStage>(static_cast<uint32>(lhs) | static_cast<uint32>(rhs));
	return lhs;
}

HS_FORCEINLINE EShaderStage operator&(EShaderStage lhs, EShaderStage rhs)
{
	return static_cast<EShaderStage>(static_cast<uint32>(lhs) & static_cast<uint32>(rhs));
}

HS_FORCEINLINE EShaderStage operator&=(EShaderStage& lhs, uint32 rhs)
{
	lhs = static_cast<EShaderStage>(static_cast<uint32>(lhs) & rhs);
	return lhs;
}

HS_FORCEINLINE bool operator==(EShaderStage lhs, EShaderStage rhs)
{
	return (static_cast<uint32>(lhs) == static_cast<uint32>(rhs));
}

struct ShaderInfo
{
	EShaderStage stage;
	const char* entryName;
};

enum class EResourceType : uint8
{
	SAMPLER,
	COMBINED_IMAGE_SAMPLER,
	SAMPLED_IMAGE,
	STORAGE_IMAGE,
	UNIFORM_TEXEL_BUFFER,
	STORAGET_TEXEL_BUFFER,
	UNIFORM_BUFFER,
	STORAGE_BUFFER,
	UNIFORM_BUFFER_DYNAMIC,
	STORAGE_BUFFER_DYNAMIC,
	INPUT_ATTACHMENT,
};

struct RHISampler;

struct ResourceBinding
{
	struct Resource
	{
		std::vector<RHIBuffer*>  buffers;
		std::vector<RHITexture*> textures;
		std::vector<RHISampler*> samplers;

		std::vector<uint32> offsets;
	};

	EResourceType type;
	EShaderStage  stage;
	uint8         binding;
	uint8         arrayCount;
	Resource      resource;

	std::string name;
	uint32      nameHash;
};

class RHIShader;


struct ShaderProgramDescriptor
{
	std::vector<RHIShader*> stages;
};

struct VertexInputLayoutDescriptor
{
	uint32 binding; // Metal에서는 무시됩니다.
	uint32 stride;
	uint8  stepRate : 7;
	bool   useInstancing : 1;
};

struct VertexInputAttributeDescriptor
{
	uint32 location;
	uint32 binding; // Metal에서는 무시됩니다.
	EVertexFormat format;
	uint32 offset;
};

struct VertexInputStateDescriptor
{
	std::vector<VertexInputLayoutDescriptor>    layouts;
	std::vector<VertexInputAttributeDescriptor> attributes;
};

enum class EPrimitiveTopology
{
	POINT_LIST,
	LINE_LIST,
	LINE_STRIP,
	TRIANGLE_LIST,
	TRIANGLE_STRIP,
	TRIANGLE_FAN,
	LINE_LIST_WITH_ADJACENCY,
	LINE_STRIP_WITH_ADJACENCY,
	TRIANGLE_LIST_WITH_ADJACENCY,
	TRIANGLE_STRIP_WITH_ADJACENCY,
	PATCH_LIST
};

struct InputAssemblyStateDescriptor
{
	EPrimitiveTopology primitiveTopology;
	bool isRestartEnable = false;
};

enum class ELogicOp
{
	CLEAR = 0,
	AND = 1,
	AND_REVERSE = 2,
	COPY = 3,
	AND_INVERTED = 4,
	NO_OP = 5,
	XOR = 6,
	OR = 7,
	NOR = 8,
	EQUIVALENT = 9,
	INVERT = 10,
	OR_REVERSE = 11,
	COPY_INVERTED = 12,
	OR_INVERTED = 13,
	NAND = 14,
	SET = 15,
};

enum class EBlendFactor
{
	ZERO = 0,
	ONE = 1,
	SRC_COLOR = 2,
	ONE_MINUS_SRC_COLOR = 3,
	DST_COLOR = 4,
	ONE_MINUS_DST_COLOR = 5,
	SRC_ALPHA = 6,
	ONE_MINUS_SRC_ALPHA = 7,
	DST_ALPHA = 8,
	ONE_MINUS_DST_ALPHA = 9,

	SRC_ALPHA_SATURATE = 14,
	SRC1_COLOR = 15,
	ONE_MINUS_SRC1_COLOR = 16,
	SRC1_ALPHA = 17,
	ONE_MINUS_SRC1_ALPHA = 18,

	INVALID = 0xFF
};

enum class EBlendOp
{
	ADD = 0,
	SUBTRACT = 1,
	REVERSE_SUBTRACT = 2,
	MIN = 3,
	MAX = 4,

	INVALID = 0xFF
};

struct ColorBlendAttachmentDescriptor
{
	bool blendEnable;

	EBlendFactor srcColorFactor;
	EBlendFactor dstColorFactor;
	EBlendOp     colorBlendOp;

	EBlendFactor srcAlphaFactor;
	EBlendFactor dstAlphaFactor;
	EBlendOp     alphaBlendOp;

	uint32 writeMask = 0x0000'000F;
};

struct ColorBlendStateDescriptor
{
	bool                                        logicOpEnable;
	ELogicOp                                    blendLogic;
	uint32                                      attachmentCount;
	std::vector<ColorBlendAttachmentDescriptor> attachments;
	float                                       blendConstants[4];
};

enum class EPolygonMode
{
	FILL,
	LINE,
	POINT,
};

enum class ECullMode
{
	NONE = 0x0,
	FRONT = 0x1,
	BACK = 0x2,
	ALL = 0x3
};

enum class EFrontFace
{
	COUNTER_CLOCKWISE = 0,
	CCW = COUNTER_CLOCKWISE,
	CLOCKWISE = 1,
	CW = CLOCKWISE,
};

struct RasterizerStateDescriptor
{
	bool			depthClampEnable;
	bool			rasterizerDiscardEnable;
	EPolygonMode	polygonMode;
	ECullMode		cullMode;
	EFrontFace		frontFace;
	bool			depthBiasEnable;
	float			depthBias;
	float			depthBiasClamp;
	float			depthBiasSlope;
	float			depthBiasConstant; // Metal에서는 무시됩니다.
	float			lineWidth;
};

struct MultiSampleStateDescriptor
{
	// TODO: 멀티샘플링 지원 추가
	//...
};

enum class ECompareOp
{
	NEVER,
	LESS,
	EQUAL,
	LESS_OR_EQUAL,
	GREATER,
	NOT_EQUAL,
	GREATER_OR_EQUAL,
	ALWAYS,
};

enum class EStencilOp
{
	KEEP,
	ZERO,
	REPLACE,
	INCREMENT_AND_CLAMP,
	DECREMENT_AND_CLAMP,
	INVERT,
	INCREMENT_AND_WARP,
	DECREMENT_AND_WRAP
};

struct StencilTestDescriptor
{
	EStencilOp failOp;
	EStencilOp passOp;
	EStencilOp depthFailOp;
	ECompareOp compareOp;
	uint32     compareMask;
	uint32     writeMask;
	uint32     reference;
};

struct DepthStencilStateDescriptor
{
	bool                  depthTestEnable;
	bool                  depthWriteEnable;
	ECompareOp            depthCompareOp;
	bool                  depthBoundTestEnable;
	bool                  stencilTestEnable;
	StencilTestDescriptor stencilFront;
	StencilTestDescriptor stencilBack;
	float                 minDepthBound;
	float                 maxDepthBound;
};

struct TesellationStateDescriptor
{
	uint32 patchControlPoints = 0;
};

struct GraphicsPipelineInfo
{
	//...
	ShaderProgramDescriptor			shaderDesc;
	InputAssemblyStateDescriptor	inputAssemblyDesc;
	TesellationStateDescriptor		tesellationDesc;
	VertexInputStateDescriptor		vertexInputDesc;
	RasterizerStateDescriptor		rasterizerDesc;
	MultiSampleStateDescriptor		multisampleDesc;
	DepthStencilStateDescriptor		depthStencilDesc;
	ColorBlendStateDescriptor		colorBlendDesc;

	RHIResourceLayout* resourceLayout;
	RHIRenderPass* renderPass;
};

struct ComputePipelineInfo
{
	//...
};

template <>
struct hs::Hasher<Attachment>
{
	static uint32 Get(const Attachment& key)
	{
		uint32 hash = HashCombine(Hasher<EPixelFormat>::Get(key.format), Hasher<ELoadAction>::Get(key.loadAction), Hasher<EStoreAction>::Get(key.storeAction));
		hash = HashCombine(hash, key.isDepthStencil);

		return hash;
	}
};

template <>
struct hs::Hasher<RenderPassInfo>
{
	static uint32 Get(const RenderPassInfo& key)
	{
		uint32 hash = HashCombine(Hasher<uint64>::Get(key.colorAttachmentCount), key.useDepthStencilAttachment, key.isSwapchainRenderPass);
		for (size_t i = 0; i < key.colorAttachmentCount / 2; i += 2)
		{
			hash = HashCombine(hash, Hasher<Attachment>::Get(key.colorAttachments[i]), Hasher<Attachment>::Get(key.colorAttachments[i + 1]));
		}

		uint32 b = (key.colorAttachmentCount % 2 != 0) ? Hasher<Attachment>::Get(key.colorAttachments.back()) : 0;
		uint32 c = (key.useDepthStencilAttachment) ? Hasher<Attachment>::Get(key.depthStencilAttachment) : 0;
		hash = HashCombine(hash, b, c);
		return hash;
	}
};

template <>
struct Hasher<TextureInfo>
{
	static uint32 Get(const TextureInfo& key)
	{
		uint32 hash = 0;
		hash = HashCombine(key.extent.width, key.extent.height, key.extent.depth);
		hash = HashCombine(hash, Hasher<EPixelFormat>::Get(key.format), Hasher<ETextureType>::Get(key.type));
		hash = HashCombine(hash, Hasher<ETextureUsage>::Get(key.usage), key.mipLevel);
		hash = HashCombine(hash, key.arrayLength, Hasher<size_t>::Get(key.byteSize));

		hash = HashCombine(hash, key.isCompressed, key.isSwapchainTexture);
		hash = HashCombine(hash, key.isDepthStencilBuffer, key.useGenerateMipmap);

		return hash;
	}
};

template <>
struct Hasher<SamplerInfo>
{
	static uint32 Get(const SamplerInfo& key)
	{
		uint32 hash = 0;
		hash = HashCombine(static_cast<uint32>(key.type), static_cast<uint32>(key.minFilter), static_cast<uint32>(key.magFilter));
		hash = HashCombine(hash, static_cast<uint32>(key.mipmapMode), static_cast<uint32>(key.addressU));
		hash = HashCombine(hash, static_cast<uint32>(key.addressV), static_cast<uint32>(key.addressW));
		hash = HashCombine(hash, key.isPixelCoordinate);

		return hash;
	}
};

template <>
struct Hasher<BufferInfo>
{
	static uint32 Get(const BufferInfo& key)
	{
		uint32 hash = 0;
		hash = HashCombine(static_cast<uint32>(key.usage), static_cast<uint32>(key.memoryOption));
		return hash;
	}
};

HS_NS_END

#endif
