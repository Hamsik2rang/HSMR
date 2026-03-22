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

namespace hs
{
struct NativeWindow;
}
namespace hs
{
class Swapchain;
}

HS_NS_BEGIN

class HS_RHI_API RHIHandle
{
public:
	enum class EType
	{
		Swapchain,
		Buffer,
		Texture,
		Sampler,
		Shader,
		ResourceLayout,
		ResourceSet,
		ResourceSetPool,
		RenderPass,
		Framebuffer,
		GraphicsPipeline,
		ComputePipeline,
		CommandQueue,
		CommandPool,
		CommandBuffer,
	};

	RHIHandle() = delete;
	RHIHandle(RHIHandle::EType type, const char* name)
		: _type(type)
		, _name(name)
	{}

	// RAII: Virtual destructor calls Release() automatically
	virtual ~RHIHandle()
	{
		//// Only cleanup if we're the last reference
		// if (_refs == 1)
		//{
		//	delete this;
		// }
	}

	// Copy constructor - increase reference count
	RHIHandle(const RHIHandle& other) = delete; // Disable copy to prevent issues

	// Move constructor - transfer ownership
	RHIHandle(RHIHandle&& other) noexcept
		: _type(other._type)
		, _name(other._name)
		, _refs(other._refs)
		, _hash(other._hash)
	{
		other._refs = 0; // Moved-from object should not trigger destruction
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
			_name = other._name;
			_refs = other._refs;
			_hash = other._hash;
			other._refs = 0;
		}
		return *this;
	}

	HS_FORCEINLINE RHIHandle::EType GetType() const { return _type; }
	HS_FORCEINLINE uint32 GetHash() const { return _hash; }
	HS_FORCEINLINE const char* GetName() const { return _name; }

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


protected:
	// Pure virtual method for platform-specific resource cleanup
	// virtual void OnDestroy() = 0;

	const char* _name;
	EType _type;
	int _refs = 1; // Start with 1 reference
	uint32 _hash = 0;
};

enum class ERHIPlatform
{
	Invalid = 0,
	Vulkan,
	Metal,
	// DIRECTX12,
	// OPENGL,
	// OPENGL_ES,
	// WEBGPU,
	Virtual,
};

enum class EVertexFormat
{
	Invalid,

	Float,
	Float2,
	Float3,
	Float4,

	Half,
	Half2,
	Half3,
	Half4,

	Mat2x2,
	Mat2x3,
	Mat2x4,

	Mat3x2,
	Mat3x3,
	Mat3x4,

	Mat4x2,
	Mat4x3,
	Mat4x4
};

enum class EPixelFormat
{
	Invalid = 0,

	R8Unorm = 10,
	RG8Unorm = 30,
	R8G8B8A8Unorm = 70,
	R8G8B8A8Srgb = 71,
	B8G8A8R8Unorm = 80,
	B8G8A8R8Srgb = 81,

	// Floating-point formats (for HDR, compute, atmosphere LUTs)
	R16f = 100,
	RG16f = 101,
	Rgba16f = 102,
	R32f = 110,
	RG32f = 111,
	Rgba32f = 112,

	Depth32 = 252,
	Stencil8 = 253,
	Depth24Stencil8 = 255,
	Depth32Stencil8 = 260,
};

enum class ETextureType
{
	Invalid = 0,

	Tex1D,
	Tex1DArray,
	Tex2D,
	Tex2DArray,
	TexCube,
	Tex3D,
};

enum class ETextureUsage : uint16
{
	Unknown = 0x0000,
	Static = 0x0001,
	Staging = 0x0002,
	Sampled = 0x0004,
	Storage = 0x0008,
	ColorAttachment = 0x0010,
	DepthStencilAttachment = 0x0020,
	TransientAttachment = 0x0040,
	InputAttachment = 0x0080,
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
	lhs = lhs & rhs;
	return lhs;
}

HS_FORCEINLINE bool operator!=(ETextureUsage lhs, uint16 rhs)
{
	return (false == (static_cast<uint16>(lhs) == rhs));
}

struct TextureInfo
{
	EPixelFormat format = EPixelFormat::R8G8B8A8Unorm;
	ETextureType type = ETextureType::Tex2D;
	ETextureUsage usage = ETextureUsage::Unknown;
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

