#pragma once
# Animation Resource / Runtime Design

커스텀 엔진에서 **애니메이션 가능한 모델 리소스**를 다루기 위해,  
`MESH_ENTRY`, `MODEL_ENTRY`, `SKELETON_ENTRY`, `ANIMATION_CLIP_ENTRY`, `ANIMATOR_DATA`의 역할을 분리하는 구조 문서이다.

이 구조는 **converter.exe 에서 FBX를 Assimp로 읽어 바이너리/JSON로 굽고**,  
엔진 런타임에서는 그 결과물만 로드하는 파이프라인을 전제로 한다.

---

## 0. 용어 정리

### Mesh
- GPU가 직접 그릴 수 있는 **저수준 드로우 단위**이다.
- 정점 버퍼, 인덱스 버퍼, topology, AABB 등을 가진다.
- 엔진에서는 `MESH_ENTRY`로 관리한다.

### Model
- 하나 이상의 메쉬 파트와 머티리얼 참조를 묶은 **상위 리소스**이다.
- 애니메이션이 없는 모델 / 있는 모델 모두 포함할 수 있다.
- 엔진에서는 `MODEL_ENTRY`로 관리한다.

### Model Part
- 모델 내부의 **개별 메쉬 조각**이다.
- 어떤 메쉬를 그리고, 어떤 머티리얼을 쓸지, 어떤 인덱스 구간을 그릴지 등을 가진다.
- 스키닝 모델의 경우, 파트별 bone 매핑 정보도 포함할 수 있다.

### Skeleton
- 본(Bone)들의 **집합 + 부모/자식 계층 구조**이다.
- 모델이 어떤 본 구조를 가지는지 표현한다.
- 애니메이션 클립은 이 skeleton의 bone들을 기준으로 재생된다.

### Bone
- skeleton을 구성하는 단일 노드이다.
- 부모/자식 관계, bind pose 정보, offset matrix 등을 가진다.

### Animation Clip
- 하나의 애니메이션 데이터 묶음이다.
- duration, ticks per second, channel, keyframe 정보를 가진다.
- 본 자체를 저장하는 것이 아니라, **어느 bone을 어떻게 움직일지**를 저장한다.

### Animation Channel
- 특정 bone 하나에 대응되는 애니메이션 트랙이다.
- 해당 bone의 scale / rotation / translation keyframe들을 가진다.

### Animator
- 애니메이션 클립을 **실제로 재생하는 런타임 상태 객체/컴포넌트**이다.
- 현재 시간, 현재 클립, 채널별 현재 키프레임 인덱스, 최종 bone matrices 등을 가진다.

---

## 1. 전체 방향

이 구조의 핵심은 **정적 리소스와 런타임 상태를 분리**하는 것이다.

### 정적 리소스
- `MESH_ENTRY`
- `MODEL_ENTRY`
- `SKELETON_ENTRY`
- `ANIMATION_CLIP_ENTRY`

이들은 converter가 굽고, 엔진이 로드한 뒤 **여러 오브젝트가 공유**할 수 있는 데이터이다.

### 런타임 상태
- `ANIMATOR_DATA`
- 현재 재생 중인 시간
- 현재 키프레임 인덱스
- 최종 bone matrices

이들은 오브젝트마다 다를 수 있는 데이터이다.  
같은 모델/같은 애니메이션을 써도 오브젝트마다 재생 시간이 달라질 수 있으므로,  
절대로 리소스 엔트리에 넣으면 안 된다.

---

## 2. MESH_ENTRY

`MESH_ENTRY`는  **저수준 draw unit** 역할을 유지한다.

### 역할
- VB / IB 보관
- stride, index count, topology 보관
- 로컬 AABB 보관
- IA 바인딩 및 draw 호출 담당

### 설계 의도
애니메이션이 들어온다고 해서 `MESH_ENTRY`를 skeleton, clip, pose까지 다 가진 구조로 확장하지 않는다.  
그렇게 하면 메쉬가 너무 무거워지고, 모델/애니메이션과의 책임 분리가 무너진다.

### 결론
- `MESH_ENTRY`는 계속 **그리는 단위**이다.
- 애니메이션 모델도 결국 `MODEL_ENTRY`가 여러 `MESH_ENTRY`를 참조하는 형태로 처리한다.

---

## 3. MODEL_PART

`MODEL_PART`는 `MODEL_ENTRY` 내부의 개별 파트를 표현한다.

예시 역할은 다음과 같다.

