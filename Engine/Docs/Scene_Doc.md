# Scene_System

에디터/런타임에서 **`.scene` 파일(JSON)** 을 기준으로 씬을 저장/로드하고, 씬 전환 이벤트를 발행하며, `SCENE_OBJECT_SPEC` 기반으로 런타임 오브젝트를 재구성하는 시스템 문서이다.

---

## 0. 용어 정리

### Scene Asset (`.scene`)
- `Assets/Scenes` 아래에 존재하는 씬 에셋 파일이다.
- JSON 포맷으로 저장되며 기본 형태는 아래 구조를 따른다.
  - `root["version"] = 1`
  - `root["objects"] = [...]`

### Scene GUID
- `.scene` 파일 경로를 기준으로 `SYS_ASSET`(Asset Registry)가 관리하는 GUID이다.
- 씬 전환은 **파일 경로가 아니라 GUID 기준**으로 동작한다.

### Runtime Scene (`CScene`)
- 런타임에서 실제로 Update/Render 되는 씬 객체이다.
- `CScene_Handler::m_pCurrentScene`가 현재 씬을 소유한다.

### `SCENE_OBJECT_SPEC`
- `.scene` 파일에서 “오브젝트 1개” 단위로 저장되는 직렬화 스펙이다.
- 주요 필드는 아래와 같다.
  - `uuid` : 인스턴스 UUID(씬 내 오브젝트 식별자)이다.
  - `protoGuid` : 프로토타입 GUID(프리팹/원형 참조)이다.
  - `isUI` : UI 오브젝트 여부이다.
  - `name`, `layer` : 이름과 레이어 값이다.
  - `parent` : 부모 UUID이다. 없으면 빈 문자열/invalid이다.
  - `overrides` : 컴포넌트 오버라이드(스파스 리스트)이다.

### Override (`COMPONENT_SPEC_BUNDLE`)
- 런타임 오브젝트의 “최종 컴포넌트 상태”를 저장/복원하기 위한 데이터 묶음이다.
- 내부 구조는 `components[COMPONENT_MAX]` 타입 슬롯별 벡터이며, 같은 타입이 여러 개 들어갈 수 있다(Primary/Extras 형태이다).

---

## 1. 데이터 포맷: `.scene` JSON 구조

### Top-level
- `version` : 포맷 버전이다.
- `objects` : `SCENE_OBJECT_SPEC` 배열이다.

### Object(`SCENE_OBJECT_SPEC`) 저장 필드
- `uuid` : string. 필요하지 않다면 비어있을 수 있다.(= 기본 값 Invalid)
- `protoGuid` : string이다. 없다면 비어있을 수 있다.(= 기본 값 Invalid) 
- `isUI` : UI 여부를 나타내며 게임 오브젝트 생성 시 반영된다.
- `name` : string.
- `layer` : uint32로 저장 시 게임오브젝트가 속한 레이어다.
- `parent` : string. 없으면 `""`이다.
- `overrides` : 에디터에서 편집된 컴포넌트의 값을 가진 COMPONENT_SPEC 구조체_array이다.

### overrides 저장 규칙(Sparse flat list)
- 저장 시 타입 슬롯별로 묶지 않고 **flat list**로 저장한다.
- 각 원소는 아래 형태이다.
  - `type` : uint32(`COMPONENT_TYPE`)이다.
  - `data` : json payload(`COMPONENT_SPEC_BASE::ToJson` 결과)이다.

---

## 2. 씬 전환(로드) 전체 흐름

### 개요
**GUID → path resolve → .scene 로드 → specs 생성 → 런타임 오브젝트 생성 → parent 연결 → 이벤트 발행** 흐름이다.

### 2.1 `CScene_Handler::Change_Scene(const ASSET_GUID& tGUID, APP_MODE eMode)`
- `tGUID` 유효성 검사를 수행한다.
- `SYS_ASSET.Find(tGUID)`로 `ASSET_RECORD`를 조회한다.
- `pRec->eType == ASSET_TYPE::SCENE`인지 확인한다.
- `pRec->path`를 확보한 뒤 `Load_NextScene(path, tGUID)`를 호출한다.
- **실패한 경우, 씬을 전환하지 않는다.**

### 2.2 `CScene_Handler::Load_NextScene(const std::filesystem::path& path, const ASSET_GUID& tGUID)`
- `Load_SceneFile(specs, path, createSpec)`로 JSON을 specs로 복원한다.
- `SYS_GAMEOBJECT.Destroy_All_SceneObjects()`로 기존 씬 오브젝트를 정리한다.
- 복원한 specs를 기반으로 `LoadScene_Runtime(specs)`을 호출하여 런타임 오브젝트를 생성한다.