	bool operator==(const TextureInfo& other) const
	{
		return format == other.format
			&& type == other.type
			&& usage == other.usage
			&& extent.width == other.extent.width
			&& extent.height == other.extent.height
			&& extent.depth == other.extent.depth
			&& mipLevel == other.mipLevel
			&& arrayLength == other.arrayLength
			&& byteSize == other.byteSize
			&& isCompressed == other.isCompressed
			&& isSwapchainTexture == other.isSwapchainTexture
			&& isDepthStencilBuffer == other.isDepthStencilBuffer
			&& useGenerateMipmap == other.useGenerateMipmap;
	}
};

enum class EFilterMode
{
	Nearest,
	Linear
};

enum class EAddressMode
{
	Invalid = 0,

	Repeat,
	MirroredRepeat,
	ClampToEdge,
	ClampToBorder,
	MirrorClampToEdge
};

struct SamplerInfo
{
	ETextureType type;
	EFilterMode minFilter;
	EFilterMode magFilter;
	EFilterMode mipmapMode;
	EAddressMode addressU;
	EAddressMode addressV;
	EAddressMode addressW;

	bool isPixelCoordinate = false;
};

struct RHIBuffer;

enum class EBufferUsage
{
	Invalid = 0,

	Uniform = 0x00000010,
	StorageBuffer = 0x00000020,
	Index = 0x00000040,
	Vertex = 0x00000080,
	Texel = 0x00000004,
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
	Invalid = 0,

	Nothing,
	Mapped,
	Static,
	Dynamic
};

struct BufferInfo
{
	EBufferUsage usage;
	EBufferMemoryOption memoryOption;

	bool operator==(const BufferInfo& other) const
	{
		return usage == other.usage
			&& memoryOption == other.memoryOption;
	}
};

struct RHITexture;
struct RHIRenderPass;
struct RHIResourceLayout;

struct RenderTexture
{
	EPixelFormat format;
	uint32 width;
	uint32 height;

	std::vector<RHITexture*> colorBuffers;
	RHITexture* depthStencilBuffer;
};

struct SwapchainInfo
{
	bool useDepth;
	bool useStencil;
	bool useMSAA;
	bool enableVSync;

	const NativeWindow* nativeWindow;
};

enum class EStoreAction
{
	Invalid = 0,

	DontCare,
	Store,
};

enum class ELoadAction
{
	Invalid = 0,

	DontCare,
	Load,
	Clear,
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
		} depthStencil;
	};
};

struct Area
{
	Area()
		: x(0)
		, y(0)
		, width(1)
		, height(1)
	{}
	Area(uint32 x, uint32 y, uint32 width, uint32 height)
		: x(x)
		, y(y)
		, width(width)
		, height(height)
	{}

	uint32 x;
	uint32 y;
	uint32 width;
	uint32 height;
};

struct Attachment
{
	EPixelFormat format;
	ELoadAction loadAction;
	EStoreAction storeAction;
	ClearValue clearValue;
	uint8 sampleCount;
	bool isDepthStencil = false;
};

struct RenderPassInfo
{
	std::vector<Attachment> colorAttachments;
	// std::vector<Attachment> resolveColorAttachments; // TODO: Resolve Color Attachments
	Attachment depthStencilAttachment;
	uint8 colorAttachmentCount;

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
	Bool = 0,
	Char,
	Int8,
	Uint8,
	Int16,
	Uint16,
	Int32,
	Uint32,
	Int64,
	Uint64,

	Half,
	Float,
	Double,

	Vec2,
	Vec3,
	Vec4,

	Mat22,
	Mat33,
	Mat44,

	Struct // Constant Buffer
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
	Invalid = 0,
	Spirv,
	Msl,
	Hlsl,
	// GLSL,
};

#ifdef DOMAIN
#pragma push_macro("DOMAIN")
#undef DOMAIN
#endif

enum class EShaderStage
{
	None = 0x00000000,
	Vertex = 0x00000001,
	Domain = 0x00000002,
	Hull = 0x00000004,
	Geometry = 0x00000008,
	Fragment = 0x00000010,

	Compute = 0x00000020
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
	Sampler,
	CombinedImageSampler,
	SampledImage,
	StorageImage,
	UniformTexelBuffer,
	StorageTexelBuffer,
	UniformBuffer,
	StorageBuffer,
	UniformBufferDynamic,
	StorageBufferDynamic,
	InputAttachment,
};

struct RHISampler;