- 어떤 mesh를 참조하는지
- 어떤 material을 참조하는지
- draw 시 어느 인덱스 범위를 쓰는지
- 스키닝 모델이라면 이 파트가 어떤 bone들을 사용하는지

### 역할
- 모델 내부의 파트별 draw 정보 보관
- 파트별 material slot 정보 보관
- 필요 시 part-local bone mapping 정보 보관

### 넣어도 되는 데이터
- `hMesh`
- `hMaterial`
- `materialGUID`
- `iFirstIndex`
- `iIndexCount`
- bone index 목록
- offset matrix 목록

이런 값들은 import 이후 바뀌지 않는 **정적 데이터**이므로 part에 넣어도 된다.

### 넣으면 안 되는 데이터
- 현재 포즈 결과 bone matrices
- 현재 키프레임 인덱스
- 현재 재생 시간

이 값들은 런타임마다 바뀌고, 오브젝트마다 달라질 수 있으므로 `Animator` 쪽에 있어야 한다.

---

## 4. MODEL_ENTRY

`MODEL_ENTRY`는 모델 전체를 대표하는 상위 리소스이다.

### 역할
- 여러 `MODEL_PART`를 소유
- 모델이 사용할 skeleton을 참조
- 모델이 사용할 animation clip들을 보관하거나 참조
- anim / non-anim 여부를 관리

### 설계 의도
기존 구조의 `CModel`은 다음을 가진다.

- `CMesh`
- `CBone`
- `CMaterial`
- `CAnimation`

현재 엔진 구조의 `MODEL_ENTRY`는 다음과 같다.


- `CMesh` → `MODEL_PART`가 참조하는 `MESH_ENTRY`
- `CBone` → `SKELETON_ENTRY`
- `CAnimation` → `ANIMATION_CLIP_ENTRY`
- `CMaterial` → part별 material handle / GUID

### 결론
애니메이션 가능한 모델은 **새로운 별도 메쉬 타입**이 아니라,  기존 모델 리소스가 skeleton / animation clip을 추가로 가진다.

---

## 5. SKELETON_ENTRY

`skeleton`은 **bone들의 집합 + 계층 구조**이다.

### 역할
- bone 전체 목록 보관
- 부모/자식 관계 보관
- bone 이름 → bone index 매핑 제공
- bind pose 관련 데이터 보관

### Bone과 Skeleton의 관계
- bone은 단일 노드
- skeleton은 bone들의 트리/계층 전체

즉, **bone들의 집합과 계층 구조 전체가 skeleton**이다.

### aiNode와의 관계
Assimp에서는 보통 `aiNode` 계층을 타고 들어가며 bone 구조를 복원한다.  
엔진의 `SKELETON_ENTRY`도 이와 유사하게 **부모-자식 구조**를 저장한다.

### 포함되는 정보의 예
- bone 이름
- 부모 bone index
- 자식 bone index 목록
- local bind transform
- combined bind transform
- offset matrix

### 주의점
Assimp의 모든 `aiNode`가 반드시 스키닝 bone인 것은 아니다.  
초기 구현에서는 **animation / mesh에서 사용되는 bone들만 추려서 skeleton에 넣는 방식**이 단순하다.

---

## 6. ANIMATION_CLIP_ENTRY

`ANIMATION_CLIP_ENTRY`는 하나의 애니메이션 리소스이다.

### 역할
- clip 이름 보관
- duration 보관
- ticks per second 보관
- 여러 channel 보관

### 기존 구조와의 대응
기존 구조의 `CAnimation`에 대응되는 리소스 개념이다.  
단, `CAnimation` 안의 모든 멤버를 그대로 들고 오는 것이 아니라,  **고정 데이터만** 담는다.

### 포함되는 정보
- clip 이름
- 길이(duration)
- 재생 tick 정보
- channel 목록

### 포함하지 말아야 하는 정보
- 현재 재생 시간
- 현재 키프레임 인덱스
- loop 중인지 여부에 따른 런타임 상태
- 현재 bone matrices

이런 값들은 clip 자체의 속성이 아니라,  
**“누가 이 clip을 지금 어떻게 재생 중인가”** 에 해당하는 상태이므로 `Animator`에 둔다.

---

## 7. ANIMATION_CHANNEL_ENTRY

`ANIMATION_CHANNEL_ENTRY`는 특정 bone 하나에 대응되는 트랙이다.

### 역할
- 어떤 bone을 움직이는지 식별
- 해당 bone의 keyframe들을 보관

### bone 식별 방식
보통 다음 두 가지를 함께 둘 수 있다.

- bone name
- bone index

