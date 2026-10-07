# Engine 포트폴리오 분석·리뷰 로드맵

기준: 2026-09-22, `b337e31`의 Engine 소스. 이 문서는 분석할 범위를 정하는 체크리스트이며 전체 코드의 무결성 검증 보고서가 아니다. 핵심 구조와 주요 함수 본문을 추적했고, 주변 시스템은 선언·주요 진입점 중심으로 범위를 확인했다. 빌드, 실행, GPU 캡처, 성능 측정은 하지 않았다. 소스 코드는 변경하지 않았다.

Engine/Public, Engine/Private, Engine/Default를 중심으로 한다. Client는 스크립트 등록을 실제로 호출하는 경계만 확인했다. 셰이더 파일은 Client/Bin/Assets/Shaders에 있으므로, 추후 렌더링 분석 시 Engine이 사용하는 계약을 확인하는 범위에서 함께 읽어야 한다. Client 게임 로직, Editor 프로젝트 UI 구현, Converter 내부 구현은 이번 범위 밖이다. Engine 내부의 Editor_System은 범위에 포함한다.

## 1. 먼저 잡아야 하는 프로젝트의 구조

이 엔진은 GameObject가 모든 컴포넌트 객체를 직접 소유하고 각각 Update하는 구조로만 설명하면 부정확하다. GameObject의 타입별 슬롯, 컴포넌트 핸들, 타입별 DATA Pool, 접근용 Proxy, 처리용 Processor가 나뉜 구조다. 다만 스크립트 실제 인스턴스는 별도 `new TScript`로 생성된다. 따라서 모든 데이터가 연속 배치되고 모든 가상 호출이 제거된 ECS라고 주장해서는 안 된다.

```text
게임 코드 / 에디터
    ↓ CGameObject::Add_Component<TProxy>()
타입별 슬롯 + componentMask
    ├─ 단일: COMPONENT_HANDLE
    └─ 복수: GROUP_FLAG | GroupID → Primary + Extras
    ↓ COMPONENT_TYPE → PROCESSOR_ID
Processor → Component_Pool<TProxy> → 페이지 안 DATA
    ↑ Proxy는 DATA 포인터 + Handle을 들고 접근

파일 JSON ↔ Component Spec ↔ Processor의 Initialize_From_Spec / Build_Spec ↔ DATA

.script 메타정보 → CScript_Handler::Generate → 등록용 C++
    → ScriptBinder<T>::Build() → GUID별 SCRIPT_VTABLE
    → 런타임 TypeID → SCRIPT_DATA.pState → Awake / Start / Tick / Destroy

Processor의 DRAW_CMD 생성 → Render_System의 레이어 분배·일부 정렬
    → 패스 실행 → 모델의 part 전개 → 실제 material/shader/mesh 바인딩 → Draw
```

**추천 읽기 순서:** ① 한 프레임 → ② GameObject/Handle/Pool/Proxy/Group → ③ Spec/Scene/Prototype → ④ Script 등록·수명 → ⑤ Resource/Render → ⑥ Physics/Animation → ⑦ 나머지 시스템과 제출 검증.

각 항목은 다음 여섯 가지를 설명할 수 있어야 완료다: 해결하려던 문제, 현재 자료구조, 실제 호출 경로, 유지해야 하는 불변조건, 실패·경계 상황, 대안과 비용. 당시 선택한 동기는 소스만으로 확정할 수 없으므로 아래의 설계 이유는 설명 가능한 해석과 실제 기억을 구분한다.

## 2. 현재 소스에서 먼저 바로잡아 둘 사실

| 주제 | 확인한 구현 | 면접에서 주의할 설명 |
|---|---|---|
| 스크립트 템플릿 | Script_Register.h가 타입별 Create/Destroy/Tick 래퍼와 함수 테이블을 구성한다. Client의 Register_AllScripts가 실행 중 등록한다. | ‘컴파일 타임에 모든 전역 인스턴스를 만든다’는 설명과 다르다. 템플릿 인스턴스화, 함수 로컬 static 리플렉션 정보, 런타임 등록, 실제 스크립트 생성 시점을 나눠야 한다. |
| 가상 호출 | IScript에 virtual 함수가 있고 Get_Script<T>는 dynamic_cast를 사용한다. | 함수 포인터 테이블이 있다고 가상 함수·RTTI가 모두 제거된 것은 아니다. 최적화 결과는 별도 확인 대상이다. |
| Spec 분리 | 독립 Spec이 JSON 변환·Clone을 제공하고 Processor가 DATA와 변환한다. | 저장 상태와 런타임 상태를 분리할 수 있지만, 모든 Spec 필드가 실제로 영속 식별자인지는 별도 감사가 필요하다. |
| Component Group | 한 GameObject의 같은 COMPONENT_TYPE을 여러 개 담기 위한 구조다. | 여러 컴포넌트 타입의 조합을 묶는 archetype/query group이 아니다. |
| Pool | 1024 슬롯 페이지를 필요할 때 추가한다. 초기부터 모든 Pool에 한 페이지를 할당하는 것은 아니다. | 연속성은 페이지 내부 DATA에 대한 것. DATA 안 vector/map/pState의 대상까지 연속인 것은 아니다. |
| 렌더 큐 | MeshRenderer는 키를 생성하고 CRender_System은 BLEND/UI만 정렬한다. | 현재 NONBLEND까지 재질 순으로 정렬된다고 설명하면 안 된다. |
| 리소스 재사용 | Load_Mesh/Texture 등에는 GUID 캐시가 있다. Draw 경로는 재질·텍스처·IA 설정을 반복한다. | 로드 중복 방지, GPU 바인딩 생략, draw call 병합은 별개다. |
| Transform 계층 | Bake_World는 자신의 S×R×T를 계산한다. | GameObject 부모·자식 관계만 보고 부모 월드 행렬이 자동 전파된다고 말하면 안 된다. |
| 물리 시간 | Editor PLAY는 프레임마다 고정 스텝 1회, Game은 누산 후 조건 충족 시 1회 실행하고 누산값을 0으로 만든다. Component_System은 전달된 fDT 대신 0.02f를 넘긴다. | ‘일관된 고정 시간 누산 루프’로 소개하기 전에 실행 경로를 정리해야 한다. |

## 3. 상세 학습·리뷰 체크리스트

체크박스는 아직 설명·검증을 끝냈다는 뜻이 아니다. 각 절의 ‘완료 기준’까지 충족한 뒤 체크한다. P0는 다른 시스템을 이해하는 데 선행하는 항목, P1은 주요 기능, P2는 지원 기능·제출 정리다. 결함의 심각도 등급과는 다르다.

### A01. Engine 경계와 한 프레임 — P0

시작점: [Core_System.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/Core_System.cpp:41), [Component_System.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/Component_System.cpp:67), [GameInstance.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/GameInstance.cpp).

- [ ] CCore_System, CGameInstance, 각 SYS_*의 역할과 소유권을 구분한다. 전달만 하는 API와 실제 상태를 가진 시스템을 나눈다.
- [ ] 초기화 순서를 적고, Processor의 Initialize/Late_Initialize를 나눈 이유와 상호 참조 시점을 확인한다.
- [ ] Script → Transform → Environment → RectTransform → UI → Physics → Animator → MeshRenderer → CanvasRenderer → SpriteEffect 순서를 설명한다.
- [ ] Script의 Priority/Update/Late가 전부 Script_Processor::Update 안에서 실행되는 것을 다른 Processor의 LateUpdate와 구분한다.
- [ ] CComponent_System::LateUpdate가 실제 어디서 호출되는지 추적하고, 이름만 있는 단계와 실행되는 단계를 구분한다.
- [ ] PendingDestroy, 씬 재시작, Fixed_Update, Draw 사이 순서를 그려 삭제 직전 데이터를 누가 읽을 수 있는지 확인한다.
- [ ] EDIT/PLAY/PAUSE/Step/Game 경로에서 입력·시간·Transform·물리가 각각 언제 실행되는지 비교한다.
- [ ] 초기화 중 실패와 정상 종료에서 역순 해제가 필요한 의존성, 싱글톤 재생성 가능성, 이벤트 대상 생존 기간을 확인한다.

완료 기준: ‘스크립트가 위치를 바꾸면 언제 화면과 충돌에 반영되는가’를 함수명과 한 프레임 순서로 설명한다.

### A02. Object 식별자와 수명 — P0

시작점: [GameObject_System.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/GameObject_System.cpp:32), [Object_Struct.h](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Public/Object_Struct.h), [Identity.h](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Public/Identity.h).

- [ ] OBJECT_HANDLE, COMPONENT_HANDLE, INSTANCE_UUID, ASSET_GUID, 리소스 uint32_t handle, Script TypeID의 생존 범위와 쓰임을 각각 설명한다.
- [ ] GameObject의 wrapper pool과 data pool을 나눈 이유, 기본 2048개 선할당, free queue, 0번 더미, 풀 고갈 시 동작을 확인한다.
- [ ] wrapper 포인터를 오래 저장했을 때 같은 주소의 새 오브젝트를 가리킬 가능성과 generation handle 저장의 차이를 검토한다.
- [ ] Destroy_Object에서 즉시 하는 일과 Flush_PendingDestroy에서 하는 일을 분리한다. 중복 Destroy와 부모·자식 동시 Destroy를 추적한다.
- [ ] Reset에서 초기화되는 필드와 보존되는 필드를 분류한다. version, enable, proto GUID, 마스크가 재사용 시 올바르게 설정되는지 확인한다.
- [ ] UUID 등록·해제·검색 경로와 중복 UUID 정책을 확인한다. Scene 로드와 일반 생성 경로가 같은 보장을 하는지 비교한다.
- [ ] Layer bucket의 iIndexInLayer와 swap/remove 시 이동된 오브젝트의 인덱스 보정 규칙을 확인한다.
- [ ] CGameObject의 자체 상태와 GAMEOBJECT_DATA 상태가 중복되는 부분에서 label/mask/enable/clone의 기준값을 확인한다.

완료 기준: 생성 → 참조 저장 → 삭제 요청 → 해제 → 같은 슬롯 재사용을 추적하고, 옛 참조가 거부되는 지점과 거부되지 않는 지점을 구분한다.

### A03. Component DATA / Proxy / Processor — P0

시작점: [CComponent_Proxy_Base.h](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Public/CComponent_Proxy_Base.h), [Component_Processor_Impl.h](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Public/Component_Processor_Impl.h), [Component_System.inl](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Public/Component_System.inl).

