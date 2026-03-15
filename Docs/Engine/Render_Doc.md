# Render_Doc

## 개요

이 문서는 현재 렌더링/리소스 구조에서 가장 헷갈리기 쉬운 핵심만 정리한다. 특히 `Mesh`와 `Model`의 구분, `MeshRenderer`의 동작 방식, `Resource_System`의 로드 책임, 그리고 실제 드로우 시 머테리얼이 어떻게 결정되는지를 중심으로 설명한다.

이 구조의 핵심은 다음과 같다.

* `Mesh`는 단일 지오메트리 리소스이다.
* `Model`은 여러 `Mesh` 파트를 묶은 상위 리소스이다.
* `MeshRenderer`는 일단 `hMesh`, `hMaterial`을 가지고 드로우 커맨드를 만든다.
* 하지만 실제 렌더 단계에서는 `hMesh`가 진짜 메쉬인지 모델인지 다시 판별한다.
* 모델인 경우에는 모델 내부의 `parts`를 순회하면서 각 파트의 메쉬와 머테리얼을 사용해 렌더링한다.

즉, **드로우 커맨드 생성 시점의 정보와 실제 최종 드로우 시점의 정보 해석 방식이 다를 수 있다**는 점이 이 시스템의 핵심이다.

---

## 1. Mesh와 Model의 구분

현재 시스템은 핸들의 MSB를 사용해 `Mesh`와 `Model`을 구분한다.

* MSB = 0 이면 `Mesh`이다.
* MSB = 1 이면 `Model`이다.

따라서 `MeshRenderer`는 이름만 보면 메쉬만 렌더링하는 것처럼 보이지만, 실제로는 `hMesh` 자리에 모델 핸들이 들어올 수도 있다. 이 경우 렌더러는 단순히 핸들을 넘기고, 최종 렌더 단계인 `CRender_System::Execute_Draw_Mesh()`가 이를 다시 판별한다.

이 방식의 장점은 `MeshRenderer` 하나로 단일 메쉬와 모델을 모두 처리할 수 있다는 점이다. 반면 이름과 실제 역할이 살짝 어긋나므로, 구조를 처음 보는 사람은 `MeshRenderer`가 모델도 처리한다는 사실을 놓치기 쉽다.

---

## 2. MeshRenderer의 역할과 실제 머테리얼 적용 방식

`MeshRenderer`는 기본적으로 다음 두 핸들을 가진다.

* `hMesh`
* `hMaterial`

드로우 커맨드를 빌드할 때는 우선 이 값을 그대로 사용한다. 즉, `MeshRenderer`는 자신이 참조 중인 메쉬와 머테리얼 핸들을 기준으로 `DRAW_CMD`를 생성한다.

하지만 이것이 곧바로 "항상 그 머테리얼로 그린다"는 뜻은 아니다. 
실제 렌더 단계에서는 다음 흐름으로 동작한다.

### 단일 Mesh인 경우

`cmd.mesh.hMesh`가 일반 메쉬 핸들이면 다음 정보가 그대로 사용된다.

* `cmd.mesh.hMesh`
* `cmd.mesh.hMaterial`

즉, 이 경우에는 `MeshRenderer`가 가진 머테리얼이 최종 머테리얼이 된다.

### Model인 경우

`cmd.mesh.hMesh`가 모델 핸들이면 `MODEL_ENTRY`를 가져와 `parts`를 순회한다. 이때 실제 드로우는 모델 내부의 각 `MODEL_PART` 기준으로 수행된다.

즉, 모델인 경우 실제 렌더링은 다음 단위로 분해된다.

* `part.hMesh`
* `part.hMaterial`

그리고 현재 구현은 다음 정책을 사용한다.

* `part.hMaterial`이 유효하면 그것을 사용한다.
* `part.hMaterial`이 없으면 `cmd.mesh.hMaterial`을 fallback으로 사용한다.

