#include "ShaderSystem/Spirv/SpirvCrossHelper.h"

#include "Core/Log.h"
#include <spirv_cross/spirv_glsl.hpp>
#include <spirv_cross/spirv_hlsl.hpp>

HS_NS_BEGIN

SpirvCrossHelper::SpirvCrossHelper()
{}

SpirvCrossHelper::~SpirvCrossHelper()
{
	Shutdown();
}

bool SpirvCrossHelper::Initialize()
{
	if (_initialized)
	{
		return true;
	}

	_initialized = true;
	return true;
}

void SpirvCrossHelper::Shutdown()
{
	_initialized = false;
}

SpirvReflectionData SpirvCrossHelper::ExtractReflection(const std::vector<uint32>& spirvBytecode)
{
	SpirvReflectionData reflectionData;

	spirv_cross::Compiler compiler(spirvBytecode);
	spirv_cross::ShaderResources resources = compiler.get_shader_resources();

	for (const auto& resource : resources.uniform_buffers)
	{
		ProcessBufferResource(compiler, resource, reflectionData.uniformBuffers);
	}

	for (const auto& resource : resources.storage_buffers)
	{
		ProcessBufferResource(compiler, resource, reflectionData.storageBuffers);
	}

	for (const auto& resource : resources.sampled_images)
	{
		ProcessImageResource(compiler, resource, reflectionData.sampledImages);
	}

	for (const auto& resource : resources.storage_images)
	{
		ProcessImageResource(compiler, resource, reflectionData.storageImages);
	}

	for (const auto& resource : resources.separate_samplers)
	{
		SpirvReflectionData::SamplerResource samplerRes;
		samplerRes.name = resource.name;
		samplerRes.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
		samplerRes.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
		reflectionData.samplers.push_back(samplerRes);
	}

	for (auto& input : resources.stage_inputs)
	{
		reflectionData.stageInputs.push_back(input);
	}

	for (auto& output : resources.stage_outputs)
	{
		reflectionData.stageOutputs.push_back(output);
	}

	for (auto& pushConst : resources.push_constant_buffers)
	{
		reflectionData.pushConstants.push_back(pushConst);
	}

	return reflectionData;
}

std::string SpirvCrossHelper::CrossCompileToMSL(const std::vector<uint32>& spirvBytecode, bool enableDebugInfo)
{
	if (spirvBytecode.empty())
	{
		HS_LOG(crash, "Cannot cross-compile empty SPIR-V bytecode to MSL");
		return "";
	}

	spirv_cross::CompilerMSL compiler(spirvBytecode);

	spirv_cross::CompilerMSL::Options options;
	options.enable_decoration_binding = true;
	options.enable_frag_output_mask = 0xFFFFFFFF;
	options.enable_frag_depth_builtin = true;
	options.enable_frag_stencil_ref_builtin = true;
	options.disable_rasterization = false;

	if (enableDebugInfo)
	{
		options.enable_decoration_binding = true;
	}

	compiler.set_msl_options(options);

	std::string mslSource = compiler.compile();
	if (mslSource.empty())
	{
		HS_LOG(error, "Failed to cross-compile SPIR-V to MSL: compile() returned empty string");
		return "";
	}
	
	return mslSource;
}

std::string SpirvCrossHelper::CrossCompileToHLSL(const std::vector<uint32>& spirvBytecode, bool enableDebugInfo)
{
	if (spirvBytecode.empty())
	{
		HS_LOG(crash, "Cannot cross-compile empty SPIR-V bytecode to HLSL");
		return "";
	}

	spirv_cross::CompilerHLSL compiler(spirvBytecode);

	spirv_cross::CompilerHLSL::Options options;
	options.shader_model = 50;
	options.point_size_compat = true;
	options.point_coord_compat = true;

	compiler.set_hlsl_options(options);

	std::string hlslSource = compiler.compile();
	if (hlslSource.empty())
	{
		HS_LOG(error, "Failed to cross-compile SPIR-V to HLSL: compile() returned empty string");
		return "";
	}
	
	return hlslSource;
}

std::string SpirvCrossHelper::CrossCompileToGLSL(const std::vector<uint32>& spirvBytecode, uint32 version, bool es)
{
	if (spirvBytecode.empty())
	{
		HS_LOG(crash, "Cannot cross-compile empty SPIR-V bytecode to GLSL");
		return "";
	}

	spirv_cross::CompilerGLSL compiler(spirvBytecode);

	spirv_cross::CompilerGLSL::Options options;
	options.version = version;
	options.es = es;
	options.vulkan_semantics = true;
	options.separate_shader_objects = true;

	compiler.set_common_options(options);

	std::string glslSource = compiler.compile();
	if (glslSource.empty())
	{
		HS_LOG(error, "Failed to cross-compile SPIR-V to GLSL: compile() returned empty string");
		return "";
	}
	
	return glslSource;
}

void SpirvCrossHelper::ProcessBufferResource(
	const spirv_cross::Compiler& compiler,
	const spirv_cross::Resource& resource,
	std::vector<SpirvReflectionData::BufferResource>& buffers)
{
	SpirvReflectionData::BufferResource bufferRes;
	bufferRes.name = resource.name;
	bufferRes.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
	bufferRes.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);

	const auto& type = compiler.get_type(resource.type_id);
	bufferRes.size = static_cast<uint32>(compiler.get_declared_struct_size(type));
	bufferRes.baseType = type.basetype;

	buffers.push_back(bufferRes);
}

void SpirvCrossHelper::ProcessImageResource(
	const spirv_cross::Compiler& compiler,
	const spirv_cross::Resource& resource,
	std::vector<SpirvReflectionData::ImageResource>& images)
{
	SpirvReflectionData::ImageResource imageRes;
	imageRes.name = resource.name;
	imageRes.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
	imageRes.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);

	const auto& type = compiler.get_type(resource.type_id);
	imageRes.imageType = type.image;
	imageRes.multisampled = type.image.ms;

	images.push_back(imageRes);
}

HS_NS_END