- [ ] DATA가 상태를 소유하고 Proxy가 접근하며 Processor가 반복 처리하는 분리의 이점과 복잡성 비용을 설명한다.
- [ ] CRTP의 DATA_T, TProxy_T, KType가 제공하는 type alias/static constexpr와 실제 컴파일 타임 검사 범위를 확인한다.
- [ ] Proxy 값 복사가 컴포넌트 복제인지 같은 DATA를 가리키는 view 복제인지 설명한다. virtual 소멸자가 Proxy 크기에 미치는 영향도 측정 후보로 둔다.
- [ ] Proxy::Is_Valid가 포인터와 0 아닌 핸들만 검사하는 점, Pool의 version 검사와의 차이를 확인한다.
- [ ] Get_Proxy<TProxy>(eComType, handle)의 템플릿 타입과 런타임 eComType 불일치를 누가 막는지 확인한다.
- [ ] enum → g_TypeToProcessorIndex → Processor 배열 연결과, 매핑은 있지만 생성되지 않는 AUDIO 슬롯 등의 지원 상태를 점검한다.
- [ ] 단일 타입 Processor 템플릿과 Physics/UI/Environment처럼 복수 Pool을 다루는 Processor를 비교한다.
- [ ] operator->/_Data로 직접 수정할 때 dirty, enable 이벤트, 참조 재해결을 우회하는 경우를 찾는다.

완료 기준: 새 컴포넌트 한 종류를 추가할 때 수정할 enum·매핑·Proxy·Processor·Spec·Factory 목록을 정확히 적는다.

### A04. Component Pool 메모리·핸들 — P0

시작점: [Component_Pool.h](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Public/Component_Pool.h), [Component_Struct.h](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Public/Component_Struct.h), [Engine_Typedef.h](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Public/Engine_Typedef.h:52).

- [ ] global index의 page = index >> 10, offset = index & 1023를 예제로 설명한다. 핸들의 하위 20비트는 페이지 번호 자체가 아닌 전역 슬롯 인덱스다.
- [ ] 20비트 index, 11비트 version, 슬롯의 최상위 GROUP_FLAG를 구분하고 각 한계와 예약값을 계산한다.
- [ ] 페이지 내부 byte storage, alignas, placement new, destroy_at가 필요한 이유와 비자명 DATA의 수명을 설명한다.
- [ ] 페이지별 unique_ptr 때문에 페이지 vector가 재할당돼도 DATA 주소가 유지되는 이유와, 슬롯 재사용에서는 그 주소가 안전한 참조가 아닌 이유를 구분한다.
- [ ] freeIndices LIFO 재사용, m_iNextIndex 증가, 페이지 추가, 첫 슬롯 0 예약에 따른 첫 페이지 사용 가능 수를 계산한다.
- [ ] 11비트 version 순환 후 오래된 handle이 다시 일치하는 경우와 index 표현 범위 초과를 검토한다.
- [ ] 할당 bitset, version 배열, DATA의 bEnable의 역할을 구분한다. allocated와 enabled는 같은 뜻이 아니다.
- [ ] Deallocate의 콜백 → DATA 소멸 → version 증가 → free list 순서와 콜백 재진입 시 상태를 검토한다.
- [ ] Pool 소멸자가 Deallocate 이벤트 없이 DATA 소멸만 수행하는 경로에서 pState·외부 버퍼 등 Processor 정리 책임이 남는지 확인한다.
- [ ] 타입별 sizeof(DATA), 페이지 수, live 수, peak 수, reserved bytes, 빈 슬롯 방문 수를 계측할 위치를 정한다.

완료 기준: 단순 ‘캐시 친화적’ 주장 대신 페이지 메모리 추산과 실제 순회 비용을 설명한다. 1023/1024/1025개 할당, 삭제·재할당, 오래된 handle과 Proxy를 따로 검증한다.

### A05. Component Group과 타입 슬롯 — P0

시작점: [GameObject.inl](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Public/GameObject.inl:9), [ComponentGroup_Manager.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/ComponentGroup_Manager.cpp:17), [GameObject.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/GameObject.cpp:25).

- [ ] 고정 타입 슬롯 하나로 단일 컴포넌트를 바로 조회하고, 복수일 때만 외부 Group을 쓰는 선택을 설명한다.
- [ ] 빈 슬롯 → 단일 handle → GROUP_FLAG|ID → Extras 추가 과정을 값 단위로 따라간다.
- [ ] UNIQUE_MASK의 단일 허용 타입과 복수 허용 타입을 적는다. Add_Component가 기존 컴포넌트를 반환하는 경우를 확인한다.
- [ ] Get_Component가 Primary를 반환하고 Get_Components가 새 vector를 만드는 API 의미와 비용을 설명한다.
- [ ] Primary 삭제 후 Extra 승격, Extra 삭제, 한 개 남을 때 단일 슬롯으로 축소, 모두 삭제 상태를 추적한다.
- [ ] GroupID 0이 유효한 경우와 빈 슬롯 0의 구분, free group 재사용, Group의 generation 부재를 검토한다.
- [ ] 개별 Remove_Component와 전체 Remove_Components의 슬롯·componentMask 정리가 동일한지 확인한다.
- [ ] Group vector 확장으로 Get_Group 참조가 무효화되는 조건, 삭제 콜백에서 새 그룹을 만들 때의 영향을 확인한다.

완료 기준: 0→1→2→3→2→1→0 상태 전이표를 만든다. Primary/Extra 삭제, 없는 handle 삭제, 전체 삭제 후 재추가를 각각 검증한다.

### A06. Component Spec을 별도 구조체로 둔 이유 — P0

시작점: [Spec_Struct.h](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Public/Spec_Struct.h), [Component_Spec.h](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Public/Component_Spec.h), [Transform_Processor.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/Transform_Processor.cpp:22).

- [ ] Spec=저장·복제할 설정, DATA=실행 상태, Proxy=접근 수단이라는 설명이 각 컴포넌트에서 실제로 성립하는지 필드별로 확인한다.
- [ ] Transform의 TRS와 재계산할 matWorld, Script GUID와 pState, Mesh GUID와 GPU 리소스 handle을 비교 예제로 삼는다.
- [ ] 컴포넌트 없이 JSON을 읽고 보관할 수 있는 이점, Prototype/Scene에서 공유하는 이점과 변환 코드 중복 비용을 설명한다.
- [ ] COMPONENT_SPEC_BASE의 virtual Get_Type/Clone/ToJson/FromJson과 unique_ptr 소유권, Bundle의 move-only 정책을 확인한다.
- [ ] JSON → concrete Spec → factory → Add_Component → Initialize_From_Spec → DATA의 순서를 추적한다.
- [ ] DATA → Build_Spec → Bundle → JSON 역방향에서 누락·기본값 보정·정보 손실을 찾는다.
- [ ] enum 숫자 직렬화의 호환성, 필수/선택 키, 타입 검사, 범위 검사, float 비정상값, 누락 에셋 처리 정책을 확인한다.
- [ ] MeshRenderer의 OverrideMaterials, SpriteEffect의 PerObjectParams처럼 런타임 숫자 handle을 저장하는 필드의 실행 간 안정성을 검토한다.
- [ ] 초기화 실패 시 이미 Add_Component로 만든 슬롯을 되돌리는지 확인한다. 원래 존재하던 단일 컴포넌트 재초기화와 신규 생성 실패를 구분한다.
- [ ] 컴포넌트에 직접 Serialize를 둘 대안, 별도 serializer 대안과 비교하고 현재 규모에서 분리 유지 여부를 판단한다.

완료 기준: Transform, Script, MeshRenderer 각 하나씩 저장→프로세스 재시작→로드→재저장 결과를 비교한다. 같은 실행 안에서만 왕복하는 것으로 handle 저장 안전성을 판단하지 않는다.

### A07. Scene / Prototype / 참조 복구 — P0

시작점: [Scene_Handler.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/Scene_Handler.cpp:376), [Prototype.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/Prototype.cpp:53), [Prototype_Handler.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/Prototype_Handler.cpp).

- [ ] Scene의 INSTANCE_UUID와 Prototype의 ASSET_GUID가 다른 이유, 같은 Prototype 여러 인스턴스의 식별 방법을 설명한다.
- [ ] 오브젝트 일괄 생성 → 부모 연결 → Spec 적용 → 스키닝/부착 참조 → 스크립트 UUID 복구 순서의 필요성을 설명한다.
- [ ] overrides라는 이름이 실제 차이만 저장하는지, 전체 컴포넌트 상태를 저장하는지 Build_SceneSpecs부터 확인한다.
- [ ] Scene 로드에서 protoGuid를 기록하는 것과 Prototype::Clone을 호출하는 것을 구분한다. 이름만으로 prefab inheritance라고 설명하지 않는다.
- [ ] Create_Spec_By_Type switch와 Processor의 두 factory 등록이 중복 관리되는 이유·누락 가능성을 점검한다.
- [ ] 부모 순환, 없는 부모 UUID, 중복 UUID, 빈 Spec, 알 수 없는 타입에서 실패 정책을 확인한다.
- [ ] Scene 교체 실패 시 CScene 포인터뿐 아니라 생성·삭제된 GameObject, 리소스, 이벤트 상태까지 복구되는지 확인한다.
- [ ] Prototype 재귀 Clone에서 자식 UUID 재발급, 내부 ObjectRef remap, 부모 연결, enable/mask, 실패 중간 정리를 검토한다.
- [ ] Scene 복원·Restart·Editor Play/Stop이 같은 경로인지 비교하고 잔존 구독·Tick·리소스를 확인한다.

완료 기준: 서로 참조하는 오브젝트 두 개와 부모·자식이 포함된 작은 Scene/Prototype을 만들고 복제·저장·재실행·실패 로드를 검증한다.

### A08. 스크립트 등록과 컴파일 타임/런타임 경계 — P0

시작점: [Script_Register.h](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Public/Script_Register.h:10), [Script_Handler.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/Script_Handler.cpp:54), [Script.h](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Public/Script.h).

- [ ] C++ 클래스, .script 파일, .meta GUID, 생성된 등록 코드가 각각 무엇을 제공하는지 설명한다.
- [ ] Script_Create<T>/Destroy<T>/Tick<T>가 정적 타입을 void* 기반 공통 함수 서명에 연결하는 과정을 설명한다.
- [ ] ScriptBinder<T>::Build는 constexpr 함수가 아니라 실행 중 함수 테이블 값을 만드는 함수임을 구분한다.
- [ ] CScript_Handler::Generate의 입력·호출 시점·출력 파일과 재빌드가 필요한 변경을 정리한다.
- [ ] 실제 경계인 Client::Register_AllScripts 및 MainApp 초기화 순서에서 에셋 스캔·등록·Scene 로드 순서를 확인한다.
- [ ] Engine이 구체적인 Client 클래스를 알 필요 없이 함수 포인터로 생성·실행할 수 있는 이점과 DLL/빌드 의존성을 설명한다.
- [ ] 수동 factory, 자기 등록 static 객체, RTTI 기반 접근과 비교한다. 이 프로젝트가 사용하지 않는 자동 등록 방식을 현재 구현으로 설명하지 않는다.
- [ ] 중복 GUID 등록, 등록 누락, 클래스명·헤더명 변경, 생성 코드의 절대경로, 다른 PC에서 재생성 가능성을 검토한다.
- [ ] IScript virtual 함수가 남는 이유, TScript→void*→IScript* 변환에 대한 상속 배치 가정, 다중 상속 확장 제한을 점검한다.