따라서 **모델인 경우 `MeshRenderer`의 머테리얼은 무조건 무시되는 것이 아니라, 파트 머테리얼이 없을 때의 예비값 역할을 한다**고 이해해야 한다.

 `MeshRenderer`는 드로우 커맨드를 빌드할 때 자신의 머테리얼을 넣지만, 최종 렌더 단계에서 모델로 판정되면 파트별 머테리얼 우선 정책으로 덮어써질 수 있다.

---

## 3. Resource_System에서 Mesh와 Model을 로드하는 방식

`CResource_System::Load_Mesh()`는 GUID를 받아 해당 에셋이 진짜 메쉬인지 확인한다. 여기서 중요한 점은, **요청 이름이 `Load_Mesh()`여도 실제 GUID가 모델이면 `Load_Model()`로 다시 보낸다는 점**이다.

즉 현재 구조는 다음처럼 동작한다.

* GUID가 `MESH`이면 실제 `MESH_ENTRY`를 생성한다.
* GUID가 `MODEL`이면 `Load_Model()`로 위임한다.

이 구조 덕분에 상위 호출부에서는 일단 `Load_Mesh()`처럼 접근해도 내부에서 메쉬/모델을 다시 판별할 수 있다.

### Model 로드 시 생성되는 것

`Load_Model()`은 `.model` 파일을 읽어 `MODEL_DESC`를 만든 뒤, 그 안의 `parts` 정보를 기반으로 `MODEL_ENTRY`를 생성한다.

각 파트는 다음 정보를 가진다.

* `hMesh`
* `hMaterial`
* 필요 시 인덱스 범위 정보

즉, 모델은 스스로 여러 메쉬 파트를 소유하는 상위 리소스이며, 각 파트가 자기 메쉬와 머테리얼을 가질 수 있다.

이 때문에 **모델 단위 렌더링은 결국 여러 개의 단일 메쉬 드로우로 분해되는 구조**라고 볼 수 있다.

---

## 4. 왜 MeshEntry가 아니라 ModelPart가 머테리얼을 가져야 하는가

현재 방향에서 중요한 설계 포인트는 `MeshEntry`가 아니라 `MODEL_PART`가 머테리얼을 가진다는 점이다.

이렇게 해야 하는 이유는 다음과 같다.

* `Mesh`는 순수 지오메트리 리소스로 두는 편이 재사용성이 높다.
* 같은 메쉬를 여러 머테리얼로 그릴 수 있어야 한다.
* 머테리얼은 종종 모델 파트 단위 또는 렌더러 인스턴스 단위에서 결정된다.

예를 들어 같은 `Mesh`를 어떤 모델에서는 금속 재질로 쓰고, 다른 모델에서는 피부 재질로 쓸 수도 있다. 따라서 `Mesh` 자체에 머테리얼을 박아 넣으면 재사용성과 유연성이 떨어진다.

반면 `Model`은 여러 파트를 조합하는 상위 개념이므로, 각 파트별 기본 머테리얼을 가지는 것이 자연스럽다.

즉 정리하면 다음과 같다.

* `MeshEntry`는 지오메트리 중심 리소스이다.
* `MODEL_PART`는 모델 내부 조합 단위이다.
* 머테리얼 매핑 책임은 `MODEL_PART` 쪽이 더 자연스럽다.

---

## 5. 최종 렌더 흐름

현재 렌더 흐름은 개념적으로 다음과 같다.

1. `MeshRenderer`가 `DRAW_CMD`를 만든다.
2. `CRender_System::Execute_Draw_Mesh()`가 `cmd.mesh.hMesh`를 보고 메쉬인지 모델인지 판별한다.
3. 메쉬면 그대로 `Execute_Draw_Mesh_Inner()`를 한 번 호출한다.
4. 모델이면 `MODEL_ENTRY::parts`를 순회하면서 파트마다 `Execute_Draw_Mesh_Inner()`를 호출한다.
5. `Execute_Draw_Mesh_Inner()`는 최종적으로

   * `MESH_ENTRY`
   * `MATERIAL_ENTRY`
   * `SHADER_ENTRY`
     를 가져와 셰이더 변수, 텍스처, 파라미터를 바인딩하고 드로우를 수행한다.

