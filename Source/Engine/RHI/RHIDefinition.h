//
//  RHIDefinition
//  HSMR
//
//  Created by Yongsik Im on 2/6/25.
//
#ifndef __HS_RHI_DEFINITION_H__
#define __HS_RHI_DEFINITION_H__

#include "Precompile.h"

#include "Engine/Core/Log.h"
#include "Engine/Utility/Hash.h"

#include <vector>
#include <string>

namespace HS { struct NativeWindow; }
namespace HS { class Swapchain;}

HS_NS_BEGIN

class RHIHandle
{
public:
	enum class EType
	{
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
		FENCE,
		SEMAPHORE,
		RESOURCE_BARRIER
	};

	RHIHandle() = delete;
	RHIHandle(RHIHandle::EType type)
		: _type(type)
	{}
	virtual ~RHIHandle() {}

	RHIHandle::EType GetType() const { return _type; }
	uint32           GetHash() const { return _hash; }
	int              Retain()
	{
		return ++_refs;
	}

	int Release()
	{
		if (_refs <= 0)
		{
			HS_LOG(crash, "Over Released");
		}

		if (--_refs == 0)
		{
			// add pending delete list
		}

		return _refs;
	}

	std::string name;

protected:
	const EType _type;

	int    _refs = 1; // Create한 순간 자동으로 Ratain하는 것으로 판단.
	uint32 _hash;
	//...
};

enum class EVertexFormat
{
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
	SHADER_READ = 0x0001,
	SHADER_WRITE = 0x0002,
	RENDER_TARGET = 0x0004,
	PIXELFORMAT_VIEW = 0x0010,
};

HS_FORCEINLINE ETextureUsage operator|(ETextureUsage lhs, ETextureUsage rhs)
{
	return static_cast<ETextureUsage>(static_cast<uint32>(lhs) | static_cast<uint32>(rhs));
}

HS_FORCEINLINE ETextureUsage operator|=(ETextureUsage lhs, ETextureUsage rhs)
{
	return static_cast<ETextureUsage>(static_cast<uint32>(lhs) | static_cast<uint32>(rhs));
}

HS_FORCEINLINE ETextureUsage operator&(ETextureUsage lhs, ETextureUsage rhs)
{
	return static_cast<ETextureUsage>(static_cast<uint32>(lhs) & static_cast<uint32>(rhs));
}

HS_FORCEINLINE bool operator!=(ETextureUsage lhs, uint16 rhs)
{
	return (static_cast<uint16>(lhs) & rhs) != 0;
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
};

struct SamplerInfo
{
	ETextureType type;
	EFilterMode  minFilter;
	EFilterMode  magFilter;
	EFilterMode  mipFilter;
	EAddressMode addressU;
	EAddressMode addressV;
	EAddressMode addressW;

	bool isPixelCoordinate = false;
};

struct Buffer;

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

struct Texture;
struct RenderPass;

struct RenderTexture
{
	EPixelFormat format;
	uint32       width;
	uint32       height;

	std::vector<Texture*> colorBuffers;
	Texture* depthStencilBuffer;
};

struct SwapchainInfo
{
	bool useDepth;
	bool useStencil;
	bool useMSAA;

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
	uint32 x;
	uint32 y;
	uint32 width;
	uint32 height;
};

struct Attachment
{
	EPixelFormat format;
	ELoadAction  loadAction;
	EStoreAction storeAction;
	ClearValue   clearValue;
	bool         isDepthStencil = false;
};

struct RenderPassInfo
{
	std::vector<Attachment> colorAttachments;
	Attachment              depthStencilAttachment;
	uint8					colorAttachmentCount;

	Area renderArea;
	bool useDepthStencilAttachment = false;
	bool isSwapchainRenderPass = false;
};

class RenderPass;

struct FramebufferInfo
{
	RenderPass* renderPass;
	Texture* const* colorBuffers;
	Texture* depthStencilBuffer;
	Texture* resolveBuffer;

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

#ifdef DOMAIN
#pragma push_macro("DOMAIN")
#undef DOMAIN
#endif
enum class EShaderStage
{
	VERTEX,
	GEOMETRY,
	DOMAIN,
	HULL,
	FRAGMENT,

	COMPUTE
};

#ifdef DOMAIN
#pragma pop_macro("DOMAIN")
#endif

struct ShaderInfo
{
	EShaderStage stage;
	const char* entryName;

	bool isBuiltIn = true;
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

struct Sampler;

struct ResourceBinding
{
	struct Resource
	{
		std::vector<Buffer*>  buffers;
		std::vector<Texture*> textures;
		std::vector<Sampler*> samplers;

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

class Shader;

struct ShaderProgramDescriptor
{
	Shader* vertexShader = nullptr;
	Shader* geometryShader = nullptr;
	Shader* domainShader = nullptr;
	Shader* hullShader = nullptr;
	Shader* fragmentShader = nullptr;

	Shader* computeShader = nullptr;
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
	uint32 formatSize;
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
	// bool isRestartEnable = false;
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

	uint32 writeMask; // 0x FF(R) FF(G) FF(B) FF(A)
};

struct ColorBlendStateDescriptor
{
	bool                                        logicOpEnable;
	ELogicOp                                    blendLogic;
	uint32                                      attachmentCount;
	std::vector<ColorBlendAttachmentDescriptor> attachments;
	float                                       blendConestant[4];
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
	bool         depthClampEnable;
	bool         rasterizerDiscardEnable;
	EPolygonMode polygonMode;
	ECullMode    cullMode;
	EFrontFace   frontFace;
	bool         depthBiasEnable;
	float        depthBias;
	float        depthBiasClamp;
	float        depthBiasSlope;
	float        lineWidth;
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
	ShaderProgramDescriptor      shaderDesc;
	InputAssemblyStateDescriptor inputAssemblyDesc;
	TesellationStateDescriptor   tesellationDesc;
	VertexInputStateDescriptor   vertexInputDesc;
	RasterizerStateDescriptor    rasterizerDesc;
	MultiSampleStateDescriptor   multisampleDesc;
	DepthStencilStateDescriptor  depthStencilDesc;
	ColorBlendStateDescriptor    colorBlendDesc;

	RenderPass* renderPass;
};

struct ComputePipelineInfo
{
	//...
};

template <>
struct Hasher<Attachment>
{
	static uint32 Get(const Attachment& key)
	{
		uint32 hash = HashCombine(Hasher<EPixelFormat>::Get(key.format), Hasher<ELoadAction>::Get(key.loadAction), Hasher<EStoreAction>::Get(key.storeAction));
		hash = HashCombine(hash, key.isDepthStencil);

		return hash;
	}
};

template <>
struct Hasher<RenderPassInfo>
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
		hash = HashCombine(hash, static_cast<uint32>(key.mipFilter), static_cast<uint32>(key.addressU));
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