완료 기준: ‘새 CMyScript를 추가한 뒤 Update가 처음 호출될 때까지’를 생성 코드·컴파일·런타임 등록·state 생성 단계로 나눠 설명한다.

### A09. 스크립트 수명과 Tick 목록 — P0

시작점: [Script_Processor.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/Script_Processor.cpp:22), [Script_Processor.h](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Public/Script_Processor.h).

- [ ] GUID→TypeID map, TypeID→GUID vector, TypeID→VTABLE, SCRIPT_DATA→pState, Tick 목록의 연결을 그린다.
- [ ] Create_State_If_Needed에서 heap 인스턴스 생성·Owner 설정·PendingAwake 등록·Tick 등록 순서를 확인한다.
- [ ] Awake 지연이 노출 필드 로드 및 오브젝트 참조 복구 완료와 어떤 관계인지 설명한다.
- [ ] Awake/Start 정확히 한 번, disabled 상태의 Awake, enable 재전환, GUID rebind 시 초기화 플래그 정책을 검토한다.
- [ ] Start가 Priority 루프에 연결된 이유와 모든 기본 Tick 함수가 등록되는 구조를 설명한다. no-op Tick 비용도 측정 대상으로 둔다.
- [ ] 자기 자신/다른 스크립트를 Disable·삭제·추가하는 콜백에서 Tick vector의 swap-pop·재할당·중복 등록을 검토한다.
- [ ] PendingAwake 순회 중 새 스크립트 생성, flush 전 GUID rebind, 파괴된 스크립트의 pending 항목을 검토한다.
- [ ] SCRIPT_CTX.hObject와 IScript::m_hObject 중 실제 사용되는 소유자 정보, 콜백 직전 context 갱신 여부를 확인한다.
- [ ] Deallocate/Rebind/엔진 종료에서 vt.Destroy가 몇 번 호출되는지, pState와 Tick 잔존 여부를 확인한다.
- [ ] Build_Spec의 GUID 역검색과 이미 존재하는 m_TypeIDToGUID를 비교하되, 최적화 전 프로파일·단순성 이득을 따진다.

완료 기준: 생명주기 로그로 생성·Awake·Start·각 Tick·Disable·Rebind·Destroy 순서와 호출 횟수를 증명한다.

### A10. 수동 리플렉션과 노출 필드 — P0

시작점: [ScriptField_Reflection.h](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Public/ScriptField_Reflection.h), [Script.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/Script.cpp).

- [ ] SCRIPT_FIELDS_* 매크로가 클래스명·필드명·enum type·offset을 수집하는 방식과 저장 위치를 설명한다.
- [ ] 함수 로컬 static s_Info와 수동 s_bInitialized 플래그를 전역 스크립트 인스턴스와 구분한다. 초기화 재진입·동시 호출 정책을 확인한다.
- [ ] offsetof를 virtual 함수가 있는 스크립트 타입에 적용하는 컴파일러 가정과 경고를 확인하고, 이식을 주장하지 않는다.
- [ ] IScript 기준 주소에 offset을 더하는 규칙과 상속 배치가 일치하는지 확인한다.
- [ ] 저장·로드 switch가 enum의 모든 필드를 처리하는지, 필드명 변경·삭제·타입 변경 시 이전 데이터 정책을 확인한다.
- [ ] SCRIPT_OBJECT_REF가 UUID와 handle을 함께 가진 이유, 로드 후 handle 무효화·참조 복구·대상 삭제 시 동작을 확인한다.
- [ ] CHAR32 길이 제한, 널 종료, UTF-8 중간 절단과 숫자/배열 원소 타입 오류를 검토한다.
- [ ] 빌드되지 않아 pState가 없는 스크립트를 Editor에서 로드·재저장할 때 exposedFields까지 보존되는지 검증한다.

완료 기준: 필드 추가·이름 변경·잘못된 JSON·참조 대상 누락·미등록 타입의 저장 결과를 확인한다.

### A11. Asset Registry와 Resource System — P0

시작점: [Asset_Registry.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/Asset_Registry.cpp:28), [Resource_System.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/Resource_System.cpp:42), [Resource_System.h](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Public/Resource_System.h).

- [ ] Asset Registry의 파일·GUID 목록과 Resource System의 CPU/GPU 실체 저장소를 분리한 이유를 설명한다.
- [ ] 경로 정규화, .meta 생성, GUID 유지, Built-in 에셋, 파일 이동·복사·중복 GUID·없는 에셋 처리를 확인한다.
- [ ] GUID 캐시 조회 → 파일 읽기 → GPU 생성 → vector 등록 → handle 반환의 성공·실패 경로를 타입별로 비교한다.
- [ ] Mesh/Model handle의 상위 태그 비트와 index mask, 0번 더미 정책을 확인한다. 최초 로드와 캐시 히트 반환값이 같은지 검증한다.
- [ ] vector 안 entry 포인터를 외부에서 보관할 때 후속 로드로 재할당되는 경우와 ComPtr가 가리키는 COM 객체의 수명을 구분한다.
- [ ] 공유 MATERIAL_ENTRY를 수정하는 것과 per-object parameter를 수정하는 것의 영향을 구분한다.
- [ ] CResource_System::Clear가 빈 구현인 상태에서 Scene 교체 중 유지하는 자원과 끝까지 누적되는 자원을 확인한다.
- [ ] GUID 로드와 MATERIAL_ENTRY 직접 등록의 중복 방지 차이, material 조합 캐시가 실제 사용되는지 확인한다.
- [ ] 경로와 GUID 문자열 변환의 길이·널 문자·인코딩, GUID 해싱의 메모리 접근 가정을 검토한다.

완료 기준: 같은 GUID 두 번 로드, 잘못된 GUID, 씬 반복 전환에서 반환 handle·실제 GPU 생성 수·메모리 추이를 기록한다.

### A12. 파일 로더·GPU 리소스 생성 — P1

시작점: [Load_Helper.h](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Public/Load_Helper.h), [MeshBuilder.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/MeshBuilder.cpp), [ShaderBuilder.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/ShaderBuilder.cpp), [MaterialBuilder.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/MaterialBuilder.cpp).

- [ ] Mesh/Model/Skeleton/Animation 바이너리 포맷의 header, magic, version, stride, count, index 범위를 정리한다.
- [ ] 파일 count를 그대로 vector 크기·ByteWidth에 쓰는 경로에서 잘린 파일, 비정상 count, 곱셈 overflow, read 실패를 확인한다.
- [ ] 구조체 sizeof로 쓰고 읽는 포맷의 padding·컴파일 설정 의존성과 포맷 버전 정책을 확인한다.
- [ ] Mesh/Model의 CPU 데이터와 GPU buffer 중 생성 후 유지할 데이터 및 bounds 계산 주체를 확인한다.
- [ ] Shader 빌드/로드, Effects pass 조회, input layout 생성과 vertex declaration 연결을 설명한다.
- [ ] Texture의 DDS/WIC/TGA 처리와 SRV 포맷·sRGB·mip 보장 범위를 확인한다.
- [ ] 빌더 도중 실패했을 때 ComPtr와 raw COM 포인터의 해제 및 반쯤 생성된 entry 등록 여부를 확인한다.

완료 기준: 정상 파일과 header만 남은 파일·잘못된 stride/count 파일에서 실패가 호출자까지 전달되는지 확인한다.

### A13. DRAW_CMD 생성·정렬·실행 — P0

시작점: [MeshRenderer_Processor.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/MeshRenderer_Processor.cpp:82), [CRender_System.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/CRender_System.cpp:313), [Render_Struct.h](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Public/Render_Struct.h).

- [ ] DRAW_CMD로 수집과 실행을 나눈 이유, command 값·handle·raw pointer 필드의 프레임 수명 계약을 설명한다.
- [ ] Mesh 키의 layer 4비트/material 32비트/mesh 28비트 배치와 Model 태그 및 큰 handle의 정보 손실을 검토한다.
- [ ] 현재 정렬되는 BLEND/UI와 정렬되지 않는 NONBLEND/Shadow를 정확히 구분한다.
- [ ] BLEND의 내림차순 정렬 대상 키에 실제 카메라 깊이가 있는지 확인한다. material 순서가 back-to-front 정렬을 대신하지 못하는 경우를 재현한다.
- [ ] 모델 하나를 command 하나로 수집한 뒤 draw 단계에서 part별 mesh/material로 전개하는 구조가 정렬 효율에 미치는 영향을 설명한다.
- [ ] override material → part material → command material fallback 순서와 실제 바인딩 값이 키에 반영되는지 확인한다.
- [ ] Outline command의 material 교체와 sortKey+1, Shadow 레이어 변경, forced pass의 계약을 확인한다.
- [ ] m_AllDrawCmds 구축 후 m_LayerCmds에 주소를 담는 시점과 이후 vector 변경 금지를 확인한다.
- [ ] 로드 캐시 / shader 변수 조회 캐시 / 바인딩 캐시 / 배칭 / instancing을 구분하고 각각 무엇을 줄이는지 설명한다.
- [ ] 동일 재질이라도 transform, bones, per-object override, pass, blend 상태가 다른 경우 캐시 재사용 조건을 정의한다.

완료 기준: 동일 모델 여러 개·서로 다른 override·겹치는 투명 Mesh에서 키, 최종 draw 순서, material 변경 수, draw 수를 비교한다.

### A14. 렌더 패스와 DX11 상태·리소스 계약 — P1

시작점: [CRender_System.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/CRender_System.cpp:353), [Graphic_Device.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/Graphic_Device.cpp), [Render_Context.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/Render_Context.cpp).

- [ ] SKY/Priority → Static/Dynamic Shadow → GBuffer → Light → Combined → PostProcess/Composite → BLEND → SpeedLine → UI 순서를 설명한다.
- [ ] 패스별 입력 SRV, 출력 RTV/DSV, 포맷, 채널 의미, clear 값, viewport를 한 표로 정리한다.
- [ ] GBuffer normal/depth의 저장 의미와 광원·합성 단계의 복원 행렬 및 좌표 공간을 실제 HLSL과 대조한다.
- [ ] UNORM과 FLOAT 선택, scene/postprocess 출력 범위, sRGB 변환 시점을 확인한다. 다른 프로젝트의 HDR/PBR 기능을 이 엔진에 있다고 가정하지 않는다.
- [ ] 한 텍스처가 SRV/RTV로 전환될 때 unbind 지점과 DX11 debug layer 경고를 확인한다.
- [ ] Pass Apply 전후 blend/depth/rasterizer 상태의 실제 최종값과 Outline/Text/SpriteBatch 이후 복구를 검증한다.
- [ ] material params 후 per-object params 적용 순서, 없는 파라미터가 이전 draw의 shader 값으로 남는 경우를 확인한다.
- [ ] C++ 변수명·shader 변수명, matrix transpose, float/vector/array 크기와 필요 시 cbuffer 정렬을 대조한다.
- [ ] resize/최소화/SceneView 크기 변경에서 texture·view·projection·UI 전역값 재생성 및 0 크기 처리를 확인한다.

