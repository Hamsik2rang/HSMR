//
//  AtmosphereSkyPass.h
//  Engine/Renderer/Atmosphere
//
//  RenderPass for atmospheric sky rendering.
//
#ifndef __HS_ATMOSPHERE_SKY_PASS_H__
#define __HS_ATMOSPHERE_SKY_PASS_H__

#include "Precompile.h"
#include "Engine/Renderer/RenderPass/RenderPass.h"
#include "Engine/Renderer/Atmosphere/AtmosphereRenderer.h"
#include "Core/ThirdParty/glm/glm.hpp"

HS_NS_BEGIN

class RHIShader;
class RHIGraphicsPipeline;
class RHIResourceLayout;
class RHIResourceSet;
class RHIBuffer;
class AtmosphereRenderer;

// =============================================================================
// AtmosphereSkyPass
// =============================================================================
// Renders the atmospheric sky as a full-screen pass.
// Uses precomputed LUT textures from AtmosphereRenderer.

class HS_API AtmosphereSkyPass : public RenderPass
{
public:
	AtmosphereSkyPass(const char* name, RenderPath* renderer, AtmosphereRenderer* atmosphereRenderer);
	~AtmosphereSkyPass() override;

	void OnBeforeRendering(uint32_t submitIndex) override;
	void Configure(RenderTarget* renderTarget) override;
	void Execute(RHICommandBuffer* commandBuffer, RHIRenderPass* renderPass) override;
	void OnAfterRendering() override;
	void Clear() override;

	// Update camera and sun parameters
	void SetViewProjectionMatrix(const glm::mat4& view, const glm::mat4& projection);
	void SetCameraPosition(const glm::vec3& position);
	void SetSunDirection(const glm::vec3& direction);

private:
	void createResources();
	void destroyResources();
	void createPipeline(RHIRenderPass* renderPass);
	void updateUniforms();

private:
	AtmosphereRenderer* _atmosphereRenderer = nullptr;
	RenderTarget* _currentRenderTarget = nullptr;

	// Shaders
	RHIShader* _vertexShader = nullptr;
	RHIShader* _fragmentShader = nullptr;

	// Pipeline
	RHIGraphicsPipeline* _pipeline = nullptr;

	// Resources
	RHIResourceLayout* _resourceLayout = nullptr;
	RHIResourceSet* _resourceSet = nullptr;
	RHIBuffer* _uniformBuffer = nullptr;
	RHIBuffer* _fullscreenQuadVB = nullptr;

	// Uniforms
	AtmosphereRenderer::SkyUniforms _uniforms;
	glm::mat4 _viewMatrix;
	glm::mat4 _projectionMatrix;
	glm::vec3 _cameraPosition = glm::vec3(0.0f, 100.0f, 0.0f); // 100m above ground (world coords)
	glm::vec3 _sunDirection = glm::normalize(glm::vec3(0.2f, 0.8f, 0.5f)); // Sun high in sky, slightly to the side

	bool _resourcesCreated = false;
	bool _uniformsDirty = true;
};

HS_NS_END

#endif /* __HS_ATMOSPHERE_SKY_PASS_H__ */
