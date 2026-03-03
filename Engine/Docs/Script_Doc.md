## Script_System

### 0. 용어 정리
#### Script Asset(`.script`)
- Assets 폴더 안에 존재하는 스크립트 에셋 파일이다.
- { "ClassName": "", "Header" : "" }

#### Script Code(`.h`, `.cpp`)
- `Client/Public`, `Client/Private` 경로에 위치하는 실제 C++ 코드다.
- 컴파일 대상으로, 런타임에 실제 동작하는 클래스다.

#### Script GUID
- `.script` 파일 경로로부터 보장되는 GUID이다. 씬에는 이 GUID만 저장된다.

#### VTable
- 런타임에서 Script GUID->함수 포인터 묶음을 연결해주는 테이블이다.
- CScript_System에 저장되어 Tick마다 호출된다.

#### `Script_Registry.gen.cpp`, `Script_Registry.h`
- Assets / Scripts 폴더 아래의 `.script` 목록을 기반으로 자동 생성되는 코드이다.
- MainApp::Initialize->CScene_Handler::Generate 흐름으로 호출된다.
- 생성된 `.cpp` 생성 이후 바로 `.h`의 Register_AllScripts()가 호출된다.
- 호출 시 vTable이 생성되고 CSystem_Handler::m_VTableMap에 저장된다.

---

### 1. 전체 흐름
#### 에셋(.script) 모으기->코드젠(.gen.cpp) 생성하기->런타임 등록(Register_AllScripts)->GUID to VTable 매핑->실행
- 1. 에디터 / 런타임 시작 시 CAsset_Registry가 Assets 폴더를 스캔한다.
- 2. `.script` 확장자는 CScene_Handler에게 전달되어, 내부에 캐싱된다.
- 3. 클라이언트 초기화 시 SYS_ASSET.Scripts().Generate 로 `gen.cpp`를 생성하여 실행한다.
- 4. 이 과정에서 코드 내 코드 파일의 GUID - VTable 매핑이 모두 등록된다.
- 5. 이 이후, 에디터에서 Script 컴포넌트에 GUID를 바인딩하면, VTable이 nullptr이 아닌 경우(정상) 즉시 실행 루틴에 포함된다.
- 6. 실행 루프에서 Script_Processor가 TickList를 돌며 vt.Tick을 호출한다.

---

### 주의
#### 재빌드가 반드시 필요한 경우
- MainApp::Initialize 이후 추가된 코드는 아직 빌드되지 않았으므로 반드시 재빌드를 거쳐야 한다.
- 빌드 이전에 추가된 파일은 VTable이 등록돼있다면 언제든지 바인딩 / 실행 가능하다.