struct ResourceBinding
{
	struct Resource
	{
		std::vector<RHIBuffer*> buffers;
		std::vector<RHITexture*> textures;
		std::vector<RHISampler*> samplers;

		std::vector<uint32> offsets;
	};

	EResourceType type;
	EShaderStage stage;
	uint8 binding;
	uint8 arrayCount;
	Resource resource;

	std::string name;
	uint32 nameHash;
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
	uint8 stepRate : 7;
	bool useInstancing : 1;
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
	std::vector<VertexInputLayoutDescriptor> layouts;
	std::vector<VertexInputAttributeDescriptor> attributes;
};

enum class EPrimitiveTopology
{
	PointList,
	LineList,
	LineStrip,
	TriangleList,
	TriangleStrip,
	TriangleFan,
	LineListWithAdjacency,
	LineStripWithAdjacency,
	TriangleListWithAdjacency,
	TriangleStripWithAdjacency,
	PatchList
};

struct InputAssemblyStateDescriptor
{
	EPrimitiveTopology primitiveTopology;
	bool isRestartEnable = false;
};

enum class ELogicOp
{
	Clear = 0,
	And = 1,
	AndReverse = 2,
	Copy = 3,
	AndInverted = 4,
	NoOp = 5,
	Xor = 6,
	Or = 7,
	Nor = 8,
	Equivalent = 9,
	Invert = 10,
	OrReverse = 11,
	CopyInverted = 12,
	OrInverted = 13,
	Nand = 14,
	Set = 15,
};

enum class EBlendFactor
{
	Zero = 0,
	One = 1,
	SrcColor = 2,
	OneMinusSrcColor = 3,
	DstColor = 4,
	OneMinusDstColor = 5,
	SrcAlpha = 6,
	OneMinusSrcAlpha = 7,
	DstAlpha = 8,
	OneMinusDstAlpha = 9,

	SrcAlphaSaturate = 14,
	Src1Color = 15,
	OneMinusSrc1Color = 16,
	Src1Alpha = 17,
	OneMinusSrc1Alpha = 18,

	Invalid = 0xFF
};

enum class EBlendOp
{
	Add = 0,
	Subtract = 1,
	ReverseSubtract = 2,
	Min = 3,
	Max = 4,

	Invalid = 0xFF
};

struct ColorBlendAttachmentDescriptor
{
	bool blendEnable;

	EBlendFactor srcColorFactor;
	EBlendFactor dstColorFactor;
	EBlendOp colorBlendOp;

	EBlendFactor srcAlphaFactor;
	EBlendFactor dstAlphaFactor;
	EBlendOp alphaBlendOp;

	uint32 writeMask = 0x0000'000F;
};

struct ColorBlendStateDescriptor
{
	bool logicOpEnable;
	ELogicOp blendLogic;
	uint32 attachmentCount;
	std::vector<ColorBlendAttachmentDescriptor> attachments;
	float blendConstants[4];
};

enum class EPolygonMode
{
	Fill,
	Line,
	Point,
};

enum class ECullMode
{
	None = 0x0,
	Front = 0x1,
	Back = 0x2,
	All = 0x3
};

enum class EFrontFace
{
	CounterClockwise = 0,
	Ccw = CounterClockwise,
	Clockwise = 1,
	Cw = Clockwise,
};

struct RasterizerStateDescriptor
{
	bool depthClampEnable;
	bool rasterizerDiscardEnable;
	EPolygonMode polygonMode;
	ECullMode cullMode;
	EFrontFace frontFace;
	bool depthBiasEnable;
	float depthBias;
	float depthBiasClamp;
	float depthBiasSlope;
	float depthBiasConstant; // Metal에서는 무시됩니다.
	float lineWidth;
};

struct MultiSampleStateDescriptor
{
	// TODO: 멀티샘플링 지원 추가
	//...
};

enum class ECompareOp
{
	Never,
	Less,
	Equal,
	LessOrEqual,
	Greater,
	NotEqual,
	GreaterOrEqual,
	Always,
};

enum class EStencilOp
{
	Keep,
	Zero,
	Replace,
	IncrementAndClamp,
	DecrementAndClamp,
	Invert,
	IncrementAndWrap,
	DecrementAndWrap
};

struct StencilTestDescriptor
{
	EStencilOp failOp;
	EStencilOp passOp;
	EStencilOp depthFailOp;
	ECompareOp compareOp;
	uint32 compareMask;
	uint32 writeMask;
	uint32 reference;
};