완료 기준: 실제 한 프레임 GPU 캡처에 패스 표를 대응시키고 렌더 타깃 한 장의 생산자부터 최종 소비자까지 설명한다.

### A15. 컬링·그림자·카메라 — P1

시작점: [MeshRenderer_Processor.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/MeshRenderer_Processor.cpp:1216), [Environment_Processor.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/Environment_Processor.cpp), [CRender_System.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/CRender_System.cpp:1679).

- [ ] local bounds를 world bounds로 옮기는 계산과 비균일·음수 scale 처리, 모델 part bounds 결합을 확인한다.
- [ ] animated mesh/PARTS/ATTACH bounds가 변형 후 형상을 감싸는지와 보수적 컬링 정책을 확인한다.
- [ ] Shadow command를 카메라 컬링 전에 넣는 이유와 광원 기준 추가 컬링 가능성을 설명한다.
- [ ] Particle을 현재 컬링하지 않는 이유와 도입하려면 필요한 lifetime/velocity 기반 bounds를 검토한다.
- [ ] static shadow 재사용을 무효화해야 하는 광원 이동·static object 변경·Scene 교체·resize 조건을 찾는다.
- [ ] shadow pass index=2와 material shader의 실제 pass 구성이 맞는지 확인하고 bias·해상도·깊이 정밀도를 분석한다.
- [ ] active camera 선택, debug camera와 cinematic camera 전환, view/proj dirty 계산의 갱신 순서를 확인한다.

완료 기준: 화면 밖 물체의 그림자, 화면 가장자리 애니메이션, static 그림자 갱신, 카메라 전환을 각각 검증한다.

### A16. Transform·공간·수학 규약 — P0

시작점: [Transform.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/Transform.cpp), [Transform_Processor.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/Transform_Processor.cpp:76), [Engine_MathDX.h](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Public/Engine_MathDX.h), [GameObject.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/GameObject.cpp:177).

- [ ] row-vector S×R×T, DirectX 좌표 규약, degree/radian, quaternion 곱 순서를 예제로 설명한다.
- [ ] Translate/Rotate의 LOCAL/WORLD 의미와 ‘LOCAL=부모 좌표계’인지 ‘자신의 회전축’인지 구분한다.
- [ ] 부모·자식 관계의 조직화와 Transform 합성의 지원 범위를 실제 코드로 설명한다.
- [ ] dirty를 세우는 API와 raw DATA 쓰기를 대조하고 같은 프레임 내 matWorld 최신성을 확인한다.
- [ ] 물리 적분 이후 world bake, 카메라 submit, animator bone 계산, 렌더 command 생성 사이 동기화를 확인한다.
- [ ] Quaternion 정규화, 0벡터 LookAt, 평행 방향, scale 0, 음수 scale의 정책과 normal 변환을 확인한다.
- [ ] Engine_Math와 Engine_MathDX의 사용처·행렬 규약 차이를 확인하고 이름만으로 통합하지 않는다.

완료 기준: 로컬 이동·월드 회전·부모 이동·물리 이동 각각의 기대 좌표와 실제 화면 결과를 비교한다.

### A17. 물리 시간·적분·Rigidbody·SpringJoint — P1

시작점: [Physics_Processor.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/Physics_Processor.cpp:51), [Rigidbody.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/Rigidbody.cpp), [Rigidbody_Builder.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/Rigidbody_Builder.cpp), [SpringJoint.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/SpringJoint.cpp).

- [ ] Spring → 힘 누적 → 힘 적분 → damping → 속도 적분 → 충돌 → 해결 → kinematic 속도 초기화 순서를 설명한다.
- [ ] Editor/Game/Step의 fixed step 호출 횟수 차이와 누산 잔여 시간 삭제가 물리 시간에 미치는 영향을 계산한다.
- [ ] 힘·충격량·속도 변경 API의 단위, 질량과 dt 적용 위치, 한 번 적용/지속 적용 차이를 설명한다.
- [ ] static/kinematic/dynamic, 중력 사용, inverse mass/inertia, 회전·위치 lock의 계약을 확인한다.
- [ ] 선속도·각속도 적분 방식과 quaternion 갱신, damping의 dt 의존성을 확인한다.
- [ ] Spring rest/min/max length, stiffness/damping, anchor 공간, 양 끝 힘 방향과 고정된 끝의 처리를 설명한다.
- [ ] 대규모 dt, 매우 작은 질량, 강한 스프링, 빠른 이동에서 불안정·관통이 생기는 조건을 파악한다.
- [ ] 현재 contact solver 반복이 1회인 선택과 안정성/비용 관계를 정리한다. CCD/substep이 있다고 추정하지 않는다.

완료 기준: 동일 10초 운동을 30/60/144 FPS 및 프레임 지연 조건에서 비교하고, 수정 시 시간 진행과 거리 오차를 수치로 확인한다.

### A18. Broad Phase / Narrow Phase / 충돌 해결 — P1

시작점: [Collision_Detector.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/Collision_Detector.cpp:332), [Uniform_Grid.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/Uniform_Grid.cpp), [Collider_Proxy_Builder.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/Collider_Proxy_Builder.cpp), [Solver.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/Solver.cpp).

- [ ] Collider DATA와 COLLIDER_PROXY_DATA의 차이, world collider snapshot 생성 시점과 contact에 저장한 포인터 수명을 확인한다.
- [ ] static grid 등록·재구축, 경계 clamp, cell size=60과 world 범위 선택의 근거를 확인한다.
- [ ] 현재 Broad Phase에서 dynamic-static은 grid 조회, dynamic-dynamic은 이중 루프라는 점을 구분한다. Dynamic Grid API 존재만으로 사용된다고 말하지 않는다.
- [ ] AABB 선별, pair 중복 제거, mask/discard mask, 같은 오브젝트의 여러 Collider 처리 규칙을 확인한다.
- [ ] Sphere/Box/Plane 조합의 검출 테이블, A/B 반전 시 contact normal 방향, 접촉점·침투 깊이 계산을 설명한다.
- [ ] OBB 분리축, 평행 축 오차, 박스 내부 sphere, 정접과 작은 overlap 같은 경계를 검토한다.
- [ ] impulse의 상대 접촉점 속도·각속도·관성·반발계수·마찰·penetration correction을 코드의 항과 대응시킨다.
- [ ] trigger는 contact 이벤트를 만들되 solver에서 제외되는 경로와 양쪽 static·kinematic 조합을 확인한다.
- [ ] 셀 크기와 오브젝트 수 변화에 대해 후보 pair 수, Narrow Phase 수, 실제 contact 수, 소요 시간을 분리 계측한다.

완료 기준: 형태 조합별 최소 충돌 장면과 상자 쌓기·경사면·정적 경계 밖 장면에서 normal, contact point, pair 수를 관찰한다.

### A19. 충돌 이벤트·Raycast·일반 이벤트 — P1

시작점: [Physics_Processor.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/Physics_Processor.cpp:516), [Raycast.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/Raycast.cpp), [Event.h](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Public/Event.h), [Event_System.h](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Public/Event_System.h).

- [ ] 이전/현재 collision pair 집합으로 Enter/Stay/Exit를 구분하는 규칙과 Physics/Update 사이 이벤트 발행 시점을 확인한다.
- [ ] Collider 비활성화·삭제·Scene 전환에서 Exit 정책과 이미 파괴된 수신자 접근 가능성을 검토한다.
- [ ] screen→viewport→NDC→world ray 변환과 Editor/Game viewport 원점·크기 차이를 설명한다.
- [ ] ray 방향 정규화, 최대 거리, 내부 출발, 평행 ray, nearest/all hit, 결과 정렬, mask 조건을 검토한다.
- [ ] Event의 타입 안전한 템플릿 인자와 EVENT_DATA enum 기반 캐스팅을 사용하는 전역 이벤트를 비교한다.
- [ ] ListenerID와 raw instance 캡처의 수명을 확인하고 구독자가 파괴될 때 누가 해제하는지 추적한다.
- [ ] Invoke 중 자기 해제·다른 listener 추가/삭제·Clear·재귀 Trigger의 iterator 및 호출 순서 정책을 검토한다.
- [ ] 전역 이벤트는 동기 호출인지 지연 큐인지 설명하고, 이벤트 콜백에서 Scene 교체·오브젝트 생성이 가능한 범위를 정한다.

완료 기준: 자기 구독을 제거하는 listener, 콜백에서 오브젝트를 삭제하는 collision, viewport resize 후 picking을 검증한다.

### A20. Animation·Skinning·Parts·Attach — P1

시작점: [Animator_Processor.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/Animator_Processor.cpp:125), [Skeleton.h](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Public/Skeleton.h), [MeshRenderer_Processor.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/MeshRenderer_Processor.cpp:423).

- [ ] 공유 clip/skeleton/model과 인스턴스별 track position, key index, local/combined/final matrices를 구분한다.
- [ ] ticks-per-second와 초 변환, loop/end, 큰 dt로 여러 경계 통과, play speed, clip 변경 시 캐시 초기화를 확인한다.
- [ ] position/scale 보간과 quaternion 보간, 동일 시간 키·키 하나·없는 channel의 기본값을 설명한다.
- [ ] local → parent combined → inverse bind/offset → final bone matrix 순서와 좌표 규약을 확인한다.
- [ ] clip별 loop와 전환 pair별 blend duration map, 현재/다음 clip의 시간 진행 및 중간 재전환 정책을 확인한다.
- [ ] PARTS에서 bone 이름 remap을 하는 이유, 없는 bone, skeleton이 다른 부위, matrix vector 수명을 확인한다.
- [ ] ATTACH의 bone/world/offset 합성과 일반 skinning의 차이, 부모 Animator 탐색·삭제·교체 시 재해결을 확인한다.
- [ ] CPU pose 평가와 GPU bone array 업로드 비용을 분리한다. 보이지 않는 캐릭터의 animation을 생략해도 되는지 이벤트/부착 의존성을 먼저 확인한다.
- [ ] Animation_Event 정의와 실제 발행·구독 경로를 확인하고, 선언만 있는 기능과 실행되는 기능을 구분한다.

완료 기준: 두 clip 블렌드, 동일 skeleton 부위 교체, 무기 bone 부착, loop 경계와 큰 dt를 pose/호출 로그로 검증한다.

### A21. Particle·SpriteEffect·Line — P1

시작점: [MeshRenderer_Processor.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/MeshRenderer_Processor.cpp:720), [SpriteEffect_Processor.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/SpriteEffect_Processor.cpp), [Line.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/Line.cpp).