### 2.3 `CScene_Handler::LoadScene_Runtime(const std::vector<SCENE_OBJECT_SPEC>& tSpecs)`

#### 1단계: 오브젝트 생성 + overrides 적용
- `spec.isUI`에 따라 생성 함수를 분기한다.
  - UI 여부에 따라 `Transform`과 `RectTransform`, 기본 설정 레이어 등이 나뉘기 때문이다.
- `Apply_Overrides(pObj, spec.overrides)`로 컴포넌트를 적용한다.

#### 2단계: 부모-자식 연결
- `spec.parent.Is_Valid()`인 경우에만 처리한다.
  - child/parent가 `objectMap`에 존재하는지 확인한다.
  - `itChild->second->Set_Parent(itParent->second)`를 호출한다.

---

## 3. 씬 저장 전체 흐름

### 개요
**현재 씬 → `SYS_GAMEOBJECT`의 specs 생성 → JSON 저장 → Asset Registry 등록** 흐름이다.

### 3.1 `CScene_Handler::Save_CurrentScene(const std::filesystem::path& path)`
- `SYS_GAMEOBJECT.Build_SceneSpecs(specs)`로 씬 오브젝트를 직렬화한다.
- 저장될 경로를 보장한 뒤, `Save_SceneFile(specs, path)`로 JSON을 저장한다.
- `SYS_ASSET.Register_File_Asset(path, ASSET_TYPE::SCENE, scene.Get_GUID())`로 레지스트리에 등록한다.

### 3.2 `CScene_Handler::Save_SceneFile(...)`
- `Serialize_SceneObjectSpec(o)`로 각 오브젝트를 JSON으로 변환해 `root["objects"]`에 push한다.
- `ofs << root.dump(2)`로 저장한다.

---

## 4. 직렬화/역직렬화 규칙

### 4.1 `Serialize_SceneObjectSpec(const SCENE_OBJECT_SPEC& tSpec)`
- 메타 필드를 저장한다.
  - `uuid`, `protoGuid`, `isUI`, `name`, `layer`, `parent`를 저장한다.
- overrides를 저장한다.
  - 타입 슬롯 i를 순회한다.
  - 비어있으면 스킵한다.
  - 각 spec마다 아래를 수행한다.
    - `jc["type"] = (uint32_t)up->Get_Type()`를 기록한다.
    - `up->ToJson(payload)` 후 `jc["data"] = payload`로 기록한다.
    - `jOverrides.push_back(jc)`를 수행한다.
- 최종적으로 `j["overrides"] = jOverrides`로 저장한다.

### 4.2 `Deserialize_SceneObjectSpec(const json& j, SCENE_OBJECT_SPEC& out, SpecFactoryFn createSpec)`
- `uuid` 파싱 실패 시 false를 반환한다.
- `protoGuid` 파싱 실패 시 `out.protoGuid = ASSET_GUID{};`로 두고 진행한다.
- `parent`는 빈 문자열이면 invalid 처리한다(`out.parent = {};`).
- overrides는 아래 규칙으로 복원한다.
  - `out.overrides.Clear_All()`를 호출한다.
  - `j["overrides"]`가 array면 순회한다.
    - `type`을 읽고 `eType`을 결정한다.
    - `createSpec(eType)`로 스펙 객체를 생성한다.
    - `spec->FromJson(jc["data"])` 성공 시, `out.overrides.components[iSlot].emplace_back(std::move(spec))`로 push한다.

---

## 5. Overrides 적용 규칙

### `CScene_Handler::Apply_Overrides(CGameObject* pObject, const COMPONENT_SPEC_BUNDLE& tBundle)`
- 타입 슬롯(0 ~ `COMPONENT_MAX`)을 순회한다.
- 해당 슬롯의 `vSpecs`가 비어있으면 스킵한다.
- 각 `upSpec`에 대해 아래를 수행한다.
  - `SYS_COMPONENT.Create_Component_From_Spec(pObject, upSpec.get())`를 호출한다.

즉, 로드 시점에서 **오브젝트 생성 후 overrides로 컴포넌트를 생성/갱신**하는 방식이다.

---

## 6. 스펙 팩토리 확장 포인트

