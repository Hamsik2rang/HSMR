# HSMR 아키텍처 검토 보고서

> **목적**:
> 1. 고급 그래픽스 기법(GPU Gems, Real-Time Rendering, PBR 등)을 빠르게 구현하고 테스트
> 2. Vulkan과 Metal 두 그래픽 API를 실전 프로젝트를 통해 학습
>
> **분석일**: 2026-01-25

---

## 1. 현황 요약

### 코드베이스 규모
| 모듈 | 파일 수 | 코드 라인 | 비율 |
|------|---------|----------|------|
| RHI (그래픽 API 추상화) | 46 | 8,691 | 45.3% |
| Engine (렌더러/리소스) | 47 | 4,826 | 25.2% |
| Editor (ImGui 도구) | 34 | 2,219 | 11.6% |
| Platform (OS 추상화) | 22 | 1,863 | 9.7% |
| Core (기초 유틸리티) | 31 | 1,559 | 8.1% |
| **합계** | **180** | **19,200** | **100%** |

### 핵심 지표
```
실제 렌더링 로직:     ~800 LOC (4.2%)
인프라/추상화 코드:   ~10,600 LOC (55%)
인프라 대 렌더링 비율: 13:1
```

---

## 2. 개발 목적 대비 구조적 타당성 검토

### 2.1 목적 정의
**"고급 그래픽스 기법을 빠르게 구현하고 테스트"**

이 목적에서 핵심 요소:
1. **빠른 구현**: 최소한의 보일러플레이트로 새 기법 추가
2. **빠른 테스트**: 즉각적인 시각적 피드백
3. **다양한 기법 실험**: GPU Gems, SIGGRAPH 논문 등의 기법 구현

### 2.2 현재 아키텍처의 강점 ✅

| 강점 | 설명 | 목적 부합도 |
|------|------|------------|
| **깔끔한 레이어 구조** | 순환 의존성 없는 DAG 구조 | 유지보수성 높음 |
| **렌더 패스 확장성** | `RenderPass` 상속으로 새 패스 추가 용이 | 새 기법 추가에 적합 |
| **파이프라인 캐싱** | `RHIHandleCache`로 PSO 재생성 방지 | 반복 실행 성능 양호 |
| **컴퓨트 셰이더 지원** | Atmosphere 구현으로 검증됨 | GPU Compute 기법 가능 |
| **크로스 플랫폼** | Metal + Vulkan 동시 지원 | 다양한 환경에서 검증 가능 |

### 2.3 현재 아키텍처의 약점 ❌

| 약점 | 영향 | 심각도 |
|------|------|--------|
| **과도한 추상화 비용** | 800 LOC 렌더링을 위해 10,600 LOC 인프라 필요 | 🟡 중간 |
| **수동 레퍼런스 카운팅** | `Retain()`/`Release()` 수동 호출 필요 → `Ref<T>` 래퍼로 해결 가능 | 🟢 낮음 |
| **복잡한 리소스 프록시** | `ImageProxy`, `MeshProxy` 등 1,500 LOC 보일러플레이트 | 🟡 중간 |
| **RTTI 비활성화** | 런타임 디버깅 어려움 | 🟢 낮음 |

### 2.4 크로스 플랫폼에 대한 평가

**API 학습 목적을 고려하면 크로스 플랫폼은 약점이 아닌 강점입니다:**

| 관점 | 평가 |
|------|------|
| "빠른 프로토타이핑"만 고려 시 | 🔴 오버헤드 - 양쪽 구현 필요 |
| "API 학습" 목적 포함 시 | 🟢 **핵심 가치** - 두 API 비교 학습 가능 |

**크로스 플랫폼의 학습적 가치:**
- Vulkan의 명시적 동기화 vs Metal의 암묵적 처리 비교
- 디스크립터 셋 (Vulkan) vs 아규먼트 버퍼 (Metal) 패턴 이해
- 각 API의 철학과 설계 의도 체득
- 추상화 레이어 설계 경험 (실무에서 필수적인 역량)