struct DepthStencilStateDescriptor
{
	bool depthTestEnable;
	bool depthWriteEnable;
	ECompareOp depthCompareOp;
	bool depthBoundTestEnable;
	bool stencilTestEnable;
	StencilTestDescriptor stencilFront;
	StencilTestDescriptor stencilBack;
	float minDepthBound;
	float maxDepthBound;
};

struct TesellationStateDescriptor
{
	uint32 patchControlPoints = 0;
};

struct GraphicsPipelineInfo
{
	//...
	ShaderProgramDescriptor shaderDesc;
	InputAssemblyStateDescriptor inputAssemblyDesc;
	TesellationStateDescriptor tesellationDesc;
	VertexInputStateDescriptor vertexInputDesc;
	RasterizerStateDescriptor rasterizerDesc;
	MultiSampleStateDescriptor multisampleDesc;
	DepthStencilStateDescriptor depthStencilDesc;
	ColorBlendStateDescriptor colorBlendDesc;

	RHIResourceLayout* resourceLayout;
	RHIRenderPass* renderPass;
};

struct ComputePipelineInfo
{
	RHIShader* computeShader;
	RHIResourceLayout* resourceLayout;
};

HS_NS_END

namespace std
{
template <>
struct hash<hs::Attachment>
{
	size_t operator()(const hs::Attachment& key) const
	{
		size_t h = hs::HashCombine(
			static_cast<uint32>(key.format),
			static_cast<uint32>(key.loadAction),
			static_cast<uint32>(key.storeAction));
		h = hs::HashCombine(static_cast<uint32>(h), static_cast<uint32>(key.isDepthStencil));
		return h;
	}
};

template <>
struct hash<hs::RenderPassInfo>
{
	size_t operator()(const hs::RenderPassInfo& key) const
	{
		size_t h = hs::HashCombine(
			static_cast<uint32>(key.colorAttachmentCount),
			static_cast<uint32>(key.useDepthStencilAttachment),
			static_cast<uint32>(key.isSwapchainRenderPass));

		std::hash<hs::Attachment> attachmentHash;
		for (size_t i = 0; i + 1 < key.colorAttachmentCount; i += 2)
		{
			h = hs::HashCombine64(h, attachmentHash(key.colorAttachments[i]), attachmentHash(key.colorAttachments[i + 1]));
		}

		size_t b = (key.colorAttachmentCount % 2 != 0) ? attachmentHash(key.colorAttachments.back()) : 0;
		size_t c = (key.useDepthStencilAttachment) ? attachmentHash(key.depthStencilAttachment) : 0;
		h = hs::HashCombine64(h, b, c);
		return h;
	}
};

template <>
struct hash<hs::TextureInfo>
{
	size_t operator()(const hs::TextureInfo& key) const
	{
		size_t h = hs::HashCombine(key.extent.width, key.extent.height, key.extent.depth);
		h = hs::HashCombine64(h, static_cast<uint32>(key.format), static_cast<uint32>(key.type));
		h = hs::HashCombine64(h, static_cast<uint32>(key.usage), key.mipLevel);
		h = hs::HashCombine64(h, key.arrayLength, key.byteSize);
		h = hs::HashCombine64(h, key.isCompressed, key.isSwapchainTexture);
		h = hs::HashCombine64(h, key.isDepthStencilBuffer, key.useGenerateMipmap);
		return h;
	}
};

template <>
struct hash<hs::SamplerInfo>
{
	size_t operator()(const hs::SamplerInfo& key) const
	{
		size_t h = hs::HashCombine(
			static_cast<uint32>(key.type),
			static_cast<uint32>(key.minFilter),
			static_cast<uint32>(key.magFilter));
		h = hs::HashCombine64(h, static_cast<uint32>(key.mipmapMode), static_cast<uint32>(key.addressU));
		h = hs::HashCombine64(h, static_cast<uint32>(key.addressV), static_cast<uint32>(key.addressW));
		h = hs::HashCombine64(h, key.isPixelCoordinate);
		return h;
	}
};

template <>
struct hash<hs::BufferInfo>
{
	size_t operator()(const hs::BufferInfo& key) const
	{
		return hs::HashCombine(static_cast<uint32>(key.usage), static_cast<uint32>(key.memoryOption));
	}
};
} // namespace std

#endif
