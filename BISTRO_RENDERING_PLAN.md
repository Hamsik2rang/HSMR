# Bistro 렌더링 계획 (별도 작업)

> 본 문서는 `Assets/GLTF/Bistro_v5_2/` 자산을 SimpleWindow에서 렌더링하기 위한
> **별도 추적용 계획**이다. Sponza 작업과는 분리되어 있으며, 본 작업은 별도 일정으로
> 진행한다. (Sponza 메인 플랜: `~/.claude/plans/assets-gltf-humming-cupcake.md`)

## 자산 현황

| 항목 | 값 |
|------|----|
| 위치 | `Assets/GLTF/Bistro_v5_2/` |
| 모델 포맷 | `.fbx` (BistroExterior.fbx, BistroInterior.fbx, BistroInterior_Wine.fbx) |
| 텍스처 포맷 | `.dds` (BC1/BC3/BC5/BC7 추정, 633개) |
| 추가 자산 | `.pyscene` (라이팅/머티리얼 메타데이터, Falcor 포맷), `san_giuseppe_bridge_4k.hdr` (IBL) |
| 총 용량 | 약 1.7 GB |

폴더 이름이 `Bistro_v5_2`이지만 .gltf 파일은 없음. Sponza와 동일 경로로는 처리 불가.

---

## 갭 분석

### Gap A — FBX 로더 분기
- 현재 `ObjectManager::LoadModel()` (`Source/Resource/Private/ObjectManager.cpp:997`)은
  확장자 검사로 GLTF만 처리. FBX 분기가 없거나 우회 필요.
- Assimp는 FBX를 네이티브 지원하므로 `loadGLTF()`의 import 로직 대부분을 재활용 가능.
- 단, FBX의 머티리얼 시스템은 GLTF와 다름 (PBR 키 vs. 전통 specular 키). 매핑 로직은
  텍스처 수동 추론 필요.

### Gap B — DDS 텍스처 디코더 부재
- 현재 텍스처 로더는 stb_image (`ObjectManager.cpp:480` 내부). DDS 미지원.
- Bistro 텍스처는 *_BaseColor.dds / *_Normal.dds / *_Specular.dds 패턴 (전통 specular workflow).
- 후보 라이브러리:
  - **gli** (header-only, KTX/DDS, BC 디코딩 + GPU 직접 업로드 가능)
  - **DirectXTex** (Windows 친화, BC1~BC7 모두 지원, CPU 디코딩 + 압축 포맷 그대로 GPU 업로드)
  - **자체 미니 BC 디코더** (BC1/BC3 정도만)
- 추천: **gli** — 의존성 가볍고 헤더-only, BC 압축 포맷을 그대로 RHI 텍스처로 업로드 가능
  (현재 `EPixelFormat`에 BC 포맷 추가 필요).

### Gap C — RHI BC 포맷 지원
- `Source/RHI/RHIDefinition.h` 또는 `RHITexture.h`의 `EPixelFormat`에 BC1_RGB_UNORM,
  BC3_RGBA_UNORM, BC5_UNORM, BC7_UNORM/SRGB 추가.
- Vulkan: `VK_FORMAT_BC{1,3,5,7}_*`. Metal: `MTLPixelFormatBC{1,3,5,7}_*`.
- 디코딩 없이 그대로 업로드하려면 텍스처 생성 시 row pitch / slice pitch가 BC 블록 단위
  계산되어야 함 (4x4 블록).

### Gap D — 전통 Specular workflow → PBR 셰이더 매핑
- Bistro는 Specular/Glossiness가 아니라 BaseColor/Specular(또는 Metalness 추정 가능) 형태.
- Sponza용 PBR.slang을 그대로 쓰려면 specular → metallic 근사 변환 필요. 또는 PBR-Spec 변형
  셰이더 작성.
- 권장: 1차에는 specular 텍스처를 metallic 슬롯으로 단순 매핑 후 시각적으로 확인, 2차에서
  Spec-Gloss → Metal-Rough 변환 또는 PBR-Spec 셰이더 추가.