**권장 개발 워크플로우:**
```
새 기법 구현 시:
1. 주 개발 플랫폼(예: macOS/Metal)에서 먼저 구현
2. 셰이더 + 렌더 패스 동작 검증
3. 다른 플랫폼(Windows/Vulkan)으로 포팅
4. 양쪽 구현 비교하며 API 차이점 학습
   - Vulkan의 명시적 동기화 vs Metal의 암묵적 처리
   - 디스크립터 셋 vs 아규먼트 버퍼
   - 메모리 관리 패턴 차이
5. 공통 추상화 개선점 발견 시 RHI 레이어 리팩토링
```

이 워크플로우를 통해 **기법 검증 속도**와 **API 학습 깊이** 두 가지를 모두 확보할 수 있습니다.

---

## 3. 구조적 불일치 분석

### 3.1 "빠른 프로토타이핑" vs "프로덕션 엔진"

현재 아키텍처는 **프로덕션 엔진** 패턴을 따르고 있습니다:

```
프로토타이핑 렌더러가 필요로 하는 것:
├─ 최소한의 추상화
├─ 단일 플랫폼 집중
├─ 셰이더 중심 개발
└─ 즉각적 피드백 루프

현재 HSMR이 제공하는 것:
├─ 다중 추상화 레이어 (Core → Platform → RHI → Engine)
├─ 크로스 플랫폼 지원 (Win + Mac)
├─ 복잡한 리소스 관리 시스템
└─ 빌드 → 링크 → 실행 사이클
```

### 3.2 비용-이점 분석

**새로운 렌더링 기법 추가 시 필요한 작업:**

```cpp
// 현재 구조에서 새 기법(예: SSAO) 추가 시:
1. ForwardRenderPass 또는 새 RenderPass 클래스 생성
2. Slang 셰이더 작성 (.vert, .frag 또는 .comp)
3. 필요시 새로운 RHI 리소스 타입 추가 (Metal + Vulkan 양쪽)
4. ObjectManager에 리소스 로딩 로직 추가
5. Renderer에 패스 통합
6. 양 플랫폼에서 테스트

// 예상 작업량: 새 기법당 500-2000 LOC
```

**비교: 단순 프로토타이핑 렌더러의 경우:**
```cpp
// 단순 구조에서 새 기법 추가 시:
1. 셰이더 작성
2. 드로우 콜 추가

// 예상 작업량: 새 기법당 50-200 LOC
```

---

## 4. 간소화 제안

### 4.1 단기 개선 (현재 구조 유지)

#### A. 프로토타이핑 전용 진입점 추가
```cpp
// 제안: Engine/Experimental/QuickPass.h
class QuickPass : public RenderPass {
public:
    // 최소한의 설정으로 셰이더만 실행
    void SetShader(const char* path);
    void SetUniform(const char* name, void* data, size_t size);
    void Draw(int vertexCount);
};
```

**장점**: 기존 인프라 활용하면서 빠른 실험 가능
**단점**: 여전히 RHI 추상화 비용 발생

#### B. 단일 플랫폼 모드 옵션
```cmake
# CMakeLists.txt 수정
option(HSMR_SINGLE_PLATFORM "Build for current platform only" OFF)

if(HSMR_SINGLE_PLATFORM)
    # 현재 플랫폼만 빌드 - 크로스 플랫폼 코드 제외
endif()
```

**효과**: 빌드 시간 단축, 유지보수 대상 절반으로 감소

#### C. RAII 래퍼 `Ref<T>` 추가

현재 `Retain()`/`Release()` 수동 호출 방식은 **성능상 적절한 선택**입니다:
- `std::shared_ptr`의 atomic 연산 오버헤드 회피
- Intrusive reference counting으로 캐시 효율성 확보
- 고성능 그래픽스 엔진의 표준 패턴

다만 **사용 편의성**을 위해 RAII 래퍼 추가를 권장합니다:

