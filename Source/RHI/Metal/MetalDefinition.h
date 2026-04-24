//
//  RHIDefinitionMetal.h
//  Engine
//
//  Created by Yongsik Im on 2/14/25.
//

#import <Metal/Metal.h>

// Metal Vertex Stage Buffer Slot Layout
// ──────────────────────────────────────────────────────────────
// Metal의 vertex stage에서 setVertexBuffer로 바인딩하는 버퍼 슬롯은
// UBO(Uniform Buffer)와 정점 버퍼(Vertex Buffer)가 같은 네임스페이스를 공유한다.
//
//   Slot  0 ~ 28 : UBO (Slang 리플렉션의 getOffset(MetalBuffer)로 결정)
//   Slot 30 ~ 29 : Vertex Buffer (kMetalVertexBufferBaseIndex - binding)
//
// MTLVertexDescriptor의 layouts[i].bufferIndex와
// BindVertexBuffers()의 setVertexBuffer:atIndex: 모두
// kMetalVertexBufferBaseIndex를 기준으로 역방향 오프셋한다.
// 이로써 UBO 슬롯과 정점 버퍼 슬롯이 겹치지 않는다.
//
// [[stage_in]] 메커니즘:
// Metal 셰이더에서 vertex 함수의 [[stage_in]] 파라미터는
// MTLVertexDescriptor가 정의한 레이아웃에 따라 정점 데이터를
// 자동으로 fetch하여 구조체로 조립한 결과를 받는다.
// 즉 CPU 쪽에서 setVertexBuffer:atIndex:로 바인딩한 raw 버퍼를
// GPU가 MTLVertexDescriptor(attributes + layouts)에 맞춰 읽고,
// 셰이더 입력 구조체의 [[attribute(n)]] 필드에 매핑한다.
// 이 과정은 Metal이 내부적으로 처리하므로 셰이더 코드에서
// 버퍼 슬롯 번호를 직접 참조할 필요가 없다.
// ──────────────────────────────────────────────────────────────
static constexpr NSUInteger kMetalVertexBufferBaseIndex = 30;
static constexpr NSUInteger kMetalVertexBufferMinIndex = 29;
static constexpr NSUInteger kMetalMaxVertexBufferSlotCount = 31;
static constexpr NSUInteger kMetalReservedVertexBufferSlotCount =
    kMetalVertexBufferBaseIndex - kMetalVertexBufferMinIndex + 1;

static inline NSUInteger MetalVertexBufferSlotForBinding(NSUInteger binding)
{
    return kMetalVertexBufferBaseIndex - binding;
}