### Gap E — `.pyscene` 메타데이터 처리
- Falcor 포맷. 카메라/라이트/머티리얼 오버라이드가 들어 있음.
- 1차 검증에서는 무시. 단순 FBX 지오메트리만 로드.

### Gap F — IBL (san_giuseppe_bridge_4k.hdr)
- 환경맵. Sponza/Bistro 모두 외관 품질에 큰 영향.
- 별도 IBL 통합 작업 (HDR 로드, prefilter, BRDF LUT, irradiance map 생성). 본 Bistro 계획
  외 별도 마일스톤으로 분리.

---

## 권장 단계

1. **포맷 확인 단계 (1일)**
   - FBX 한 모델만 Assimp로 시험 로드 → `loadGLTF()` 함수 복제 → `loadFBX()` 신규 작성
   - DDS 헤더 확인 (`DDS ` magic + `DDPIXELFORMAT.dwFourCC`) — BC 포맷 종류 파악
   - 결과: 어떤 BC 포맷이 필요한지 (BC1/3/5/7) 결정

2. **DDS 로더 도입 (2-3일)**
   - gli 헤더 추가 (`Dependency/include/gli/`)
   - `Source/Resource/Private/ImageDDS.cpp` 신규 — gli로 DDS → 압축 픽셀 데이터 + 포맷 정보
     반환
   - `Image` 클래스에 압축 데이터 저장 옵션 또는 `CompressedImage` 별도 클래스 도입

3. **RHI BC 포맷 추가 (1-2일)**
   - `EPixelFormat` 추가 + Vulkan/Metal 매핑 테이블 갱신
   - `RHIContext::CreateTexture` 가 압축 포맷일 때 row/slice pitch 4x4 블록 단위 처리
   - 테스트: 단일 BC1 텍스처 → 풀스크린 quad 출력으로 검증

4. **FBX 로더 (2-3일)**
   - `ObjectManager::loadFBX()` 작성 — `loadGLTF()` 기반, GLTF PBR 키 대신 전통 specular 키
     사용
   - `LoadModel()`에서 확장자 분기 (`.fbx` → `loadFBX()`)
   - 텍스처 매핑: aiTextureType_DIFFUSE→Diffuse, aiTextureType_NORMALS→Normal,
     aiTextureType_SPECULAR→Metallic (1차 근사)

5. **PBR 셰이더 호환 (1일)**
   - Sponza용 PBR.slang을 specular workflow에서도 동작하도록 #ifdef 분기 또는
     `PBR_Specular.slang` 별도 작성

6. **시각 검증**
   - BistroExterior.fbx 단독 로드 → 회랑/외벽이 정상 표시되는지 확인

7. **선택: IBL 통합** (별도 마일스톤)
   - san_giuseppe_bridge_4k.hdr → IBL 파이프라인 (irradiance + prefilter + BRDF LUT)
   - PBR.slang에 IBL 항 추가

---

## 대안: 자산 사전 변환

상기 작업이 부담스러울 경우 외부 도구로 일괄 변환:

- **Blender / Assimp CLI**: FBX → GLTF 일괄 변환
- **texconv** (DirectXTex CLI): DDS → PNG 변환 (압축 손실 + 디스크 용량 증가 약 4-10배)
- 변환 후 `Bistro/Bistro.gltf` + `Bistro/textures/*.png`로 재구성하면 Sponza와 동일 경로로
  재사용 가능

이 경로는 **단기 검증**에는 효과적이지만 메모리/디스크 비용이 크므로 장기적으로는 위 단계
1-7 진행 권장.

---

## 결정 필요 항목

- [ ] gli vs. DirectXTex vs. 자체 디코더 — 어느 것을 도입할지
- [ ] BC 포맷 GPU 직접 업로드 vs. CPU 디코딩 후 RGBA 업로드
- [ ] specular workflow → metallic 근사 vs. PBR-Spec 별도 셰이더
- [ ] `.pyscene` 처리 여부
- [ ] IBL 통합 시점 (Bistro 작업 일부 vs. 별도)

