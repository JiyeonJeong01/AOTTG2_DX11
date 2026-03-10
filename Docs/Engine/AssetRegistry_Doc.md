# Asset_Registry_System

프로젝트의 Assets 폴더를 파일 시스템 관점에서 스캔하고, 각 파일/폴더를 **GUID 기반 레코드(ASSET_RECORD)** 로 정규화해 관리하는 시스템이다.  
핵심 목적은 “경로가 바뀌어도 GUID는 유지되는 에셋 식별”을 보장하고, 타입별 핸들러/시스템에게 에셋을 배분하는 것이다.

---

## 0. 용어 정리

### Asset Root
- `/Client/Bin/Assets/`
- `m_assetRoot`로 관리되는 Assets 루트 경로이다.
- `Normalize_Path`를 통해 동일 파일을 가리키는 다양한 경로 표현을 하나로 통일하는 기준점이다.

### Asset GUID (`ASSET_GUID`)
- 절대 경로 대신 에셋을 식별하는 영구 ID이다.

### Asset Record (`ASSET_RECORD`)
- GUID 하나에 대해 “이 에셋이 무엇이며 어디에 있는지”를 담는 레지스트리 엔트리이다.
- 주요 정보는 `tGUID`, `eType`, `eSrc(FILE/BUILTIN)`, `path`, `bDirectory`이다.

### Meta File (`.meta`)
- “경로 ↔ GUID” 결속을 유지하는 보조 파일이다.
- `Ensure_Asset_Has_Meta`가 에셋마다 meta 존재를 강제하고, 필요하면 생성/갱신한다.

---

## 1. 책임과 경계

### CAsset_Registry가 책임지는 것
- Assets 루트 이하의 모든 항목(파일/폴더)을 스캔해 레코드로 등록하는 것
- 각 항목이 meta를 갖도록 보장하고 GUID를 확정하는 것
- `path → guid`, `guid → record` 두 방향 조회를 제공하는 것
- 빌트인(내장) 에셋 GUID를 레지스트리에 미리 심는 것
- 타입별로 “어느 시스템이 먹어야 하는지” 분류해 배분하는 것

### CAsset_Registry가 책임지지 않는 것
- 실제 로딩/디코딩/리소스 생성(예: Mesh GPU 업로드) 자체는 담당하지 않는다.
- 타입별 세부 로직은 핸들러(`CPrototype_Handler`, `CScript_Handler`)나 시스템(`SYS_RESOURCE`)이 담당한다.

---

## 2. 내부 데이터 구조

### GUID 중심 저장소: `m_byGUID`
- `unordered_map<ASSET_GUID, ASSET_RECORD>` 형태이다.
- “이 GUID는 어떤 에셋인가”를 즉시 찾는 것이 목적이다.

### Path 중심 인덱스: `m_byPathUtf8`
- `unordered_map<std::string, ASSET_GUID>` 형태이다.
- “이 경로의 에셋 GUID가 무엇인가”를 즉시 찾는 것이 목적이다.
- 키는 `Normalize_Path` + `Path_To_UTF8`로 정규화된 문자열이다.

이 구조는 “GUID가 1차 키”이고, path는 “보조 인덱스” 성격이다.  
즉, 참조 안정성은 GUID가 담당하고, 검색 편의성은 path 인덱스가 담당하는 구조이다.

---

## 3. 라이프사이클(초기화 흐름)

### Initialize(assetRoot)
초기화의 큰 흐름은 아래와 같다.

1) Asset Root 확정  
- `m_assetRoot = Normalize_Path(assetRoot)`로 기준 경로를 고정한다.

2) 핸들러 생성 및 리셋  
- `m_upPrototype_Handler`, `m_upScript_Handler`를 생성한다.
- `Clear()`로 레지스트리 맵과 핸들러 캐시를 초기화한다.

3) Built-in 등록  
- `Register_Builtin_Asset()`로 엔진 기본 제공 GUID를 레지스트리에 넣는다.  
- 내장 에셋은 보통 파일 경로가 없거나 특별 취급이 필요하므로, “레지스트리에 존재한다”는 사실 자체가 중요하다.

