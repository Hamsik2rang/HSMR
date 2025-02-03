//
//  DebugPass.mm
//  MetalSamples
//
//  Created by Yongsik Im on 2/2/25.
//
#include "DebugPass.h"



void HSDebugPass::OnInit() { 
    return;
}

void HSDebugPass::OnAfterRendering() { 
    <#code#>;
}


void HSDebugPass::Execute(id<MTLRenderCommandEncoder> renderEncoder) { 
    <#code#>;
}


void HSDebugPass::Configure(id<MTLTexture> renderTarget) { 
    <#code#>;
}


void HSDebugPass::OnBeforeRendering(uint32_t submitIndex) { 
    <#code#>;
}


