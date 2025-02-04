//
//  DebugPass.mm
//  MetalSamples
//
//  Created by Yongsik Im on 2/2/25.
//
#include "DebugPass.h"

HS_NS_BEGIN

void DebugPass::Initialize() {
    return;
}

void DebugPass::Finalize()
{
    
}

void DebugPass::OnBeforeRendering(uint32_t submitIndex) { 
    return;
}


void DebugPass::Configure(id<MTLTexture> renderTarget) {
    return;
}

void DebugPass::Execute(id<MTLRenderCommandEncoder> renderEncoder) {
    return;
}

void DebugPass::OnAfterRendering() {
    return;
}

HS_NS_END