4) File 에셋 스캔 및 재구축  
- `Rebuild()`가 루트부터 재귀적으로 순회하며, 각 항목에 meta를 보장하고 레코드를 등록한다.

5) 타입별 핸들러/시스템으로 배분  
- `Distribute_Assets_To_Handlers()`에서 타입별로 아래처럼 전달한다.
  - PROTOTYPE → `CPrototype_Handler`가 GUID 기반으로 로드한다.
  - SCRIPT → `CScript_Handler`에 GUID를 캐시한다(실제 vtable 바인딩은 별도 흐름이다).
  - MESH → `SYS_RESOURCE.Load_Mesh(guid)`로 리소스 시스템에게 로딩을 요청한다.
  - SCENE → (현 코드에서는 레지스트리에만 존재, 로드는 Scene 시스템이 담당하는 구조이다)

이 초기화 흐름은 “레지스트리는 발견과 식별”, “핸들러/시스템은 소비와 로딩”으로 역할이 분리되어 있다.

---

## 4. Rebuild의 의미와 정책

### Rebuild가 하는 일
- Assets 루트 이하를 재귀 순회하며, “모든 항목이 meta를 가지고, GUID가 확정되며, 레지스트리에 등록되는 상태”를 만든다.
- `.meta` 파일은 스캔 대상에서 제외한다.

### meta 강제 정책
- 각 항목마다 `Ensure_Asset_Has_Meta(path, typeStr)`를 호출하여 meta가 없으면 만든다.
- 결과 GUID가 유효하지 않으면 해당 항목은 등록하지 않는다.

### 중복 방지 정책(중요)
레지스트리의 “정합성”을 위해 아래 두 중복을 막는다.

1) path 중복  
- 동일 정규화 path가 이미 등록되어 있으면 스킵한다.

2) GUID 중복(서로 다른 경로가 같은 GUID를 갖는 상황)  
- 이미 `m_byGUID`에 같은 GUID가 있는데 경로가 다르면 충돌이다.
- 이 경우 새 GUID를 발급하고 meta를 다시 써서 충돌을 해소한다.
- 즉, “GUID는 레지스트리에서 유일해야 한다”가 강한 규칙이다.

이 정책은 결과적으로 “GUID 충돌은 발견 즉시 자동 치유”라는 성격을 가진다.

---

## 5. 타입 감지(Detect_Type)와 확장 포인트

### Detect_Type(path, bDir)
- 디렉토리면 FOLDER이다.
- 파일이면 확장자 기반으로 TEXTURE/MESH/MODEL/MATERIAL/SCENE/PROTOTYPE/SCRIPT/SHADER 등을 판정한다.
- 판정되지 않으면 UNKNOWN이다.

확장 포인트는 두 가지이다.
- 새 확장자를 추가해 타입을 늘리는 것
- 특정 폴더 규칙(예: “Materials/ 아래는 무조건 MATERIAL”) 같은 정책을 추가하는 것

---

## 6. “경로 정규화”가 중요한 이유

### Normalize_Path
- `weakly_canonical` 기반으로 경로를 정규화한다.
- 같은 파일을 가리키는 서로 다른 표현(상대/절대, `..` 포함 등)을 통일한다.

레지스트리의 모든 키(특히 `m_byPathUtf8`)는 이 정규화를 전제로 한다.  
즉, 정규화가 흔들리면 “같은 파일이 서로 다른 에셋으로 보이는” 문제가 생긴다.

---

## 7. 외부 시스템과의 상호작용

### Prototype_Handler와의 관계
- 레지스트리는 PROTOTYPE GUID를 발견하고, 로딩은 `CPrototype_Handler`가 수행한다.
- 즉, 레지스트리는 “무엇이 프로토타입인지”만 판정하고 “내용을 어떻게 읽는지”는 모른다.