- [ ] Particle asset 설정과 PARTICLE_RUNTIME을 나눈 이유, runtime index free list와 생성·재생·정지·반환 순서를 확인한다.
- [ ] spawn rate/수명/최대 개수/loop/prewarm 등 실제 지원 필드를 정리하고 한 프레임 대량 spawn의 정책을 확인한다.
- [ ] particle simulation 공간과 emitter 이동·회전·scale의 관계, billboard 기준 camera를 확인한다.
- [ ] GPU buffer 용량·Map/Unmap·업로드 크기·활성 개수와 할당 실패 처리를 확인한다.
- [ ] SpriteEffect의 row/col/total frame/frame duration, 0값·마지막 프레임·loop 종료·큰 dt 건너뛰기를 검토한다.
- [ ] Line/Trail/SpeedLine의 데이터 생성 책임과 draw submit, 프레임 간 잔존, 투명도·depth 상태를 확인한다.
- [ ] 컴포넌트 해제와 종료 시 Particle buffer·per-object block이 반환되는지, 반복 재생에서 메모리가 안정되는지 검증한다.

완료 기준: 반복 spawn/despawn, 수명 완료, pause/restart에서 CPU/GPU 메모리·활성 수·정지 시각을 비교한다.

### A22. UI 데이터·입력·렌더 순서 — P1

시작점: [UI_Processor.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/UI_Processor.cpp:313), [RectTransform_Processor.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/RectTransform_Processor.cpp), [CanvasRenderer_Processor.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/CanvasRenderer_Processor.cpp).

- [ ] UIImage/UIButton/UIText, CanvasRenderer, RectTransform의 책임과 이미지→Canvas 동기화 방향을 설명한다.
- [ ] UI의 pixel 위치·중심·크기·화면 좌표 변환과 부모 관계·해상도 변경 지원 범위를 확인한다.
- [ ] 이미지·텍스트의 sortZ/visual priority/material 키가 같은 정렬 규약을 쓰는지 확인한다.
- [ ] Button의 Normal/Hover/Pressed/Disabled 전이, 현재 클릭이 mouse-down 기준인 점과 드래그 후 release 정책을 정리한다.
- [ ] 겹치는 버튼을 모두 순회할 때 입력이 여러 개에 전달되는지, 렌더 순서와 hit 우선순위가 일치하는지 검토한다.
- [ ] SceneView 밖으로 마우스가 나가면 early return하는 경우 기존 Hover/Pressed 상태가 남는지 확인한다.
- [ ] clip/scissor 적용, 폰트 크기·한글·텍스트 batching, SpriteBatch가 바꾼 D3D 상태의 복구를 확인한다.
- [ ] 버튼 콜백에서 자신·Canvas·다른 버튼을 파괴하거나 생성했을 때 Pool 순회와 참조 수명이 안전한지 확인한다.

완료 기준: 겹친 버튼·비활성화·viewport 이탈·창 크기 변경·콜백 중 삭제를 검증한다.

### A23. Navigation·Cinematic — P1

시작점: [NavMesh.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/NavMesh.cpp:148), [NavBuilder.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/NavBuilder.cpp), [CinematicSystem.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/CinematicSystem.cpp:149).

- [ ] Nav point/cell/neighbor 포맷과 shared edge 판정, 중복 정점, winding, 이웃 없는 cell 처리를 확인한다.
- [ ] XZ 삼각형 내부 판정의 높이 제한, 시작/목표 cell 검색, mesh 밖 목표 정책을 설명한다.
- [ ] A*의 비용·휴리스틱·open/closed·경로 복구와 도달 불가·동일 cell·재탐색 조건을 분석한다.
- [ ] waypoint 생성·shared edge midpoint·통과 판정의 실제 방식을 읽고, 없는 funnel/회피 기능을 있다고 소개하지 않는다.
- [ ] Cinematic clip의 shot/key/event/shake, 시간 평가·ease·LookAt·카메라 fallback 경로를 설명한다.
- [ ] Play/Pause/Preview/Stop과 큰 dt·seek·재생 시작점에서 event key가 한 번만 실행되는지 검토한다.
- [ ] cinematic camera 소유권과 종료 시 gameplay camera 복원, 강제 shake와 clip shake 중첩을 확인한다.
- [ ] SYS_CINEMATIC.Update가 실제 누구에게 호출되는지 확인하고 Core에 Initialize만 있다고 자동 업데이트를 가정하지 않는다.

완료 기준: 도달 불가 경로·경계 waypoint·shot 전환·pause 후 resume·event 시각 건너뛰기를 각각 검증한다.

### A24. 입력·시간·사운드·로그·Engine 내부 Editor — P2

시작점: [Input_System.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/Input_System.cpp), [Timer_System.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/Timer_System.cpp), [Sound_System.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/Sound_System.cpp), [Editor_System.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/Editor_System.cpp).

- [ ] 이전/현재 key 상태로 Down/Up/Held를 만드는 방식, 포커스 이탈·재획득, mouse 좌표 기준을 확인한다.
- [ ] system DT와 frame DT, QueryPerformanceCounter 사용, pause/slow motion이 각 시스템에 전달되는 경로를 확인한다.
- [ ] Sound channel 재사용, 강제/비강제 재생, loop/BGM, 중복 key, 파일 로드 실패, sound/channel 종료 책임을 확인한다.
- [ ] 사운드 라이브러리가 제공하는 기능과 자체 관리 코드를 구분해 포트폴리오 설명 범위를 정한다.
- [ ] Logger/ISink/CLI_Sink/GUI_Sink의 전달 경로, 필터·보관량·문자열 수명·Release 로그 비용을 확인한다.
- [ ] Engine의 Editor_System이 담당하는 debug camera, picking, scene play/pause/step, nav 편집과 Editor 프로젝트 UI 경계를 설명한다.
- [ ] Debug_Renderer/DebugDraw가 geometry·line을 어떻게 수집·업로드하고 프레임 종료에 비우는지 확인한다.
- [ ] debug 전용 서비스가 Game 실행과 종료 경로에 요구되는지, 테스트를 위해 분리할 실익이 있는지 판단한다.

완료 기준: 입력 포커스·pause/step·반복 사운드·debug draw on/off에서 동작과 비용을 구분한다.

### A25. C++ 수명·오류 정책·제출 재현성 — P0/P2

시작점: [Engine.vcxproj](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Default/Engine.vcxproj), [Engine_Macro.h](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Public/Engine_Macro.h), [Base.cpp](C:/Users/delay/Jusin/AOTTG2_DX11/Engine/Private/Base.cpp).

- [ ] raw pointer/unique_ptr/ComPtr/reference counting 각각의 owner와 borrowed 관계를 실제 멤버별로 표시한다.
- [ ] static_cast/reinterpret_cast/dynamic_cast 사용 목적, 타입 불일치가 가능한 입력 경계와 Release 검사 유무를 확인한다.
- [ ] IF_* 매크로의 Debug/Release 동작과 HRESULT/bool/null/예외 간 오류 전달 일관성을 검토한다.
- [ ] 템플릿·헤더 정의·inline/static·DLL export/import·STL 노출과 runtime library 설정이 빌드 경계에 주는 제약을 설명한다.
- [ ] component Pool 소멸, Processor 파생 멤버 소멸, 이벤트 콜백 대상 소멸 순서가 맞는지 검토한다.
- [ ] 단일 스레드 가정과 실제 동시 작업 여부를 확인한다. Pool/Event/Registry가 thread-safe라고 근거 없이 주장하지 않는다.
- [ ] EngineSDK 복사본과 Engine/Public 원본, UpdateLib.bat, 산출물 복사 경로 중 실제 Client가 참조하는 파일을 확인한다.
- [ ] 절대경로·작업 디렉터리·생성 코드·shader/asset 배포·외부 라이브러리 조건을 다른 경로의 깨끗한 체크아웃에서 검증한다.
- [ ] 선언·빈 구현·주석 처리된 코드·미사용 경로를 실제 제출 기능과 분리한다. 기존 Docs/Engine 문서도 현재 호출 경로와 대조한다.

완료 기준: 다른 경로에서 Debug/Release 빌드 및 대표 Scene 실행을 재현하고, 직접 구현·외부 라이브러리·미완성 범위를 적는다.

## 4. 제출 전에 먼저 검증할 구체적 후보

아래는 소스에서 관찰한 상태에 근거한 우선 검증 목록이다. 런타임 장애·발생 빈도는 이번에 확인하지 않았다. ‘후보’는 수정 완료나 결함 재현을 뜻하지 않는다.

| 우선 | 위치·관찰 | 먼저 할 검증 | 최소 수정 방향 후보 |
|---|---|---|---|
| 1 | Resource_System::Load_Mesh에서 `!pRec` 분기에 들어간 뒤 `pRec->eType`을 읽는다. | 캐시에 없는 미등록 GUID를 전달한다. | null과 잘못된 타입 분기를 분리한다. |
| 1 | 같은 함수 최초 로드 끝에서 `handle & ASSET_TAG_MESH`를 반환하며 해당 상수는 0이다. 캐시 hit는 저장한 handle을 반환한다. | 직접 MESH 타입의 첫 로드/두 번째 로드와 Get_Mesh 결과를 비교한다. Model 경로와 구분한다. | 기존 Make_MeshHandle 또는 index mask 계약에 맞춘다. |
| 1 | Remove_Components는 Pool/Group을 해제하지만 해당 슬롯과 componentMask를 직접 비우지 않는다. | 살아 있는 오브젝트에 전체 삭제→Get/Add/전체 삭제를 반복한다. Destroy 뒤 Reset 경로와 구분한다. | 전체 삭제 API도 슬롯·mask 불변조건을 유지한다. |
| 1 | Proxy::Is_Valid는 Pool의 generation과 allocated를 재검사하지 않는다. | Proxy 보관→컴포넌트 해제→같은 슬롯 재할당→옛 Proxy 접근. | Proxy 수명 계약을 먼저 정하고 장기 참조는 handle 재조회로 제한하거나 검증을 보강한다. |
| 1 | Tick 순회 중 Set_Enable이 같은 vector를 swap-pop 할 수 있고 PendingAwake 순회 중 새 pending 추가도 가능하다. | 첫 Tick에서 자기/다른 script disable, Awake에서 script 생성·rebind. | 필요하다면 변경을 안전한 단계까지 지연한다. 단순 vector 복사가 모든 수명 문제를 해결한다고 가정하지 않는다. |
| 1 | Pool 소멸은 OnDeallocate를 호출하지 않는다. SCRIPT_DATA의 pState는 raw pointer다. | 활성 스크립트를 남긴 종료에서 Create/Destroy 횟수와 누수를 계측한다. | Processor 멤버가 살아 있을 때 명시적 정리한다. Pool 소멸자에서 파생 멤버 콜백을 무조건 호출하는 식으로 고치지 않는다. |
| 1 | Editor/Game 물리 호출 방식과 고정 dt 전달이 다르다. | 같은 운동을 FPS별·Editor/Game/Step별 비교한다. | 잔여 시간을 보존하는 공통 fixed-step 정책을 정의한 뒤 수정한다. |
| 2 | Mesh sortKey에 깊이가 없고 BLEND는 이를 내림차순 정렬한다. | 두 투명 Mesh의 앞뒤를 바꿔 결과와 draw 순서를 확인한다. | 투명도용 깊이 키를 별도 정의한다. 교차하는 투명 표면의 한계도 설명한다. |
| 2 | CResource_System::Clear는 비어 있고 scene load는 중간 생성 작업을 수행한다. | 실패 로드·씬 반복 전환 뒤 객체/구독/메모리를 센다. | 엔진 공통 캐시 보존과 Scene 소유 상태 정리의 책임부터 정한다. |
| 2 | 일부 Spec은 런타임 handle을 JSON에 저장한다. | 다른 리소스 로드 순서로 재실행해 동일 에셋/파라미터를 복원하는지 확인한다. | 영속 GUID나 직렬화 가능한 값으로 저장하고 로드 시 새 handle을 해석한다. |
| 2 | 미등록 Script는 GUID를 유지하나 pState 없이 Build_Spec하면 exposedFields 보존 경로를 재확인해야 한다. | 등록 코드를 빼고 기존 노출값 Scene을 로드·저장한다. | 미해결 Spec 데이터를 보존할 필요를 판단한다. |
| 2 | Event::Invoke는 listener map을 직접 순회한다. | 자기 Unsubscribe/Clear/재귀 Trigger를 실행한다. | 구독 변경 정책과 적용 시점을 명시한다. |
| 2 | Script 등록 생성 결과에 개발 PC 절대경로가 들어 있다. | 다른 디렉터리 체크아웃에서 재생성·빌드·실행한다. | 프로젝트 기준 경로 또는 GUID 사용 범위를 검토한다. |

