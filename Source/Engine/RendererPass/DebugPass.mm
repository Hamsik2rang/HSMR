//
//  DebugPass.mm
//  HSMR
//
//  Created by Yongsik Im on 2/2/25.
//
#include "DebugPass.h"

#import <Metal/Metal.h>

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


void DebugPass::Configure(RenderTexture* renderTarget) {
    return;
}

void DebugPass::Execute(RenderCommandEncoder* renderEncoder) {
    return;
}

void DebugPass::OnAfterRendering() {
    return;
}

HS_NS_END