### Script_Handler와의 관계
- 레지스트리는 SCRIPT GUID를 발견하면 `m_upScript_Handler->Cache_GUID(guid)`로 목록만 전달한다.
- 실제로 `Register_AllScripts()`를 통해 vtable이 만들어지고, GUID가 확정/보장되는 흐름은 별도이다.
- `Ensure_GUID_For_Path`는 “스크립트 코드젠/등록 과정에서 경로를 레지스트리에 강제로 편입시키는 연결점”이다.

### Resource_System과의 관계
- 레지스트리는 MESH 같은 리소스를 발견하면 `SYS_RESOURCE.Load_Mesh(guid)`로 로딩을 트리거한다.
- 레지스트리는 “로드 시점”을 정책적으로 결정하지만, 로딩 구현은 리소스 시스템이 담당한다.

### Scene 시스템과의 관계
- 레지스트리는 SCENE 타입을 인지하고 GUID/경로를 보관하지만, 실제 로드는 `CScene_Handler`가 담당하는 구조이다.
- 즉, 레지스트리는 씬을 “찾을 수 있게 해주는” 역할에 가깝다.

---

## 8. 런타임 중 동적 등록(Register_File_Asset)의 역할

### Register_File_Asset(rawPath, forcedType, forcedGuid)
런타임 도중 생성/저장된 파일을 레지스트리에 즉시 반영하기 위한 함수이다.  
대표 사용처는 “씬 Save As”, “새 파일 생성”, “외부 툴 결과물 반영” 같은 상황이다.

핵심 기능은 아래와 같다.
- meta 보장 후 GUID 확보
- `forcedGuid`가 주어지면 해당 GUID로 meta를 덮어써 GUID를 강제한다
- `path → guid`, `guid → record`를 동시에 갱신한다
- 기존 GUID가 다른 경로를 가리키던 상황이면, 이전 path 인덱스를 지워 유령 데이터를 방지한다

즉, 이 함수는 “Rebuild 전체 스캔 없이도 레지스트리를 부분적으로 최신화”하는 수단이다.

---

## 9. Ensure_GUID_For_Path의 의미

### Ensure_GUID_For_Path(path)
- 코드젠/등록 흐름에서 “이 경로에 해당하는 에셋 GUID가 반드시 존재해야 한다”는 요구를 충족하기 위한 함수이다.
- 파일이 없으면 빈 파일을 생성할 수도 있다(특히 `.script` 용도이다).
- 이후 타입 판정 및 `Register_File_Asset`으로 레지스트리에 편입시키고 GUID를 반환한다.

즉, 이 함수는 “파일 시스템 상태를 레지스트리 상태로 강제 동기화”하는 브릿지 역할이다.

---

## 10. 설계상 강한 가정(암묵적 규칙)

- GUID는 레지스트리 내에서 유일해야 한다.
- 모든 에셋은 meta를 가져야 한다.
- 레지스트리는 “발견/식별/인덱싱”에 집중하고, 실제 로딩/실행은 핸들러/시스템이 담당한다.
- path는 참고 수단이고, 안정성의 중심은 GUID이다.

---

## 11. 흔한 문제와 관찰 포인트

### GUID 충돌이 났을 때
- 서로 다른 경로가 같은 GUID를 공유하면, 레지스트리가 새 GUID를 발급해 meta를 재작성한다.
- 이 동작은 자동 치유이지만, “어떤 에셋이 원래 GUID였는지” 추적이 필요하면 로그/정책이 추가로 필요하다.

### 레지스트리와 핸들러 상태 불일치
- `Rebuild()`만 호출하고 `Distribute_Assets_To_Handlers()`를 호출하지 않으면, 핸들러 캐시가 오래된 상태가 될 수 있다.
- 반대로, 파일 시스템에서 변경이 생겼는데 Rebuild를 하지 않으면 레지스트리 자체가 오래된 상태가 된다.

### 정규화 실패(weakly_canonical 에러)
- `Normalize_Path`가 정규화 실패 시 원본 path를 돌려준다.
- 이 경우 동일 파일에 대해 서로 다른 키가 생길 여지가 있으므로, 실패 케이스가 자주 발생하면 정책을 강화할 필요가 있다.

---
