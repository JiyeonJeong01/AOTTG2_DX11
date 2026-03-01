#include "Editor_System.h"

#include "Asset_Meta.h"
#include "Asset_Registry.h"
#include "Event_System.h"
#include "SceneChange_Event.h"
#include "Scene.h"
#include "Core_System.h"
#include "Engine_Log.h"
#include "CRender_System.h"
#include "Input_System.h"

IMPLEMENT_SINGLETON(CEditor_System)

CEditor_System::CEditor_System()
{
    
}
CEditor_System::~CEditor_System()
{
    
}


HRESULT CEditor_System::Initialize(const std::filesystem::path& assetRoot)
{
    m_pathAsset = assetRoot;

    SYS_EVENT.Subscribe(EVENT_TYPE::On_Scene_Changed, &CEditor_System::On_SceneChanged, this);

    m_fCamSpeed = 3.f;
    m_bCalculAcc = true;
	m_fMouseSens = 0.0025f;

    return S_OK;
}

void CEditor_System::Update(_float fDT)
{
	Update_Input(fDT);
	/* TODO 현재 씬 상태에 따른 (Edit, Play) 자유 카메라 모드 설정 */
	Submit_SceneViewCamera();
}

ASSET_GUID CEditor_System::Ensure_DefaultScene()
{
    IF_TRUE_RETURN_MSG_BREAK(m_pathAsset.empty(), ASSET_GUID{}, "Ensure_DefaultScene failed: assetRoot empty");

    const std::filesystem::path sceneDir = m_pathAsset / "Scenes";
    const std::filesystem::path scenePath = sceneDir / "Untitled.scene";

    std::error_code ec;
    std::filesystem::create_directories(sceneDir, ec);

    if (!std::filesystem::exists(scenePath))
    {
        json root;
        root["version"] = 1;
        root["objects"] = json::array();

        std::ofstream ofs(scenePath);
        IF_TRUE_RETURN_MSG_BREAK(!ofs.is_open(), ASSET_GUID{}, "Ensure_DefaultScene failed: cannot create Default.scene");
        ofs << root.dump(2);
    }

    /* 레지스트리에 등록 (.meta와 path<->guid 맵 보장) */
    SYS_ASSET.Register_File_Asset(scenePath, ASSET_TYPE::SCENE, ASSET_GUID{});

    ASSET_GUID guid{};
    if (!SYS_ASSET.Try_Get_GUID(scenePath, guid) || !guid.Is_Valid())
    {
        guid = Ensure_Asset_Has_Meta(SYS_ASSET.Normalize_Path(scenePath), SYS_ASSET.AssetType_ToStr(ASSET_TYPE::SCENE));
        (void)SYS_ASSET.Register_File_Asset(scenePath, ASSET_TYPE::SCENE, guid);
    }

    return guid;
}

ASSET_GUID CEditor_System::Create_NewScene_Asset(std::filesystem::path* outPath)
{
    IF_TRUE_RETURN_MSG_BREAK(m_pathAsset.empty(), ASSET_GUID{}, "Create_NewScene_Asset failed: assetRoot empty");

    const std::filesystem::path sceneDir = m_pathAsset / "Scenes";
    std::error_code ec;
    std::filesystem::create_directories(sceneDir, ec);
    std::filesystem::path path;

    for (int i = 1; i < 10000; ++i)
    {
        path = sceneDir / ("Untitled_" + std::to_string(i) + ".scene");
        if (!std::filesystem::exists(path))
            break;
    }

    json root;
    root["version"] = 1;
    root["objects"] = json::array();

    std::ofstream ofs(path);
    IF_TRUE_RETURN_MSG_BREAK(!ofs.is_open(), ASSET_GUID{}, "Create_NewScene_Asset failed: cannot create scene file");
    ofs << root.dump(2);

    ASSET_GUID guid = ASSET_GUID::New_GUID();
    IF_TRUE_RETURN_MSG_BREAK(!SYS_ASSET.Register_File_Asset(path, ASSET_TYPE::SCENE, guid), ASSET_GUID{}, "Create_NewScene_Asset failed: registry register failed");

    if (outPath) *outPath = path;
    return guid;
}