즉, **최종 드로우 함수는 언제나 단일 메쉬 기준**이다. 모델은 그 직전에 여러 개의 단일 메쉬 드로우로 풀어지는 상위 개념일 뿐이다.

이 관점으로 보면 구조가 훨씬 덜 헷갈린다.

---

## 6. 이 구조에서 반드시 기억할 것

### 6-1. MeshRenderer는 모델도 받을 수 있다

이름 때문에 단일 메쉬 전용처럼 보이지만 실제로는 아니다. `hMesh` 핸들이 모델일 수도 있고, 이 경우 렌더 단계에서 모델 파트 순회가 발생한다.

### 6-2. MeshRenderer의 hMaterial은 항상 최종값이 아니다

단일 메쉬일 때는 최종 머테리얼이다. 하지만 모델일 때는 파트 머테리얼이 우선이며, `MeshRenderer`의 머테리얼은 fallback일 뿐이다.

### 6-3. 모델은 여러 draw call로 분해된다

모델 하나를 렌더한다는 것은 내부적으로 `parts` 수만큼 여러 번 `Execute_Draw_Mesh_Inner()`를 호출한다는 뜻이다.

### 6-4. 최종 렌더 단위는 Mesh + Material이다

렌더 시스템의 실제 처리 단위는 언제나 단일 `Mesh`와 단일 `Material`의 조합이다. 모델은 이를 묶어서 관리하는 상위 데이터일 뿐이다.

---

## 7. 현재 코드 기준 주의할 점

### 7-1. partDesc의 머테리얼 GUID를 실제 part에 복사해야 한다

`.model` 파일에 `part0MaterialGuid=...` 형태로 저장되어 있어도, `Load_ModelDesc()`와 `Load_Model()`에서 그 값을 실제 `MODEL_PART` 또는 중간 `MODEL_DESC_PART`에 반영하지 않으면 런타임에서 사용할 수 없다.

즉 다음 단계가 모두 연결되어야 한다.

* `.model` 파일에 머테리얼 GUID 저장
* `Load_ModelDesc()`에서 머테리얼 GUID 파싱
* `Load_Model()`에서 `partDesc`의 머테리얼 GUID 사용
* `MODEL_PART.hMaterial` 로드 결과 저장

이 중 하나라도 빠지면 모델 파트 머테리얼 매핑은 동작하지 않는다.

---

## 8. 권장 해석

현재 구조를 가장 간단히 요약하면 다음과 같다.

* `MeshRenderer`는 렌더 요청을 만든다.
* `Render_System`은 그 요청이 메쉬인지 모델인지 해석한다.
* 모델이면 내부 파트들로 분해한다.
* 최종적으로는 각 파트를 `Mesh + Material` 단위로 렌더링한다.

따라서 이 시스템은 겉보기에는 `MeshRenderer` 중심 구조이지만, 실제로는 **모델 해석 책임이 렌더 시스템 내부에 들어간 하이브리드 구조**라고 볼 수 있다.

이 점을 이해하면 `MeshRenderer`가 머테리얼을 들고 있는데 왜 모델 파트 머테리얼이 적용되는지, `Load_Mesh()`가 왜 모델도 처리하는지, 왜 최종 draw 함수는 단일 메쉬 기준인지가 모두 자연스럽게 연결된다.

---

## 9. 한 줄 요약

현재 구조에서 `Mesh`는 단일 지오메트리, `Model`은 여러 메쉬 파트의 묶음이다. `MeshRenderer`는 일단 자신의 `hMesh`, `hMaterial`로 드로우 요청을 만들지만, 실제 렌더 단계에서는 핸들의 MSB를 보고 메쉬/모델을 다시 판별한다. 모델로 판정되면 `MODEL_ENTRY::parts`를 순회하며 각 파트의 메쉬와 머테리얼을 사용해 최종 렌더링한다.
