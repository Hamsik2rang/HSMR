#ifndef __HS_SPIRV_CROSS_HELPER_H__
#define __HS_SPIRV_CROSS_HELPER_H__

#include "Precompile.h"
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_msl.hpp>
#include <spirv_cross/spirv_reflect.hpp>
#include <vector>
#include <string>

HS_NS_BEGIN

struct SpirvReflectionData
{
    struct BufferResource
    {
        std::string name;
        uint32 binding;
        uint32 set;
        uint32 size;
        spirv_cross::SPIRType::BaseType baseType;
    };
    
    struct ImageResource
    {
        std::string name;
        uint32 binding;
        uint32 set;
        spirv_cross::SPIRType::ImageType imageType;
        bool multisampled;
    };
    
    struct SamplerResource
    {
        std::string name;
        uint32 binding;
        uint32 set;
    };
    
    std::vector<BufferResource> uniformBuffers;
    std::vector<BufferResource> storageBuffers;
    std::vector<ImageResource> sampledImages;
    std::vector<ImageResource> storageImages;
    std::vector<SamplerResource> samplers;
    
    std::vector<spirv_cross::Resource> stageInputs;
    std::vector<spirv_cross::Resource> stageOutputs;
    std::vector<spirv_cross::Resource> pushConstants;
};

class HS_SHADERSYSTEM_API SpirvCrossHelper
{
public:
    SpirvCrossHelper();
    ~SpirvCrossHelper();

    bool Initialize();
    void Shutdown();

    SpirvReflectionData ExtractReflection(const std::vector<uint32>& spirvBytecode);

    std::string CrossCompileToMSL(
        const std::vector<uint32>& spirvBytecode,
        bool enableDebugInfo = false
    );

    std::string CrossCompileToHLSL(
        const std::vector<uint32>& spirvBytecode,
        bool enableDebugInfo = false
    );

    std::string CrossCompileToGLSL(
        const std::vector<uint32>& spirvBytecode,
        uint32 version = 450,
        bool es = false
    );

    bool IsInitialized() const { return _initialized; }

private:
    bool _initialized = false;

    void ProcessResource(
        const spirv_cross::Compiler& compiler,
        const spirv_cross::Resource& resource,
        SpirvReflectionData& reflectionData
    );
    
    void ProcessBufferResource(
        const spirv_cross::Compiler& compiler,
        const spirv_cross::Resource& resource,
        std::vector<SpirvReflectionData::BufferResource>& buffers
    );
    
    void ProcessImageResource(
        const spirv_cross::Compiler& compiler,
        const spirv_cross::Resource& resource,
        std::vector<SpirvReflectionData::ImageResource>& images
    );
};

HS_NS_END

#endif