void CEditor_System::On_SceneChanged(EVENT_DATA& event)
{
    SCENECHANGE_EVENT_DATA& onSceneChanged = SCAST(SCENECHANGE_EVENT_DATA&, event);
    m_pCurScene = onSceneChanged.m_pNewScene;
}

void CEditor_System::Play()
{
    IF_NULL_RETURN_MSG_BREAK(m_pCurScene, , "m_pCurScene is nullptr");
    m_pCurScene->Set_State(SCENE_STATE::PLAY);
}

void CEditor_System::Pause()
{
    IF_NULL_RETURN_MSG_BREAK(m_pCurScene, , "m_pCurScene is nullptr");
    m_pCurScene->Set_State(SCENE_STATE::PAUSE);
}

void CEditor_System::Step(_float fDT)
{
    IF_NULL_RETURN_MSG_BREAK(m_pCurScene, , "m_pCurScene is nullptr");
    if (m_pCurScene->Get_State() == SCENE_STATE::PAUSE)
        SYS_CORE.Request_Step(fDT, m_pCurScene);
}

void CEditor_System::Submit_SceneViewCamera()
{
    Build_SceneView_Matrices();

    /* Render context에 제출 */
    SYS_RENDER.Submit_Camera(Engine::Math::Load(m_matCamView), Engine::Math::Load(m_matCamProj));
}

void CEditor_System::Update_SceneView_State(_float fWidth, _float fHeight)
{
    m_fAspect = (fHeight > 0.f) ? (fWidth / fHeight) : (16.f / 9.f);
    const _matrix matProj = Engine::Math::Matrix_PerspectiveFovLH(m_fFovyRad, m_fAspect, m_fNear, m_fFar);
    Engine::Math::Store(m_matCamProj, matProj);
}

void CEditor_System::Build_SceneView_Matrices()
{
    /* 방향 계산 */
	_matrix matRot = Math::Load(Math::RotationRollPitchYaw(m_fPitch, m_fYaw, 0.f));
    const _float3 vForward = Engine::Math::TransformNormal(_float3{ 0.f, 0.f, 1.f }, matRot);
    const _float3 vUp = _float3{ 0.f, 1.f, 0.f };

    const _float3 vTarget = _float3{
        m_vCamPos.x + vForward.x,
        m_vCamPos.y + vForward.y,
        m_vCamPos.z + vForward.z
    };

    /* --- View / Proj 생성 --- */
    const _matrix matView = Engine::Math::Matrix_LookAtLH(m_vCamPos, vTarget, vUp);
    const _matrix matProj = Engine::Math::Matrix_PerspectiveFovLH(m_fFovyRad, m_fAspect, m_fNear, m_fFar);

    Engine::Math::Store(m_matCamView, matView);
    Engine::Math::Store(m_matCamProj, matProj);
}