```cpp
// 제안: Core/Ref.h
template<typename T>
class Ref {
    T* _ptr = nullptr;
public:
    Ref() = default;
    explicit Ref(T* p) : _ptr(p) {}  // 생성 시 카운트 이미 1

    ~Ref() { if (_ptr) _ptr->Release(); }

    Ref(const Ref& o) : _ptr(o._ptr) {
        if (_ptr) _ptr->Retain();
    }

    Ref(Ref&& o) noexcept : _ptr(o._ptr) {
        o._ptr = nullptr;  // 이동 시 카운트 변화 없음
    }

    Ref& operator=(const Ref& o) {
        if (this != &o) {
            if (_ptr) _ptr->Release();
            _ptr = o._ptr;
            if (_ptr) _ptr->Retain();
        }
        return *this;
    }

    Ref& operator=(Ref&& o) noexcept {
        if (this != &o) {
            if (_ptr) _ptr->Release();
            _ptr = o._ptr;
            o._ptr = nullptr;
        }
        return *this;
    }

    T* operator->() const { return _ptr; }
    T& operator*() const { return *_ptr; }
    T* Get() const { return _ptr; }
    explicit operator bool() const { return _ptr != nullptr; }
};

template<typename T, typename... Args>
Ref<T> MakeRef(Args&&... args) {
    return Ref<T>(new T(std::forward<Args>(args)...));
}
```

**기존 네이밍과 일관성:**
```cpp
Scoped<T>  // std::unique_ptr 래퍼 - 단독 소유
Ref<T>     // Intrusive shared ptr - 공유 소유 (신규)

// 사용 예
Ref<RHITexture> tex(context->CreateTexture(info));
Ref<RHITexture> tex2 = tex;  // 복사 → 내부에서 Retain() 자동 호출
// 스코프 종료 → 소멸자에서 Release() 자동 호출
```

**장점:**
- 기존 intrusive 카운팅의 성능 특성 100% 유지
- 수동 `Retain()`/`Release()` 호출 불필요
- 기존 raw pointer 코드와 공존 가능
- 구현 비용: ~50 LOC

### 4.2 중기 구조 변경

#### A. RHI 레이어 경량화 옵션

**현재 RHI 구조** (8,691 LOC):
```
RHI/
├─ RHIDefinition.h (961 LOC) - 타입 정의
├─ Metal/ (1,866 LOC)
├─ Vulkan/ (4,048 LOC)
└─ 공통 추상화 (~2,000 LOC)
```

**제안: "Thin RHI" 모드**
```cpp
// Metal 전용 경량 경로 (macOS 개발 시)
#ifdef HSMR_THIN_RHI
    // 직접 Metal API 사용, 추상화 없음
    id<MTLCommandBuffer> cmd = [queue commandBuffer];
    // ...
#else
    // 기존 RHI 추상화 사용
    RHICommandBuffer* cmd = context->CreateCommandBuffer();
#endif
```

**예상 효과**:
- 개발 시 RHI 추상화 우회 가능
- 새 기법 검증 후 필요시 RHI 통합

#### B. 리소스 프록시 시스템 선택적 사용

```cpp
// 현재: 모든 리소스가 프록시 필수
ImageProxy* image = ObjectManager::Get().CreateImage("path.png");

// 제안: 직접 RHI 리소스 생성 경로 추가
RHITexture* tex = RHIContext::Get().CreateTextureFromFile("path.png");
// 프록시 없이 직접 사용 (실험용)
```

### 4.3 장기 아키텍처 대안

현재 목적에 더 적합한 구조 제안:

```
Alternative: "Shader Playground" 구조
─────────────────────────────────────
Core/
├─ Math.h          (현재 유지)
├─ Window.h        (단일 플랫폼)
└─ Timer.h

Graphics/
├─ Context.h       (Metal 또는 Vulkan 직접 사용)
├─ Shader.h        (Slang 컴파일러 래핑)
├─ Buffer.h        (최소 추상화)
└─ Texture.h

Experiments/
├─ SSAO/
├─ VolumetricFog/
├─ AtmosphericScattering/
└─ ...각 기법별 독립 폴더

예상 규모: 3,000-5,000 LOC (현재의 15-25%)
```

---

## 5. 보완 제안

### 5.1 개발 워크플로우 개선

#### A. 핫 리로드 셰이더 시스템
```cpp
// 제안: 셰이더 파일 변경 감지 및 자동 재컴파일
class ShaderHotReload {
    void WatchDirectory(const char* path);
    void OnShaderModified(std::function<void(Shader*)> callback);
};
```