## 4-1. 포트폴리오 정리 중 발견한 확인 항목 (2026-10-06)

노션 포트폴리오의 Script Component 페이지를 정리하면서 나온 항목이다. 위 체크리스트와 겹치는 것은 관련 절을 표시했다. 해당 절을 분석할 때 함께 처리한다.

**이미 확인된 사실**
- IScript의 Awake/Start/Priority_Update/Update/Late_Update 기본 구현은 모두 비어 있다. Tick을 등록하지 않아도 빠지는 동작은 없다.
- IScript::Get_Field_Ptr는 non-const와 const 버전이 모두 있다. 포트폴리오 코드 블록에는 non-const만 보여서 const 함수인 Save_Exposed_Fields와 어긋나 보이므로, 노션에는 const 버전을 함께 싣는다.

**확인·수정할 항목**

| 관련 | 관찰 | 확인할 것 | 수정 방향 후보 |
|---|---|---|---|
| A09 | Fill_VTable_Default가 모든 타입에 PRIORITY/UPDATE/LATE를 채운다. Add_To_TickLists의 `if (!vt.Tick[i]) continue;`가 실제로 걸리지 않아 모든 Script가 세 Tick 목록에 들어간다. | Script별 Tick 목록 등록 수를 로그로 센다. | 오버라이드한 함수만 채운다. 함수 포인터 값 비교는 쓰지 않는다(가상 함수 포인터는 MSVC에서 오버라이드와 무관하게 같을 수 있음). `if constexpr (!std::is_same_v<decltype(&TScript::Update), decltype(&IScript::Update)>)` 형태로 타입을 비교한다. |
| A09 | Ensure_Start가 PRIORITY 루프 안에만 있다(포트폴리오 코드 기준). 위 수정을 적용하면 Priority_Update를 정의하지 않은 Script는 Start가 호출되지 않는다. | 실제 Script_Processor::Update와 LateUpdate에서 Ensure_Start 호출 위치를 모두 찾는다. | 위 수정 전에 Start 보장을 Tick 루프 밖(Flush_PendingAwake 이후 등)으로 옮긴다. |
| A08 | IScript가 virtual Tick 함수를 가지므로, Tick까지 SCRIPT_VTABLE로 호출하는 이유를 설명하기 어렵다. Create/Destroy는 팩토리 역할이라 필요하다. | 면접 질문 대비: "virtual로 호출하면 되는데 VTable을 왜 따로 두었나?" | 첫 번째 항목을 적용하면 "오버라이드한 Tick만 VTable에 채워 TickList 등록을 거른다"가 설계 이유가 된다. |
| A09, 4장 | 실제로 Script 관련 메모리 누수를 겪었다. SCRIPT_DATA는 trivially destructible이라 Pool의 Destroy_At이 pState를 해제하지 않는다. 슬롯 재사용 시 Construct_At이 pState를 nullptr로 덮어써 주소도 잃는다. | 삭제(Flush_PendingDestroy → Remove_All_Components), 씬 전환(Destroy_All_SceneObjects), 종료(Processor 소멸자) 세 경로에서 Deallocate 전에 vt.Destroy(pState)가 호출되는지 확인한다. 누수 리포트의 할당 번호로 `_CrtSetBreakAlloc`을 걸어 콜스택에 Script_Create가 나오는지 본다. | Processor가 Deallocate 직전과 소멸 시점에 살아 있는 슬롯의 vt.Destroy를 명시적으로 호출한다. |
| A08, A10 | CPlayer는 `public IScript, public CHuman`으로 다중 상속한다. Script_Create는 `new TScript`를 void*로 반환하고, Create_State_If_Needed는 이를 `static_cast<IScript*>(void*)`로 바로 쓴다. Get_Field_Ptr는 IScript 기준 this에 TScript 기준 offset을 더한다. IScript가 첫 번째 부모라 주소가 우연히 같아서 동작한다. | 상속 순서를 바꾸거나 IScript 앞에 다형 부모가 오면 둘 다 잘못된 주소를 쓰는지 확인한다. | Script_Create에서 `static_cast<IScript*>(new TScript{})`를 저장하고 Destroy는 TScript*로 되돌려 지운다. offset은 IScript 기준으로 보정하거나 "IScript는 첫 번째 부모" 규칙을 static_assert 등으로 강제한다. |
| A02 | Flush_PendingDestroy가 `data.iVersion++` 다음에 `data.Reset()`을 호출한다. | Reset이 iVersion을 0으로 되돌리는지 확인한다. 되돌리면 generation 검사가 무력화된다. | Reset에서 iVersion을 보존한다. |
| A02 | Destroy_Object에서 m_mapUUIDToHandle 해제가 보이지 않는다. | 다른 경로에서 Unregister_UUID_Handle을 호출하는지 확인한다. 아니면 삭제된 UUID로 죽은 handle이 조회된다. | 삭제 확정 시점(Flush)에 UUID를 해제한다. |
| A17, 4장 | 물리 루프(Fixed_Update)가 한 프레임에 두 번 호출된다(직접 확인). 위 2장의 Editor/Game 호출 방식 차이와 연결된다. | 호출 지점을 모두 찾아 프레임당 호출 횟수를 로그로 센다. Editor PLAY, Game, Step 각각 확인한다. | 중복 호출을 없앤다. 또는 누산기 기반 고정 갱신 루프(`acc += dt; while (acc >= step) { Fixed_Update(step); acc -= step; }`, 잔여 시간 보존, 최대 반복 수 제한)로 통일한다. |
| A17, A18 | Solver 반복이 1회라 빠른 이동 중 가끔 벽을 뚫는다. 반복 횟수를 늘리자 다른 버그가 생겨 1회로 되돌렸다. | 반복 횟수 1/4/8에서 관통 빈도와 새 버그 증상을 기록한다. 원인 가설: Solve_Contacts가 매 반복마다 Solve_Penetration으로 Transform을 직접 옮기는데, contact의 깊이·법선·접촉점은 반복 전에 계산한 값 그대로라 위치가 반복 횟수만큼 과보정될 수 있다. | 속도 impulse만 여러 번 반복하고 침투 보정은 반복 뒤 한 번만 적용한다. 그래도 관통이 남으면 빠른 물체에 한해 substep이나 sweep(CCD)을 검토한다. 반복 횟수만으로는 터널링이 해결되지 않는다. |
| A18 | Generate_BroadPhase_Pairs가 `const vector<COLLIDER_PROXY_DATA>&`로 받은 원소를 `const_cast`해 COLLIDER_PAIR에 넣는다. | Narrow Phase·Solver가 이 포인터로 무엇을 수정하는지 추적한다. 면접 질문 대비: "왜 const를 벗기나? 처음부터 non-const로 받으면?" | 수정이 필요하면 입력을 non-const로 받는다. 수정이 없다면 COLLIDER_PAIR를 const 포인터로 바꾼다. |
| A18 | 동적-동적 후보쌍은 같은 함수 안의 `j = i + 1` 이중 루프로 전수 비교한다. 주석 처리된 이전 버전 두 개는 Insert_Dynamic/Query_DynamicOverlap으로 동적 Grid를 썼고, 현재는 되돌린 상태다. CUniform_Grid의 동적 API는 구현돼 있지만 호출되지 않는다. | 동적 Grid를 되돌린 이유를 기억해 기록한다(성능, 버그 등). 동적 Collider 10/50/200개에서 두 방식의 Broad Phase 시간과 후보쌍 수를 비교한다. 동적 Grid는 매 프레임 Clear_Dynamic이 전체 Cell을 비우므로 Cell 수가 많으면 동적 수가 적을 때 더 느릴 수 있다. | 측정 결과로 선택 근거를 남긴다. 되돌린 이유가 측정에 근거하면 면접 소재로 쓴다. 쓰지 않을 동적 API는 제거하거나 미사용으로 명시한다. |
| A18 | 코드만 보면 setAddedPairs 중복 검사는 걸릴 수 없다(동적-정적은 바깥 동적 기준 1회, Query_StaticOverlap이 핸들 중복 제거, 동적-동적은 j > i). 그런데 현재 버전 로그에서 `DuplicateFiltered=40` 전후가 매 프레임 나온다. | allColliders(m_ActivatedColliders)에 같은 hSelf의 proxy가 두 번 이상 들어가는지 확인한다. 들어간다면 어디서 중복 추가되는지 추적한다(물리 루프 2회 호출과의 관계 포함). | 중복 proxy가 원인이면 수집 단계에서 막는다. 그 뒤에도 0이면 unordered_set을 제거한다. |
| A18 | 동적 Grid 버전 로그(BroadPhase \| dynInsert=118 …)에서 동적 Collider 118개일 때 dynCand가 6,000~8,000이다. 전수 비교 쌍 C(118,2)=6,903과 비슷해 동적 Grid가 후보를 거의 줄이지 못했다. 거인 1체당 Bound Collider 18개가 활성화되며 한 Cell(크기 60)에 몰린 것으로 보인다. | dynCand가 양방향·자기 자신을 세는지 로그 코드로 확인한다. Cell 크기를 줄였을 때 dynCand 변화를 본다. | 동적 Grid를 되돌린 근거로 정리한다. 개선 시 Cell 크기 조정, 거인 단위 상위 AABB로 먼저 거르는 계층 구조, 또는 BVH를 검토한다. |
| A18 | Query_StaticOverlap은 디버그 Cell 기록(unordered_set insert, vector push)을 `m_bDebugDrawQueriedCells` 확인 없이 항상 수행한다(Query_DynamicOverlap은 확인함). 또한 호출마다 지역 unordered_set을 새로 만든다. | 동적 Collider 수 × 조회 Cell 수만큼 매 프레임 실행되는 비용을 측정한다. | 디버그 기록을 플래그로 감싼다. 중복 제거 set은 멤버로 재사용하거나 frame stamp 방식으로 바꾼다. |
| A18 | Find_ActivatedCollider_ByHandle이 정적 핸들마다 호출된다. 주석 처리된 이전 버전은 이를 피하려고 handle → proxy map을 따로 만들었다. | 현재 구현이 선형 탐색인지 확인한다. | 선형이면 map 또는 handle index 기반 조회로 바꾼다. |
| A18, A15 | Box 판정은 전부 축 정렬 기준이다. Box-Box·Box-Sphere는 aabbWorld를, Box-Plane은 월드 Right/Up/Look 축과 half extent를 쓴다. 회전한 Box의 충돌 형상은 회전하지 않는다. Box-Box 접촉점은 두 중심의 중간점이다. | 회전한 Box를 경사면·다른 Box에 충돌시켜 결과를 확인한다. Solver의 접촉점 기반 회전 반응과 함께 보면 회전은 하는데 충돌 형상은 그대로인 상태다. | 포트폴리오에는 "Box는 AABB 판정"으로 정확히 쓴다. 필요하면 OBB(SAT)로 확장한다. |
| A19 | Detect_Raycast는 bTrigger만 거르고 bEnable은 확인하지 않는다. Broad Phase 없이 전체 Collider를 순회한다(사전 컬링 코드는 주석 처리). Detect_RayBox의 vN1/vN2는 계산 후 쓰이지 않는다. | 호출부가 넘기는 AllColliders가 이미 활성 Collider만 담는지 확인한다. 비활성 Collider에 입체기동 anchor가 걸리는지 확인한다. | 필요하면 bEnable 검사를 추가한다. 미사용 법선은 RAYCAST_HIT에 담거나 제거한다. |
| A18 | Solve_Penetration은 이동 가중치를 `pBody == nullptr ? 0 : 1`로만 정한다. eBodyType을 보지 않아 STATIC·KINEMATIC Rigidbody도 침투 보정으로 밀린다. 동적 두 물체는 질량과 무관하게 깊이를 반씩 나눈다. DX9 버전(블로그 "Penetration 개념 및 구현")은 `fInvMass` 비율로 나누고 STATIC은 invMass 0으로 고정했으므로, DX11 이식 중 퇴보한 것이다. | Kinematic 발판이나 Rigidbody가 붙은 정적 물체에 동적 물체를 부딪쳐 위치가 밀리는지 확인한다. | DX9 방식으로 되돌린다: 가중치를 `Get_InvMass(pBody)`(DYNAMIC만 fInvMass, 나머지 0)로 바꾸면 body type 규칙과 질량 비율이 함께 해결된다. 수정 후 노션 Solver 표 4번 행과 블로그 링크 설명을 갱신한다. |
| A18 | Add_ImpulseAtPoint와 Calc_InvInertiaOfAxis의 `CTransform transform` 매개변수는 본문에서 쓰이지 않는다. | 다른 호출 경로나 이전 구현에서 쓰였는지 확인한다. | 쓰이지 않으면 매개변수를 제거한다. |
| A18 | Solver 곳곳의 TODO대로 vResolveN_A(A를 B에서 떼는 방향)를 contact normal처럼 쓴다. Box-Box 접촉점이 두 중심의 중간점이라, 그 지점의 r로 계산한 회전 impulse는 실제 접촉과 다를 수 있다. | Box-Box 충돌에서 각속도가 비정상적으로 생기는지 확인한다. | Narrow Phase가 실제 접촉점과 법선을 제공하도록 바꾸고 Solver는 그 값만 쓰게 한다. |
| A17 | Process_SpringJoints가 계산한 힘의 y 성분만 3배 한다(`vForce3.y *= 3.f`). 힘 방향이 로프 방향에서 벗어나고, 포트폴리오의 "Hooke's Law + damping" 설명에 없는 보정이다. | 왜 넣었는지 기록한다(상승감, 중력 상쇄 등). 1배일 때와 이동 궤적을 비교한다. | 게임플레이 보정이면 이름 있는 계수로 빼고 SpringJoint 데이터나 Script 쪽 설정으로 옮긴다. 포트폴리오에는 보정 사실을 밝힌다. |
| A17 | `fForceMag = -kx - cv`에서 앵커 쪽으로 빠르게 움직이면 `-cv`가 커져 fForceMag가 양수가 될 수 있다. 그러면 로프가 플레이어를 앵커 반대로 밀어낸다. 실제 로프는 당기기만 한다. | 앵커 쪽으로 빠르게 날아갈 때 fForceMag 부호를 로그로 확인한다. | `fForceMag = min(fForceMag, 0.f)`로 당기는 힘만 남긴다. |
| A17 | Process_SpringJoints 안에 결과를 쓰지 않는 디버그 블록(vN3, vVel3, fDotVelForce)과 `UNREFERENCED_PARAMETER(fDT)`가 남아 있다. | - | 디버그 블록을 지운다. |
| A08 | 새 Script는 Script_Registry.gen.cpp가 실행 중에 생성되어도 재빌드 후 재시작해야 반영된다. | 포트폴리오에 한계로 적었다. 면접에서 개선 방향을 설명할 수 있어야 한다. | 게임 로직을 별도 DLL로 분리해 핫 리로드한다. 교체 순서는 노출 필드 저장 → vt.Destroy → FreeLibrary/LoadLibrary(복사본 로드) → 재등록 → 재생성 → 필드 복원. 저장·복원은 기존 Reflection을 재사용한다. |