새로운 컴포넌트 추가 시, 아래를 반드시 확장해야 한다. `Component_Spec.h_`에서 확인 가능하다.
### `CScene_Handler::Create_Spec_By_Type(COMPONENT_TYPE eType)`
- `COMPONENT_TYPE::TRANSFORM` → `TRANSFORM_SPEC`이다.
- `COMPONENT_TYPE::RECT_TRANSFORM` → `RECTTRANSFORM_SPEC`이다.
- `COMPONENT_TYPE::CANVAS_RENDERER` → `CANVAS_RENDERER_SPEC`이다.
- `COMPONENT_TYPE::MESH_RENDERER` → `MESH_RENDERER_SPEC`이다.
- `COMPONENT_TYPE::SCRIPT` → `SCRIPT_SPEC`이다.

주의 사항은 아래와 같다.
- 새 컴포넌트를 씬에 저장/로드하려면 최소로 아래 3가지를 만족해야 한다.
  - `Create_Spec_By_Type`에 case를 추가해야 한다.
  - 해당 SPEC의 `ToJson / FromJson` 구현이 필요하다.
  - `SYS_COMPONENT.Create_Component_From_Spec`에서 해당 SPEC 처리가 지원되어야 한다.

---

## 7. 에디터: 새 씬 생성/기본 씬 보장

### 7.1 기본 씬 보장: `CEditor_System::Ensure_DefaultScene()`
- `Assets/Scenes/Untitled.scene`이 없으면 빈 씬 JSON을 생성한다.
- `SYS_ASSET.Register_File_Asset(scenePath, ASSET_TYPE::SCENE, ASSET_GUID{})`로 레지스트리에 등록한다.
  실패 시 재시도를 수행한다. 
- 반환값은 최종 `ASSET_GUID`이다.

### 7.2 새 씬 생성: `CEditor_System::Create_NewScene_Asset(std::filesystem::path* outPath)`
- `Untitled_1.scene`부터 빈 번호를 탐색해 **파일을 생성한 뒤, 빈 JSON을 저장**한다.
- 새로운 GUID로 생성된 씬을 `SYS_ASSET.Register_File_Asset(path, ASSET_TYPE::SCENE, guid)`로 등록한다.

### 7.3 메뉴: New SCENE
- `Editor::CMainPanel::DrawMenu()` 에서 새 씬을 생성할 수 있게 한다
  - `CEditor_System::Create_NewScene_Asset(&newPath)`를 호출하여 위 로직을 실행한다.
  - `SYS_CORE.Change_Scene(newGuid, APP_MODE::EDITOR_EDIT)`로 전환한다. 

---

## 8. 씬 변경 이벤트와 에디터 동기화

### 씬 변경 이벤트 발행
- `CScene_Handler::Change_Scene`에서 아래를 수행한다.
  - `SYS_EVENT.Trigger(SCENECHANGE_EVENT_DATA(...))`로 이벤트를 발행한다.

### 에디터 구독 및 반영
- `CEditor_System::Initialize`에서 아래를 수행한다.
  - `SYS_EVENT.Subscribe(EVENT_TYPE::On_Scene_Changed, &CEditor_System::On_SceneChanged, this);`로 구독한다.
- `CEditor_System::On_SceneChanged`에서 아래를 수행한다.
  - `m_pCurScene = onSceneChanged.m_pNewScene;`로 현재 씬 포인터를 갱신한다.

---

## 9. Play / Pause / Step (에디터)

- `Play()`는 `m_pCurScene->Set_State(SCENE_STATE::PLAY);`를 호출한다.
- `Pause()`는 `m_pCurScene->Set_State(SCENE_STATE::PAUSE);`를 호출한다.
- `Step(_float fDT)`는 아래 규칙으로 동작한다.
  - `m_pCurScene->Get_State() == SCENE_STATE::PAUSE`인 경우에만 수행한다.
  - `SYS_CORE.Request_Step(fDT, m_pCurScene);`를 호출한다.

---

## 10. 주의사항 / 실패 케이스 체크리스트

### 로드 실패 대표 케이스
- `SYS_ASSET.Find(tGUID)`가 null -> CAsset_Registry에 등록되지 않았다.
- `pRec->eType != ASSET_TYPE::SCENE` -> `.scene` 파일을 로드하지 않았다.
- `pRec->path` 가 "" -> 파일이 존재하지 않는다.
- parent가 있는데 `objectMap`에 parent/child 중 하나가 없는 경우.

### 저장은 되는데 로드가 안 되는 대표 케이스
- `Create_Spec_By_Type`에 case가 없어 `createSpec(eType)`가 `nullptr`을 반환할 수 있따.
- overrides에서 해당 컴포넌트가 스킵되어 복원되지 않을 수 있다.

### parent 처리 규칙
- parent는 `.scene`에서 빈 문자열일 수 있으며, 이 경우 기본 값인 invalid로 처리된다.
- 로드 시 `out.parent = {};`로 명시 처리된다.
