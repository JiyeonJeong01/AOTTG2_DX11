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
#include "GameObject_System.h"
#include "Engine_Math.h"
#include "Render_Context.h"
#include "MeshRenderer.h"

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

    m_fCamSpeed = 7.f;
    m_bCalculAcc = true;
	m_fMouseSens = 0.0025f;

	m_pTransform_Processor = SYS_COMPONENT.Bind_Processor<CTransform_Processor>();
    IF_NULL_RETURN_MSG_BREAK(m_pTransform_Processor, E_FAIL, "Transform processor bind failed");

    return S_OK;
}

void CEditor_System::Update(_float fDT)
{
	Update_Input(fDT);

    if (m_bDebugCam/* || (m_pCurScene != nullptr && (m_pCurScene->Get_State() != SCENE_STATE::PLAY))*/)
    	Submit_DebugCamera();
}

/* NOTE : 에디터 시작 시점에 기본 씬(Untitled)으로 시작한다. */
ASSET_GUID CEditor_System::Ensure_DefaultScene()
{
    IF_TRUE_RETURN_MSG_BREAK(m_pathAsset.empty(), ASSET_GUID{}, "Ensure_DefaultScene failed: assetRoot empty");

    const std::filesystem::path sceneDir = m_pathAsset / "Scenes";
    const std::filesystem::path scenePath = sceneDir / "Untitled.scene"; /* 실제 파일 경로에 존재하는 기본 씬  */

    std::error_code ec;
    std::filesystem::create_directories(sceneDir, ec);

    if (!std::filesystem::exists(scenePath)) /* 만약 기본 씬이 없는 경우, 즉시 파일 시스템에 생성하여 보장한다. */
    {
        json root;
        root["version"] = 1;
        root["objects"] = json::array();

        std::ofstream ofs(scenePath);
        IF_TRUE_RETURN_MSG_BREAK(!ofs.is_open(), ASSET_GUID{}, "Ensure_DefaultScene failed: cannot create Default.scene");
        ofs << root.dump(2);
    }

    /* 기본 씬인 Untitled.scene이 CAsset_Registry에 등록돼 유효함을 보장한다 (.meta와 path<->guid 맵 보장) */
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

    /* 새 씬 파일을 일단 디스크에 생성하고, GUID를 발급받아 CAsset_Registry에 우선 등록한다. */
    /* 이름 변경은 직후 에디터에서 수행된다. */
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

void CEditor_System::Submit_DebugCamera()
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

void CEditor_System::Toggle_DebugCamera(_bool bToggle)
{
    m_bDebugCam = bToggle;

    /* Debug Camera로 전환! */
    if (bToggle)
    {
        m_matCamView = SYS_RENDER.Contexts()->Get_View();
        m_matCamProj = SYS_RENDER.Contexts()->Get_Proj();
        m_vCamPos = SYS_RENDER.Contexts()->Get_CamPosition();

        const auto& matViewInv = SYS_RENDER.Contexts()->Get_ViewInv();

        _float3 vLook = {};
        memcpy(&vLook, &matViewInv.m[To<size_t>(STATE::LOOK)][0], sizeof(_float3));

        const _vector vLookNorm = XMVector3Normalize(XMLoadFloat3(&vLook));
        XMStoreFloat3(&vLook, vLookNorm);

        m_fYaw = atan2f(vLook.x, vLook.z);
        const _float fLenXZ = sqrtf(vLook.x * vLook.x + vLook.z * vLook.z);
        m_fPitch = -atan2f(vLook.y, fLenXZ);

        m_vCamVel = { 0.f, 0.f, 0.f };
    }

}

void CEditor_System::Focus_Object(CGameObject* pObj)
{
    if (!pObj || pObj->Get_Handle().Is_UI())
        return;

    CTransform tr = pObj->Get_Component<CTransform>();
    CMeshRenderer mr = pObj->Get_Component<CMeshRenderer>();

    _float3 vTargetPos = tr->vPosition;

    const _matrix matRot = Math::Load(Math::RotationRollPitchYaw(m_fPitch, m_fYaw, 0.f));
    const _float3 vForward = Engine::Math::TransformNormal(_float3{ 0.f, 0.f, 1.f }, matRot);

    const _float fFocusDistance = 5.f;

    const _float fOffset = std::fmaxf(std::fmaxf(tr->vScale.x, tr->vScale.y), tr->vScale.z);

    if (mr.Is_Valid())
    {
        const MESH_ENTRY* pMesh = SYS_RESOURCE.Get_Mesh(mr->hMesh);
        if (pMesh)
        {
            pMesh->minAABB;
            pMesh->maxAABB;
        }
    }

    m_vCamPos.x = vTargetPos.x - vForward.x * fFocusDistance * fOffset;
    m_vCamPos.y = vTargetPos.y - vForward.y * fFocusDistance * fOffset;
    m_vCamPos.z = vTargetPos.z - vForward.z * fFocusDistance * fOffset;

    m_vCamVel = { 0.f, 0.f, 0.f };
}

/* Editor 마우스 피킹은 Ray <-> AABB */
static inline bool Ray_AABB(const _float3& vOrigin, const _float3& vDir, const _float3& vMin, const _float3& vMax, _float* fOutT)
{
	_float tMin = 0.f;
	_float tMax = FLT_MAX;

    /* ----------------------------------------- SLAB 알고리즘 ----------------------------------------- */
    /* 상자를 X, Y, Z 축과 평행한 공간으로 보고, 광선이 각 축의 평면 쌍을 구하는 구간 tMin, tMax르 구한다. */
    /* 세 축 모두 공통적으로 겹치는 구간이 있으면, 광선은 상자를 통과한다. */
	auto slab = [&](float o1, float d1, float mn, float mx) -> _bool
		{
			if (fabsf(d1) < 1e-8f)
				return (o1 >= mn && o1 <= mx);

			_float invD = 1.f / d1;
			_float t1 = (mn - o1) * invD;
			_float t2 = (mx - o1) * invD;
			if (t1 > t2) { float tmp = t1; t1 = t2; t2 = tmp; }

			if (t1 > tMin) tMin = t1;
			if (t2 < tMax) tMax = t2;
			return tMin <= tMax;
		};

	if (!slab(vOrigin.x, vDir.x, vMin.x, vMax.x)) return false;
	if (!slab(vOrigin.y, vDir.y, vMin.y, vMax.y)) return false;
	if (!slab(vOrigin.z, vDir.z, vMin.z, vMax.z)) return false;

	if (fOutT) *fOutT = tMin;
	return true;
}

static inline void Transform_AABB_World(const _float3& vLocalMin, const _float3& vLocalMax, const _matrix& matWorld, _float3& outMin, _float3& outMax)
{
	/* 로컬의 AABB -> 월드의 AABB */
	_float3 corners[8] =
	{
		{ vLocalMin.x, vLocalMin.y, vLocalMin.z },
		{ vLocalMax.x, vLocalMin.y, vLocalMin.z },
		{ vLocalMin.x, vLocalMax.y, vLocalMin.z },
		{ vLocalMax.x, vLocalMax.y, vLocalMin.z },
		{ vLocalMin.x, vLocalMin.y, vLocalMax.z },
		{ vLocalMax.x, vLocalMin.y, vLocalMax.z },
		{ vLocalMin.x, vLocalMax.y, vLocalMax.z },
		{ vLocalMax.x, vLocalMax.y, vLocalMax.z },
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
	/* Ray 생성 */
	const float ndcX = (2.0f * (((float)px + 0.5f) / (float)vpW)) - 1.0f;
	const float ndcY = 1.0f - (2.0f * (((float)py + 0.5f) / (float)vpH));

	const _matrix V = Engine::Math::Load(m_matCamView);
	const _matrix P = Engine::Math::Load(m_matCamProj);

	/* Inverse */
	const _matrix invVP = Engine::Math::Matrix_Inverse(V * P);

	const _float3 pNear = Engine::Math::TransformCoord(_float3{ ndcX, ndcY, 0.f }, invVP);
	const _float3 pFar = Engine::Math::TransformCoord(_float3{ ndcX, ndcY, 1.f }, invVP);

	const _float3 rayOrigin = pNear;
	_float3 rayDir = { pFar.x - pNear.x, pFar.y - pNear.y, pFar.z - pNear.z };
	rayDir = Engine::Math::Normalize(rayDir);

	/* 충돌 대상은 그려진 것들로 한정한다 : Render_System의 m_AllDrawCmds  */
	const auto& cmds = SYS_RENDER.Get_AllDrawCmds();

	_float bestT = FLT_MAX;
	COMPONENT_HANDLE bestTransform = INVALID_HANDLE;
	uint32_t bestMesh = 0;

    for (const auto& cmd : cmds)
    {
        if (cmd.kind != DRAW_TYPE::MESH)
            continue;

        const COMPONENT_HANDLE hTr = cmd.mesh.hTransform;
        if (hTr == INVALID_HANDLE)
            continue;

        const auto tr = m_pTransform_Processor->Get_Proxy(COMPONENT_TYPE::TRANSFORM, hTr);
        if (!tr.Is_Valid())
            continue;

        const _matrix matWorld = Engine::Math::Load(tr->matWorld);
        const _matrix invWorld = Engine::Math::Matrix_Inverse(matWorld);

        const _float3 localRayOrigin = Engine::Math::TransformCoord(rayOrigin, invWorld);

        _float3 localRayDir = Engine::Math::TransformNormal(rayDir, invWorld);
        localRayDir = Engine::Math::Normalize(localRayDir);

        if (SYS_RESOURCE.Is_ModelHandle(cmd.mesh.hMesh))
        {
            const MODEL_ENTRY* pModel = SYS_RESOURCE.Get_Model(cmd.mesh.hMesh);
            if (!pModel)
                continue;

            const size_t iNumMeshes = pModel->parts.size();
            for (size_t i = 0; i < iNumMeshes; ++i)
            {
                const MESH_ENTRY* pMesh = SYS_RESOURCE.Get_Mesh(pModel->parts[i].hMesh);
                if (!pMesh)
                    continue;

                _float3 localMin{}, localMax{};
                pMesh->Get_Mesh_LocalAABB(localMin, localMax);

                float tHit = 0.f;
                if (!Ray_AABB(localRayOrigin, localRayDir, localMin, localMax, &tHit))
                    continue;

                const _float3 localHit =
                {
                    localRayOrigin.x + localRayDir.x * tHit,
                    localRayOrigin.y + localRayDir.y * tHit,
                    localRayOrigin.z + localRayDir.z * tHit
                };

                const _float3 worldHit = Engine::Math::TransformCoord(localHit, matWorld);

                const _float3 vToHit =
                {
                    worldHit.x - rayOrigin.x,
                    worldHit.y - rayOrigin.y,
                    worldHit.z - rayOrigin.z
                };

                const _float worldT =
                    sqrtf(vToHit.x * vToHit.x +
                        vToHit.y * vToHit.y +
                        vToHit.z * vToHit.z);

                if (worldT < bestT)
                {
                    bestT = worldT;
                    bestTransform = hTr;
                    bestMesh = cmd.mesh.hMesh;
                }
            }
        }
        else
        {
            const MESH_ENTRY* pMesh = SYS_RESOURCE.Get_Mesh(cmd.mesh.hMesh);
            if (!pMesh)
                continue;

            _float3 localMin{}, localMax{};
            pMesh->Get_Mesh_LocalAABB(localMin, localMax);

            float tHit = 0.f;
            if (!Ray_AABB(localRayOrigin, localRayDir, localMin, localMax, &tHit))
                continue;

            const _float3 localHit =
            {
                localRayOrigin.x + localRayDir.x * tHit,
                localRayOrigin.y + localRayDir.y * tHit,
                localRayOrigin.z + localRayDir.z * tHit
            };

            const _float3 worldHit = Engine::Math::TransformCoord(localHit, matWorld);

            const _float3 vToHit =
            {
                worldHit.x - rayOrigin.x,
                worldHit.y - rayOrigin.y,
                worldHit.z - rayOrigin.z
            };

            const _float worldT =
                sqrtf(vToHit.x * vToHit.x +
                    vToHit.y * vToHit.y +
                    vToHit.z * vToHit.z);

            if (worldT < bestT)
            {
                bestT = worldT;
                bestTransform = hTr;
                bestMesh = cmd.mesh.hMesh;
            }
        }
    }

    if (bestTransform != INVALID_HANDLE)
	{
		const auto tr = m_pTransform_Processor->Get_Proxy(COMPONENT_TYPE::TRANSFORM, bestTransform);
		IF_TRUE_RETURN_MSG_BREAK(!tr.Is_Valid(), , "Picked transform proxy invalid.");

		OBJECT_HANDLE hObj = tr->hObject;
        if (m_hSelectedObject != hObj) /* 새 오브젝트 피킹 */
        {
            CGameObject* pObj = SYS_GAMEOBJECT.Get_Wrapper(hObj);
            IF_NULL_RETURN_MSG_BREAK(pObj, , "ERROR : Picked obj is nullptr.");
            GAMEOBJECT_EVENT_DATA go{ EVENT_TYPE::GameObject, pObj };
            m_OnPicking.Invoke(go);
        }
	}
	else if (m_hSelectedObject.Is_Valid())
	{
        m_hSelectedObject = OBJECT_HANDLE{};

        GAMEOBJECT_EVENT_DATA go{ EVENT_TYPE::GameObject, nullptr };
        m_OnPicking.Invoke(go);
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
    if (!m_bDebugCam)
        return;

    /* ----------------------------- 마우스 입력 ----------------------------- */
    const long dx = SYS_INPUT.Get_DIMouseMove(MOUSE_MOVE_AXIS::HORIZONTAL);
    const long dy = SYS_INPUT.Get_DIMouseMove(MOUSE_MOVE_AXIS::VERTICAL);
    long dz = SYS_INPUT.Get_DIMouseMove(MOUSE_MOVE_AXIS::DEPTH);

    if (SYS_INPUT.Get_Key(VK_RBUTTON))
    {
        m_fYaw += SCAST(_float, dx) * m_fMouseSens;
        m_fPitch += SCAST(_float, dy) * m_fMouseSens;

        /* Pitch 제한 */
        const _float limit = 1.55334306f;
        if (m_fPitch > limit) m_fPitch = limit;
        if (m_fPitch < -limit) m_fPitch = -limit;
    }
    else
    {
        dz = 0;
    }

    /* ----------------------------- 이동 입력 ----------------------------- */

    /* 현재 카메라 회전으로 로컬 축 만들기
     * NOTE ! 반드시 Build_SceneView_Matrices와 동기화해줘야 한다. */
    const _matrix matRot = Math::Load(Math::RotationRollPitchYaw(m_fPitch, m_fYaw, 0.f));

    const _float3 vForward = Engine::Math::TransformNormal(_float3{ 0.f, 0.f, 1.f }, matRot);   /* Look(Z+) */
    const _float3 vRight = Engine::Math::TransformNormal(_float3{ 1.f, 0.f, 0.f }, matRot);     /* Right(X+) */
    const _float3 vUp = Engine::Math::TransformNormal(_float3{ 0.f, 1.f, 0.f }, matRot);        /* Up(Y+) */

    /* 마우스 휠 줌인/아웃 : 카메라를 전방/후방으로 이동 */
    if (dz != 0)
    {
        const _float fWheelStep = SCAST(_float, dz) * 0.01f * m_fWheelZoomSpeed;

        m_vCamPos.x += vForward.x * fWheelStep;
        m_vCamPos.y += vForward.y * fWheelStep;
        m_vCamPos.z += vForward.z * fWheelStep;
    }

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