## 4-2. 핸들 구조 리뷰 중 발견한 항목 (2026-10-07)

GameObject/Component 핸들 구조를 LithTech Jupiter(`HOBJECT` + `ILTServer`)와 비교하면서 나온 항목이다. Remove_Components의 슬롯·mask 미정리와 Proxy::Is_Valid의 generation 미검사는 4장 표에 이미 있으므로 여기에 다시 적지 않는다.

**이미 확인된 사실**
- CComponent_System::LateUpdate의 SpriteEffect 줄이 `Update(fDT)`를 호출해 프레임당 두 번 갱신되던 문제는 현재 소스에서 `LateUpdate(fDT)`로 고쳐져 있다. CSpriteEffect_Processor::LateUpdate는 빈 함수다.
- `INVALID_HANDLE_UINT`(Component_Struct.h)와 `Component::INVALID_COMPONENT_SLOT`(Engine_Typedef.h)은 둘 다 0이다. 동작상 문제는 없다.

**확인·수정할 항목**

| 관련 | 관찰 | 확인할 것 | 수정 방향 후보 |
|---|---|---|---|
| A04 | Component Pool의 free list는 LIFO(`m_freeIndices.back()`)이고 version은 11비트(최대 2047)다. 할당·해제를 반복하면 같은 슬롯이 계속 재사용되어 version이 빠르게 한 바퀴 돈다. 그러면 오래된 handle의 version이 다시 일치해 엉뚱한 컴포넌트에 접근할 수 있다(ABA). GameObject Pool은 `std::queue`(FIFO)라 같은 슬롯 재사용 간격이 길어 이 문제가 덜하다. | 컴포넌트 하나를 2048번 Add/Remove한 뒤 맨 처음 handle로 Get_Data_By_Handle이 non-null을 반환하는지 확인한다. 실제 게임에서 발사체·이펙트처럼 빠르게 생성·삭제되는 타입의 슬롯별 재사용 횟수를 센다. | 가장 작은 변경은 free list를 FIFO로 바꿔 재사용 간격을 늘리는 것이다. 근본 해결은 version이 최댓값에 도달한 슬롯을 free list에 돌려보내지 않고 은퇴시키는 것이다. 비트 레이아웃 변경(index 20비트 축소, 64비트 handle)은 Group 슬롯 인코딩까지 영향을 주므로 마지막에 검토한다. |
| A05 | 빈 슬롯 판정에 `INVALID_HANDLE_UINT`(Remove_Component)와 `INVALID_COMPONENT_SLOT`·리터럴 `0`(Get_Component_Handle_By_Type, Remove_All_Components)이 섞여 있다. 값은 같아 지금은 버그가 아니다. | 이름이 다른 두 상수가 같은 0이라는 계약을 코드에서 보장하는지 확인한다. | 상수 하나로 통일하거나 `static_assert(INVALID_HANDLE_UINT == Component::INVALID_COMPONENT_SLOT)`를 둔다. 면접에서 "빈 슬롯을 어떻게 표현하나"에 한 줄로 답할 수 있게 한다. |
| A02, A04 | (면접 대비) LithTech Jupiter는 `HOBJECT`(불투명 포인터) + `LTObjRef`(삭제 알림을 받아 스스로 null이 되는 참조)로 오래된 참조를 막는다. 이 엔진은 index + version 비교로 막는다. | F.E.A.R. Public Tools SDK 헤더로 HOBJECT·LTObjRef의 실제 정의를 확인한다. 기억에 의존한 비교는 포트폴리오에 쓰지 않는다. | "삭제 알림 방식은 참조마다 등록·해제 비용이 들고, version 방식은 비교가 싸지만 위 ABA 한계가 있다"를 장단점으로 정리한다. |

## 5. 요청한 리팩토링 아이디어의 구체화

### R01. Pool의 페이지 크기를 템플릿으로 받기

**목적을 먼저 선택:** 적게 쓰는 큰 DATA의 메모리 낭비 감소인지, 페이지 순회 비용 감소인지, 생성 시 할당 지연 감소인지에 따라 측정과 대안이 달라진다.

현재 대략적인 페이지 메모리는 `sizeof(DATA_T) × N + sizeof(uint32_t) × N + sizeof(uint64_t) × ceil(N/64)`에 정렬 padding 등이 더해진다. vector/map가 가진 외부 heap 메모리는 별도다. N=1024일 때 version·bitset만 약 4,224 bytes이며 DATA 크기가 비용을 크게 좌우한다.

