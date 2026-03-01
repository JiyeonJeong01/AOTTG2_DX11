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
#include "Component_System.h"
#include "Transform_Processor.h"
#include "Resource_System.h"
#include "Mesh.h"

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

	// Transform processor 접근 (지연님 코드 스타일대로)
	CComponent_Processor* pBase = nullptr;
	SYS_COMPONENT.Bind_ComponentProcessor(COMPONENT_TYPE::TRANSFORM, &pBase);
	IF_NULL_RETURN_MSG_BREAK(pBase, E_FAIL, "Transform processor bind failed");
	m_pTransform_Processor = SCAST(CTransform_Processor*, pBase);

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


// CEditor_System.cpp

static inline bool Ray_AABB(const _float3& o, const _float3& d, const _float3& bmin, const _float3& bmax, float* outT)
{
	float tmin = 0.f;
	float tmax = FLT_MAX;

	auto slab = [&](float o1, float d1, float mn, float mx) -> bool
		{
			if (fabsf(d1) < 1e-8f)
				return (o1 >= mn && o1 <= mx);

			float invD = 1.f / d1;
			float t1 = (mn - o1) * invD;
			float t2 = (mx - o1) * invD;
			if (t1 > t2) { float tmp = t1; t1 = t2; t2 = tmp; }

			if (t1 > tmin) tmin = t1;
			if (t2 < tmax) tmax = t2;
			return tmin <= tmax;
		};

	if (!slab(o.x, d.x, bmin.x, bmax.x)) return false;
	if (!slab(o.y, d.y, bmin.y, bmax.y)) return false;
	if (!slab(o.z, d.z, bmin.z, bmax.z)) return false;

	if (outT) *outT = tmin;
	return true;
}

static inline void Transform_AABB_World(const _float3& localMin, const _float3& localMax, const _matrix& matWorld, _float3& outMin, _float3& outMax)
{
	// 8 코너를 월드로 변환해서 다시 AABB로 감쌉니다. (간단/안전)
	_float3 corners[8] =
	{
		{ localMin.x, localMin.y, localMin.z },
		{ localMax.x, localMin.y, localMin.z },
		{ localMin.x, localMax.y, localMin.z },
		{ localMax.x, localMax.y, localMin.z },
		{ localMin.x, localMin.y, localMax.z },
		{ localMax.x, localMin.y, localMax.z },
		{ localMin.x, localMax.y, localMax.z },
		{ localMax.x, localMax.y, localMax.z },
	};

	outMin = { FLT_MAX,  FLT_MAX,  FLT_MAX };
	outMax = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

	for (int i = 0; i < 8; ++i)
	{
		const _float3 w = Engine::Math::TransformCoord(corners[i], matWorld);
		outMin.x = (w.x < outMin.x) ? w.x : outMin.x;
		outMin.y = (w.y < outMin.y) ? w.y : outMin.y;
		outMin.z = (w.z < outMin.z) ? w.z : outMin.z;

		outMax.x = (w.x > outMax.x) ? w.x : outMax.x;
		outMax.y = (w.y > outMax.y) ? w.y : outMax.y;
		outMax.z = (w.z > outMax.z) ? w.z : outMax.z;
	}
}