### 설계 의도
converter 단계에서는 bone name 기반 매핑이 편하고,  런타임에서는 bone index 기반 접근이 빠르다.

그래서 로드가 끝난 뒤에는 bone index를 주로 사용하게 된다.

---

## 8. ANIM_KEYFRAME

`ANIM_KEYFRAME`는 특정 시점의 bone 변환 상태이다.

### 역할
- 트랙 상의 시간 위치 저장
- scale / rotation / translation 저장

### 보간 방식
채널은 현재 트랙 위치를 기준으로
- 이전 keyframe
- 다음 keyframe

사이를 찾아서

- scale → lerp
- rotation → slerp
- translation → lerp

로 보간하고, 최종 bone local transform을 만든다.


---

## 9. ANIMATOR_DATA

`ANIMATOR_DATA`는 애니메이션을 실제로 재생하는 **런타임 컴포넌트/상태**이다.

### 역할
- 현재 어떤 clip을 재생 중인지 보관
- 현재 재생 시간 보관
- 채널별 현재 keyframe index 보관
- 최종 bone matrices 보관
- loop / speed / play state 보관

### 왜 필요한가
같은 모델과 같은 애니메이션 클립을 쓰더라도

- A 오브젝트는 Idle 0.2초
- B 오브젝트는 Idle 1.1초
- C 오브젝트는 Attack 0.4초

처럼 서로 다른 상태를 가질 수 있다.

따라서 현재 시간, 현재 키프레임 인덱스, 최종 bone matrices는  
리소스가 아니라 **오브젝트별 런타임 상태**로 관리해야 한다.

### 결론
`Animator`는 **MeshRenderer 옆에 붙어서 skinned draw를 가능하게 하는 보조 컴포넌트**로 둔다.


---

## 10. MeshRenderer와의 관계

기존 `MESH_RENDERER_DATA`는 이미

- hMesh
- hMaterial
- layer
- flags

등을 가지고 있고,  렌더 시스템은 이 핸들을 보고 접근해서 draw하는 구조이다.

이 구조와 애니메이션을 잘 붙이려면,  
**MeshRenderer는 그대로 두고 Animator를 선택적으로 추가**하는 방향이 좋다.

### 동작 방식
- `MeshRenderer`만 있으면 일반 static/non-anim 렌더
- `MeshRenderer + Animator`가 같이 있으면 skinned 렌더

### 렌더 시 판단 흐름
- `hMesh`가 mesh인지 model인지 판별
- model이면 `MODEL_ENTRY`를 가져옴
- `Animator`가 없으면 non-anim draw
- `Animator`가 있으면 bone matrices를 shader에 바인딩 후 draw

### 장점
- 기존 MeshRenderer 구조를 크게 깨지 않음
- 정적 모델과 애니메이션 모델을 같은 렌더 경로에서 확장 가능
- 책임 분리가 명확함

---

## 11. Converter 파이프라인과의 관계


### Converter 역할
- FBX를 Assimp로 로드
- mesh / model / skeleton / animation 정보를 추출
- 바이너리 + JSON/meta로 저장

### 엔진 역할
- 굽혀진 결과물만 로드
- GUID를 통해 material 등 외부 리소스를 resolve
- `MESH_ENTRY`, `MODEL_ENTRY`, `SKELETON_ENTRY`, `ANIMATION_CLIP_ENTRY` 생성

### 중요한 점
애니메이션/본 정보는 `MESH_ENTRY`에 우겨 넣기보다,  **model 계열 리소스와 함께 관리**한다.

즉 converter는 단순히 “정점/인덱스만 굽는 툴”이 아니라,
애니메이션 가능한 모델에 필요한 모든 정적 리소스를 굽는 툴이 된다.


---

## 12. 핵심 결론

이 설계의 핵심 요약은 다음과 같다.

- `MESH_ENTRY`는 계속 **저수준 draw unit**이다.
- 애니메이션 모델은 `MODEL_ENTRY`에서 다룬다.
- `SKELETON_ENTRY`는 bone들의 집합과 계층 구조이다.
- `ANIMATION_CLIP_ENTRY`는 clip의 **정적 데이터**만 가진다.
- 현재 시간, 현재 키프레임, 최종 bone matrices는 `ANIMATOR_DATA`에 둔다.
- `MeshRenderer`는 유지하고, `Animator`를 옆에 붙여서 skinned draw를 확장한다.

즉,  
**리소스는 공유하고, 재생 상태는 오브젝트별로 분리하는 것**이 가장 중요하다.