- [ ] `CComponent_Pool<TProxy, PageSize>` 또는 타입별 정책 중 필요한 최소 방식을 고른다. 템플릿 인자와 기본값만으로 충분하면 정책 계층을 만들지 않는다.
- [ ] PageSize, Shift, Mask, BitsetCount를 한 값에서 유도한다. 허용할 최소 크기, 2의 거듭제곱 조건, 마지막 bitset word의 남는 비트를 정의한다.
- [ ] Processor들이 전역 PAGE_SIZE로 직접 순회한다. Pool 헤더만 수정하면 안 되며, 각 Pool의 실제 페이지 크기를 읽거나 Pool의 순회 API로 경계를 모아야 한다.
- [ ] CComponent_Processor_Impl의 Pool 선언뿐 아니라 Physics/UI/Environment의 여러 Pool, Builder가 받는 Pool 타입까지 변경 영향을 추적한다.
- [ ] Handle의 global index/버전 포맷은 유지할 수 있다. 페이지 크기 변경과 handle 비트 레이아웃 변경을 불필요하게 묶지 않는다.
- [ ] 페이지 경계에서 pointer 안정성, 비자명 DATA 생성·소멸, 삭제 후 재사용, 기존 씬 로드가 유지되는지 확인한다.
- [ ] 타입별 live/peak/reserved/할당 횟수/Update 시간으로 64·256·1024 등 후보를 비교한 뒤 값을 선택한다. 예시 값은 추천 확정값이 아니다.

**권장 순서:** 통계 추가 → 현재 사용량 기록 → 최소 템플릿화 → 경계 검증 → 메모리·시간 비교. 규모를 모른 채 모든 Pool 크기를 낮추는 것은 개선이 보장되지 않는다.

### R02. 이전 실행의 최대 사용량으로 초기 용량 잡기

페이지 크기와 초기 확보할 페이지 수는 다른 문제다. 고정 크기 페이지를 유지하면서 초기 페이지 수를 예상 사용량에 맞추는 쪽이 변경 범위가 작다. 이 방식은 주로 실행 중 페이지 할당을 줄이고 시작 시 메모리를 더 확보하는 선택이다.

- [ ] H를 ‘최대 동시 활성 수’, ‘최대 사용 인덱스+1’, ‘최대 페이지 수’ 중 무엇으로 저장할지 정한다. fragmentation과 0번 예약 슬롯 때문에 같은 값이 아니다.
- [ ] `H < 2^n`처럼 엄격히 큰 값을 고를지 `H <= 2^n`의 최소 용량을 고를지 정한다. 정확히 거듭제곱인 값에서 결과가 다르다.
- [ ] global index 기준 필요 슬롯 수를 S라 하면 고정 페이지 N의 최소 페이지 수는 `ceil(S/N)`이다. 총 용량 자체를 반드시 거듭제곱으로 반올림할 필요는 없다.
- [ ] 파일 없을 때 기본 동작, 이전 기록보다 많이 생성될 때 확장, 잘못된 값의 상한, 타입/Scene/빌드 버전별 기록 범위를 정한다.
- [ ] 매 실행 기록 저장이 꼭 필요한지, 타입별 기본값만으로 충분한지 비교한다. JSON 하나가 있어도 버전·이동·쓰기 실패 유지비가 생긴다.
- [ ] 런타임에 페이지 크기 자체를 바꾸면 템플릿 상수와는 별도 설계가 필요하다. 초기 페이지 수 조절과 혼동하지 않는다.

**완료 지표:** 시작 시간, 처음 대량 spawn의 p95/p99 시간, 페이지 할당 수, peak/reserved 메모리. 이전 실행 최대 사용량 캐시를 넣었다는 사실만으로 최적화 성과를 주장하지 않는다.

### R03. 순회 시 빈 슬롯 검사 줄이기

여러 Processor가 모든 페이지의 1024개 슬롯을 방문하고 Is_Allocated를 확인한다. page size를 바꾸기 전에 점유율이 낮은 Pool의 순회가 병목인지 확인한다.

- [ ] 현재 방식과 allocated bitset의 set bit만 순회하는 방식, 활성 index 목록 방식을 비교한다.
- [ ] enabled 목록과 allocated 목록을 구분한다. enable 변경 비용, 스크립트 콜백 중 수정 가능성, 추가 저장 공간을 계산한다.
- [ ] 공통 순회 API를 도입한다면 실제 중복 제거와 페이지 크기 캡슐화 목적만 담당하게 한다.
- [ ] 1%, 10%, 50%, 100% 점유율과 같은 활성 개수·다른 capacity 조건에서 비교한다.

### R04. 렌더 정렬을 실제 상태 재사용으로 연결하기

**이미 있는 것:** GUID 기반 로드 캐시, shader variable 조회 캐시. **추가 검토할 것:** 불투명 큐 정렬, 최종 part 기준 key, 중복 바인딩 생략, 이후 필요한 경우 instancing.

- [ ] 먼저 투명 Mesh의 깊이 정렬을 정확성 작업으로 분리한다. BLEND와 NONBLEND에 같은 정책을 적용하지 않는다.
- [ ] NONBLEND에서 shader/pass/material/mesh와 depth 중 무엇을 우선할지, CPU 상태 변경 감소와 overdraw의 trade-off를 측정한다.
- [ ] 모델 command를 part draw로 미리 전개할지, 현재 command 구조를 유지하며 제한적으로 캐시할지 비교한다. 실제 최종 재질 기준으로 판단한다.
- [ ] shared material 값과 per-object/world/bone 값을 분리하되, 이전 per-object override가 다음 draw에 남지 않도록 기본값 복구 계약을 세운다.
- [ ] 같은 material이어도 shader effect 상태가 다른 경로에서 바뀔 수 있다. 패스 전환·Outline·UI/SpriteBatch·외부 draw 뒤 캐시 무효화 조건을 정의한다.
- [ ] IA buffer/layout, texture, material params 각각의 설정 횟수를 기록한다. Effects Apply를 생략해도 되는지는 별도로 검증한다.
- [ ] draw call 감소가 목표이면 인스턴스 데이터·shader·버퍼 변경이 필요한 instancing을 독립 작업으로 평가한다. 정렬만으로 draw 수가 줄어들지 않는다.

**완료 지표:** command 구성/정렬 시간, draw submission CPU 시간, 상태 변경 수, draw call 수, GPU 시간, 이미지 일치. 화면 정확성 검증 없이 바인딩 횟수만 줄이지 않는다.

### R05. 작지만 설명하기 좋은 유지보수 개선 후보

| 후보 | 기대 이득 | 범위·검증 |
|---|---|---|
| Component 등록 정보의 중복 관리 축소 | enum/Processor/Spec factory 추가 누락 감소 | 실제 누락 사례와 신규 타입 등록 체크부터 확인. 거대한 자동 리플렉션 시스템으로 확대하지 않는다. |
| Script GUID 역조회 일관화 | Build_Spec의 map 전수 검색 제거 | 기존 TypeID→GUID와 항상 동기화되는지 검증 후 활용한다. |
| Scene/Prototype 실패 정리 | 부분 생성 상태·잘못된 mask 방지 | 정상 경로보다 의도적 생성 실패·누락 에셋·중복 UUID를 먼저 재현한다. |
| Callback 중 변경 시점 통일 | Tick/Event의 재진입·iterator 문제 감소 | 생성·disable·destroy·unsubscribe의 즉시성 요구부터 정한다. |
| 새 컴포넌트 추가 절차 문서화 | 제출 이후 확장 설명이 쉬워짐 | 현재 코드의 필수 수정 지점을 검증한다. 코드 생성기 도입은 별도 판단이다. |

## 6. 분석 결과를 남길 형식과 면접 연습

각 시스템마다 아래 한 장을 만들면 ‘파일을 읽었지만 설명하지 못하는’ 상태를 줄일 수 있다.

1. **문제:** 이 구조가 없으면 실제로 어떤 문제가 생기는가.
2. **데이터:** owner, identifier, 저장 상태, 캐시, 실행 상태를 표시한 구조 그림.
3. **흐름:** 정상 입력 하나를 생성부터 제거까지 함수명으로 추적.
4. **불변조건:** 예를 들어 ‘단일 슬롯이면 유효한 handle 하나’, ‘Group이면 Primary 포함 두 개 이상’.
5. **실패 사례:** 빈 값, 잘못된 handle, 콜백 중 삭제, 재사용, 로드 실패 중 해당하는 사례.
6. **대안:** 지금보다 단순한 대안 하나와 더 확장 가능한 대안 하나; 채택하지 않은 비용.
7. **검증:** 테스트 장면·로그·측정 조건과 실제 결과. 미측정 이득은 예상으로 표시.
8. **리팩토링 전후:** 요청 원인, 최소 변경, 유지한 계약, 회귀 검증, 남은 한계.

60분 연습은 전체 항목 암기보다 하나의 설명에서 이어지는 꼬리 질문에 대비한다.

| 시간 | 설명할 주제 | 이어질 질문 예시 |
|---|---|---|
| 0–5분 | Engine 전체와 한 프레임 | Processor 순서가 바뀌면? Script Late는 무엇보다 늦은가? |
| 5–20분 | DATA/Proxy/Pool/Handle/Group | 페이지 증설 시 주소는? 삭제 후 Proxy는? version이 돌면? Group primary를 지우면? |
| 20–30분 | Spec/Scene/Prototype | 포인터를 왜 저장 못 하나? 순환 참조는? 로드 중 실패하면? GUID와 UUID는? |
| 30–40분 | Script 등록/Reflection/수명 | 템플릿은 언제 실행되나? 런타임 TypeID는 안정적인가? Tick 중 삭제하면? |
| 40–50분 | Render 또는 Physics 심화 | 정렬하면 draw 수가 줄어드나? fixed step인데 왜 FPS 영향이 있나? |
| 50–60분 | 실제 발견·수정·측정 사례 | 개선 근거는? 대안은? 안 고친 부분은? 가장 위험한 변경은? |

첫 분석 세션은 **A01→A02→A03→A04→A05**로 잡는 것이 좋다. 여기서 수명·소유권·타입 경계를 이해하면 Spec, Script, 렌더링의 많은 질문이 같은 구조 위에서 연결된다. 이후 각 시스템을 별도 세션으로 분석하고, 위 검증 후보부터 재현한 다음 필요한 리팩토링을 선택한다.

## 7. 이번 문서의 검증 상태

- 핵심 구조의 실제 정의와 주요 호출 본문, 등록 경계, 렌더 큐 생성·정렬·실행, 페이지 순회 사용처를 확인했다.
- 주변 기능은 파일·함수 진입점과 일부 주요 본문을 확인해 분석 범위를 목록화했다. 모든 알고리즘과 셰이더를 줄 단위로 검증한 것은 아니다.
- 빌드·런타임·GPU 캡처·메모리 누수·성능 비교는 미실시다. 체크리스트와 재현 후보를 완료된 검증으로 해석하면 안 된다.
- ‘왜 선택했는가’는 구현으로 설명 가능한 장단점이며 작성 당시의 실제 의도를 단정하지 않는다.
- 기존 소스와 기존 Docs/Engine 문서는 수정하지 않았으며, 이 로드맵만 추가했다.