void CEditor_System::Pick_SceneView(_uint px, _uint py, _uint vpW, _uint vpH)
{
	// 0) 레이 만들기: SceneView 카메라(m_matCamView/m_matCamProj) 기준
	const float ndcX = (2.0f * (((float)px + 0.5f) / (float)vpW)) - 1.0f;
	const float ndcY = 1.0f - (2.0f * (((float)py + 0.5f) / (float)vpH));

	// 지연님 Math 유틸에 "inv(view*proj) + TransformCoord" 조합이 없으면,
	// 기존에 쓰시는 XM 기반 inverse/transform 함수로 치환하시면 됩니다.
	const _matrix V = Engine::Math::Load(m_matCamView);
	const _matrix P = Engine::Math::Load(m_matCamProj);

	// 여기 inverse 함수 이름은 지연님 엔진에 맞춰 바꿔주세요.
	const _matrix invVP = Engine::Math::Matrix_Inverse(V * P);

	const _float4 nearNdc = { ndcX, ndcY, 0.f, 1.f };
	const _float4 farNdc = { ndcX, ndcY, 1.f, 1.f };

	const _float3 pNear = Engine::Math::TransformCoord(_float3{ ndcX, ndcY, 0.f }, invVP);
	const _float3 pFar = Engine::Math::TransformCoord(_float3{ ndcX, ndcY, 1.f }, invVP);

	const _float3 rayOrigin = pNear;
	_float3 rayDir = { pFar.x - pNear.x, pFar.y - pNear.y, pFar.z - pNear.z };
	rayDir = Engine::Math::Normalize(rayDir);

	LOG_INFO("Ray o=(%.2f, %.2f, %.2f), d=(%.2f, %.2f, %.2f)",
		rayOrigin.x, rayOrigin.y, rayOrigin.z,
		rayDir.x, rayDir.y, rayDir.z);

	// 1) 후보는 "그려진 것들"만: m_AllDrawCmds
	const auto& cmds = SYS_RENDER.Get_AllDrawCmds();

	float bestT = FLT_MAX;
	COMPONENT_HANDLE bestTransform = INVALID_HANDLE;
	uint32_t bestMesh = 0;

	LOG_INFO("DrawCmd count = %u", (_uint)cmds.size());
	_uint hitCount = 0;

	for (const auto& cmd : cmds)
	{
		if (cmd.kind != DRAW_TYPE::MESH)
			continue;

		// UI/기타 레이어 제외하고 싶으면 여기서 거르세요.
		// if (cmd.eLayer == RENDER_LAYER::UI) continue;

		const COMPONENT_HANDLE hTr = cmd.mesh.hTransform;
		if (hTr == INVALID_HANDLE)
			continue;

		// (A) 월드 행렬 얻기: 기존에 쓰시던 proxy 방식 그대로
		const auto tr = m_pTransform_Processor->Get_Proxy(COMPONENT_TYPE::TRANSFORM, hTr);
		if (!tr.Is_Valid())
			continue;

		const _matrix matWorld = Engine::Math::Load(tr->matWorld);

		// (B) 로컬 AABB 얻기 (메시에 min/max가 있어야 합니다)
		_float3 localMin{}, localMax{};
		const MESH_ENTRY* pMesh = SYS_RESOURCE.Get_Mesh(cmd.mesh.hMesh);
		if (!pMesh)
			continue;

		pMesh->Get_Mesh_LocalAABB(localMin, localMax);

		// (C) 월드 AABB로 변환
		_float3 worldMin{}, worldMax{};
		Transform_AABB_World(localMin, localMax, matWorld, worldMin, worldMax);

		// (D) 레이-AABB
		float tHit = 0.f;
		if (!Ray_AABB(rayOrigin, rayDir, worldMin, worldMax, &tHit))
			continue;


		if (tHit < bestT)
		{
			++hitCount;

			bestT = tHit;
			bestTransform = hTr;
			bestMesh = cmd.mesh.hMesh;
		}
	}
	LOG_INFO("HitCount = %u", hitCount);
	if (bestTransform != INVALID_HANDLE)
	{
		const auto tr = m_pTransform_Processor->Get_Proxy(COMPONENT_TYPE::TRANSFORM, bestTransform);
		IF_TRUE_RETURN_MSG_BREAK(!tr.Is_Valid(), , "Picked transform proxy invalid.");

		OBJECT_HANDLE hObj = tr->hObject;
		Set_Selection_Object(hObj);

		LOG_INFO("Picked: hTransform=%u, hMesh=%u, t=%.2f",
			bestTransform.Get_Index(),
			bestMesh,
			bestT);
	}
	else
	{
		Set_Selection_Object(OBJECT_HANDLE{});
	}
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

	m_fYaw += SCAST(_float, dx) * m_fMouseSens;
	m_fPitch += SCAST(_float, dy) * m_fMouseSens;

	/* Pitch 제한 */
	const _float limit = 1.55334306f; // 약 89도(라디안)
	if (m_fPitch > limit) m_fPitch = limit;
	if (m_fPitch < -limit) m_fPitch = -limit;

	/* ----------------------------- 이동 입력 ----------------------------- */

	/* 현재 카메라 회전으로 로컬 축 만들기 : 반드시 Build_SceneView_Matrices와 동일해야 한다. */ 
	const _matrix matRot = Math::Load(Math::RotationRollPitchYaw(m_fPitch, m_fYaw, 0.f));

	const _float3 vForward = Engine::Math::TransformNormal(_float3{ 0.f, 0.f, 1.f }, matRot); /* Look(Z+) */ 
	const _float3 vRight = Engine::Math::TransformNormal(_float3{ 1.f, 0.f, 0.f }, matRot); /* Right(X+) */ 
	const _float3 vUp = Engine::Math::TransformNormal(_float3{ 0.f, 1.f, 0.f }, matRot); /* Up(Y+) */ 

	_float3 vMove = { 0.f, 0.f, 0.f };

	if (SYS_INPUT.Get_Key('W')) { vMove.x += vForward.x; vMove.y += vForward.y; vMove.z += vForward.z; }
	if (SYS_INPUT.Get_Key('S')) { vMove.x -= vForward.x; vMove.y -= vForward.y; vMove.z -= vForward.z; }

	if (SYS_INPUT.Get_Key('D')) { vMove.x += vRight.x;   vMove.y += vRight.y;   vMove.z += vRight.z; }
	if (SYS_INPUT.Get_Key('A')) { vMove.x -= vRight.x;   vMove.y -= vRight.y;   vMove.z -= vRight.z; }

	/* E/Q : 로컬 기준으로 상승/하강 */ 
	if (SYS_INPUT.Get_Key('E')) { vMove.x += vUp.x;      vMove.y += vUp.y;      vMove.z += vUp.z; }
	if (SYS_INPUT.Get_Key('Q')) { vMove.x -= vUp.x;      vMove.y -= vUp.y;      vMove.z -= vUp.z; }

	/* 입력 방향 정규화 */ 
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
	_float fTargetSpeed = m_fCamSpeed;

	/* 가속/감속 포함한 이동 */ 
	if (m_bCalculAcc)
	{
		const _float3 vTargetVel = { vMove.x * fTargetSpeed, vMove.y * fTargetSpeed, vMove.z * fTargetSpeed };

		const _float tAcc = (m_fCamAcc > 0.f) ? (m_fCamAcc * fDT) : 1.f;
		m_vCamVel.x += (vTargetVel.x - m_vCamVel.x) * (tAcc > 1.f ? 1.f : tAcc);
		m_vCamVel.y += (vTargetVel.y - m_vCamVel.y) * (tAcc > 1.f ? 1.f : tAcc);
		m_vCamVel.z += (vTargetVel.z - m_vCamVel.z) * (tAcc > 1.f ? 1.f : tAcc);

		const _float lenSq = vMove.x * vMove.x + vMove.y * vMove.y + vMove.z * vMove.z;
		if (lenSq == 0.f && m_fCamDamping > 0.f)
		{
			const _float tDamp = m_fCamDamping * fDT;
			const _float k = (tDamp > 1.f) ? 0.f : (1.f - tDamp);
			m_vCamVel.x *= k;
			m_vCamVel.y *= k;
			m_vCamVel.z *= k;
		}

		m_vCamPos.x += m_vCamVel.x * fDT;
		m_vCamPos.y += m_vCamVel.y * fDT;
		m_vCamPos.z += m_vCamVel.z * fDT;
	}
	else
	{
		m_vCamPos.x += vMove.x * fTargetSpeed * fDT;
		m_vCamPos.y += vMove.y * fTargetSpeed * fDT;
		m_vCamPos.z += vMove.z * fTargetSpeed * fDT;
	}
}