void CEditor_System::Update_Input(_float fDT)
{
	/* ----------------------------- 마우스 입력 ----------------------------- */
	const long dx = SYS_INPUT.Get_DIMouseMove(MOUSE_MOVE_AXIS::HORIZONTAL);
	const long dy = SYS_INPUT.Get_DIMouseMove(MOUSE_MOVE_AXIS::VERTICAL);

	// 회전 입력을 걸 조건이 따로 없다면, 일단 항상 회전부터 확인하세요.
	m_fYaw += SCAST(_float, dx) * m_fMouseSens;
	m_fPitch += SCAST(_float, dy) * m_fMouseSens;

	// Pitch 제한(뒤집힘 방지)
	const _float limit = 1.55334306f; // 약 89도(라디안)
	if (m_fPitch > limit) m_fPitch = limit;
	if (m_fPitch < -limit) m_fPitch = -limit;

	/* ----------------------------- 이동 입력 ----------------------------- */

	// 현재 카메라 회전으로 로컬 축 만들기 (Build_SceneView_Matrices와 동일 기준)
	const _matrix matRot = Math::Load(Math::RotationRollPitchYaw(m_fPitch, m_fYaw, 0.f));

	const _float3 vForward = Engine::Math::TransformNormal(_float3{ 0.f, 0.f, 1.f }, matRot); // Look(Z+)
	const _float3 vRight = Engine::Math::TransformNormal(_float3{ 1.f, 0.f, 0.f }, matRot); // Right(X+)
	const _float3 vUp = Engine::Math::TransformNormal(_float3{ 0.f, 1.f, 0.f }, matRot); // Up(Y+)

	_float3 vMove = { 0.f, 0.f, 0.f };

	if (SYS_INPUT.Get_Key('W')) { vMove.x += vForward.x; vMove.y += vForward.y; vMove.z += vForward.z; }
	if (SYS_INPUT.Get_Key('S')) { vMove.x -= vForward.x; vMove.y -= vForward.y; vMove.z -= vForward.z; }

	if (SYS_INPUT.Get_Key('D')) { vMove.x += vRight.x;   vMove.y += vRight.y;   vMove.z += vRight.z; }
	if (SYS_INPUT.Get_Key('A')) { vMove.x -= vRight.x;   vMove.y -= vRight.y;   vMove.z -= vRight.z; }

	// E/Q는 “카메라 로컬 Up” 기준으로 상승/하강 (원하시면 월드 Up으로 바꿀 수도 있습니다)
	if (SYS_INPUT.Get_Key('E')) { vMove.x += vUp.x;      vMove.y += vUp.y;      vMove.z += vUp.z; }
	if (SYS_INPUT.Get_Key('Q')) { vMove.x -= vUp.x;      vMove.y -= vUp.y;      vMove.z -= vUp.z; }

	// 입력 방향 정규화(대각선 이동 속도 보정)
	{
		const _float lenSq = vMove.x * vMove.x + vMove.y * vMove.y + vMove.z * vMove.z;
		if (lenSq > 0.f)
		{
			const _float invLen = 1.f / sqrtf(lenSq);
			vMove.x *= invLen;
			vMove.y *= invLen;
			vMove.z *= invLen;
		}
	}

	// Shift 같은 가속 키가 있으시면 여기서 배수 적용 가능
	_float fTargetSpeed = m_fCamSpeed;

	// 가속/감속 포함한 이동 (m_bCalculAcc 사용)
	if (m_bCalculAcc)
	{
		// 목표 속도 벡터
		const _float3 vTargetVel = { vMove.x * fTargetSpeed, vMove.y * fTargetSpeed, vMove.z * fTargetSpeed };

		// 가속(목표 속도로 접근)
		const _float tAcc = (m_fCamAcc > 0.f) ? (m_fCamAcc * fDT) : 1.f;
		m_vCamVel.x += (vTargetVel.x - m_vCamVel.x) * (tAcc > 1.f ? 1.f : tAcc);
		m_vCamVel.y += (vTargetVel.y - m_vCamVel.y) * (tAcc > 1.f ? 1.f : tAcc);
		m_vCamVel.z += (vTargetVel.z - m_vCamVel.z) * (tAcc > 1.f ? 1.f : tAcc);

		// 입력이 없을 때 추가 감속(더 빨리 멈추고 싶으면 m_fCamDamping 키우기)
		const _float lenSq = vMove.x * vMove.x + vMove.y * vMove.y + vMove.z * vMove.z;
		if (lenSq == 0.f && m_fCamDamping > 0.f)
		{
			const _float tDamp = m_fCamDamping * fDT;
			const _float k = (tDamp > 1.f) ? 0.f : (1.f - tDamp);
			m_vCamVel.x *= k;
			m_vCamVel.y *= k;
			m_vCamVel.z *= k;
		}

		// 위치 적분
		m_vCamPos.x += m_vCamVel.x * fDT;
		m_vCamPos.y += m_vCamVel.y * fDT;
		m_vCamPos.z += m_vCamVel.z * fDT;
	}
	else
	{
		// 가속 없이 즉시 이동
		m_vCamPos.x += vMove.x * fTargetSpeed * fDT;
		m_vCamPos.y += vMove.y * fTargetSpeed * fDT;
		m_vCamPos.z += vMove.z * fTargetSpeed * fDT;
	}
}