**효과**: 셰이더 수정 → 즉시 결과 확인 (재빌드 불필요)

#### B. 렌더 디버그 GUI 강화
```cpp
// Editor/에 추가 제안
class RenderDebugPanel {
    void ShowPipelineState();      // 현재 PSO 상태
    void ShowResourceBindings();   // 바인딩된 리소스
    void ShowGPUTimings();         // 패스별 GPU 시간
    void ShowIntermediateTargets(); // 중간 렌더 타겟 미리보기
};
```

### 5.2 문서화 보완

현재 누락된 문서:
1. **렌더 패스 작성 가이드**: `RenderPass` 상속 방법
2. **셰이더 규약**: Slang 셰이더 구조 및 바인딩 규칙
3. **리소스 생명주기**: 프록시 시스템 사용법
4. **RHI 확장 가이드**: 새 기능 추가 시 Metal/Vulkan 양쪽 구현 방법

### 5.3 테스트 인프라 추가

```cpp
// 제안: 간단한 시각적 회귀 테스트
class RenderTest {
    void CaptureFrame(const char* testName);
    bool CompareWithBaseline(const char* testName, float threshold);
};
```

---

## 6. 결론 및 권장 사항

### 현재 아키텍처 평가

| 평가 항목 | 점수 | 설명 |
|----------|------|------|
| 코드 품질 | ★★★★☆ | 일관된 명명, 깔끔한 구조 |
| 확장성 | ★★★★☆ | 새 렌더 패스 추가 용이 |
| 프로토타이핑 속도 | ★★★☆☆ | 인프라 비용 있으나 패턴 정립 후 가속 |
| API 학습 가치 | ★★★★★ | Vulkan/Metal 비교 학습에 최적 |
| 목적 부합도 | ★★★★☆ | 그래픽스 기법 + API 학습 두 목적 모두 충족 |

### 권장 경로

**Option A: 현재 아키텍처 유지 + 경량 실험 레이어 추가** (권장)
- `QuickPass` 클래스 추가로 빠른 실험 지원
- 기존 인프라의 장점(캐싱, 크로스 플랫폼) 유지
- 추가 작업량: ~500 LOC

**Option B: 단일 플랫폼 모드 도입**
- macOS (Metal) 개발에 집중
- 크로스 플랫폼은 기법 검증 후 필요시 추가
- 유지보수 부담 50% 감소

**Option C: 병렬 경량 렌더러 개발**
- 현재 HSMR은 "레퍼런스 구현"용으로 유지
- 별도의 3,000 LOC급 경량 렌더러로 빠른 실험
- 검증된 기법만 HSMR에 통합

---

## 7. 즉시 실행 가능한 액션 아이템

### 높은 우선순위
1. [ ] `QuickPass` 또는 `ExperimentalPass` 클래스 설계
2. [ ] 셰이더 핫 리로드 시스템 조사 (Metal/Vulkan)
3. [ ] 단일 플랫폼 빌드 옵션 CMake에 추가

### 중간 우선순위
4. [ ] 렌더 패스 작성 가이드 문서화
5. [ ] RHI 스마트 포인터 전환 범위 결정
6. [ ] GPU 프로파일링 통합 (Metal System Trace, RenderDoc)

### 낮은 우선순위
7. [ ] 리소스 프록시 시스템 선택적 우회 경로
8. [ ] 시각적 회귀 테스트 프레임워크
9. [ ] 셰이더 디버그 출력 시스템

---

## 8. 경량화 아키텍처 제안

인프라 코드를 대폭 간소화하는 전면 재설계 제안은 별도 문서를 참조하세요:

**[Lightweight_Architecture_Proposal.md](./Lightweight_Architecture_Proposal.md)**

핵심 요약:
- 현재 19,200 LOC → 제안 5,200 LOC (73% 감소)
- Application/Window/Editor 계층 제거
- 2-레이어 구조: `Framework/` + `Samples/`
- Sample 하나가 60~200 LOC로 완결

---

*이 문서는 HSMR 프로젝트의 현재 상태를 "빠른 그래픽스 기법 프로토타이핑 + API 학습"이라는 목적에 비추어 분석한 결과입니다.*
