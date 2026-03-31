#include "ODM_Gear.h"
#include "GameInstance.h"
#include "Input_System.h"
#include "Raycast.h"
#include "Rope.h"
#include "Player_Struct.h"

NS_BEGIN(Client)

void CODM_Gear::Handle_RopeState(CRope::ROPE_STATE eState, SIDE eSide)
{
    if (eState == CRope::ROPE_STATE::ANCHORED)
    {
        m_sj.Set_Enable(true);
        m_sj.Set_Anchor(m_vAnchor);
        m_sj.Set_UseSpring(true);
        m_sj.Set_Spring(m_fSpring);
        m_sj.Set_Spring(m_fDamper);

        m_OnSuccessAnchored.Invoke(To<_uint>(PLAYER_STATE::AIRBORNE_MOVE), To<_uint>(AIRBORNE_MOVE::AIR_BEGIN));
    }

    /* 사용 상태 갱신 */
    if (eState == CRope::ROPE_STATE::ANCHORED)
        m_flagUsingSide |= To<_uint>(eSide);
    else if (eState == CRope::ROPE_STATE::RETURNING)
        m_flagUsingSide &= ~To<_uint>(eSide);
}

Engine::CGameObject* CODM_Gear::Find_Owner()
{
    m_pOwner = GAME_INSTANCE.Find_GameObject(m_hObject)->Get_Parent();

    if (m_pOwner)
    {
        m_tr = m_pOwner->Get_Component<CTransform>();
        m_sj = m_pOwner->Get_Component<CSpringJoint>();
    }

    return m_pOwner;
}

void CODM_Gear::Try_Grappling(SIDE eSide)
{
    if (!m_pOwner)
    {
        Find_Owner();
        return;
    }

    TYR_GRAPPLING_INFO tInfo{};

    /* 성공한 경우 */
    if (m_upLeftRope && Detect_GrapplingPoint(tInfo))
    {
        m_vAnchor = tInfo.vPoint;
        m_upLeftRope->Start_Extending_Success(XMLoadFloat3(&m_tr->vPosition), XMLoadFloat3(&m_vAnchor));

        m_flagUsingSide |= To<_uint>(eSide);
    }
    /* 실패한 경우 */
    else
    {
        /* 실패하여 앵커 위치가 없는 경우 발사 방향을 찾아 넘기기 */
        _vector vTryPos = XMLoadFloat3(&tInfo.vCamOrigin) + XMLoadFloat3(&tInfo.vRayDir) * m_fRopeMaxDist;
        _vector vTryDir = XMVector4Normalize(vTryPos - XMLoadFloat3(&m_tr->vPosition));

        m_upLeftRope->Start_Extending_Fail(XMLoadFloat3(&m_tr->vPosition), vTryDir);
    }
}

void CODM_Gear::Finish_Grappling()
{
    m_upLeftRope->Stop();
    m_sj.Set_UseSpring(false);
}

_bool CODM_Gear::Detect_GrapplingPoint(TYR_GRAPPLING_INFO& tInfo)
{
    POINT pt = SYS_INPUT.Get_GameCenterPos();
    RAY tRay{ };
    tRay.fMaxDist = m_fRopeMaxDist;
    tRay.fMinDist = 0.f;
    RAYCAST_HITS allHits;

    GAME_INSTANCE.RaycastAll(pt, tRay, allHits);

    tInfo.vCamOrigin = tRay.vOrigin;
    tInfo.vRayDir = tRay.vDir;

    if (allHits.iNumHits > 0)
    {
        tInfo.vPoint = allHits.primaryHit.vHitPos;
        tInfo.fDist = allHits.primaryHit.fDist;
        return true;
    }
    return false;
}

_bool CODM_Gear::Detect_GrapplingDist(_float* fDist)
{
    POINT pt = SYS_INPUT.Get_GameCenterPos();
    RAY tRay{ };
    tRay.fMaxDist = 200.f;
    tRay.fMinDist = 0.f;
    RAYCAST_HITS allHits;

    GAME_INSTANCE.RaycastAll(pt, tRay, allHits);

    if (allHits.iNumHits > 0)
    {
        *fDist = allHits.primaryHit.fDist;
        return true;
    }
    return false;
}

_uint CODM_Gear::Get_UsingFlag()
{
    return m_flagUsingSide;
}

void CODM_Gear::Awake(void* pCtx)
{
}

void CODM_Gear::Start(void* pCtx)
{
    Find_Owner();

    m_upLeftRope = CRope::Create(SIDE::LEFT);
    m_upLeftRope->Subscribe_On_RopeState_Changed(&CODM_Gear::Handle_RopeState, this);
    m_upLeftRope = CRope::Create(SIDE::RIGHT);
    m_upLeftRope->Subscribe_On_RopeState_Changed(&CODM_Gear::Handle_RopeState, this);
}

void CODM_Gear::Priority_Update(void* pCtx, _float fDT)
{
}

void CODM_Gear::Update(void* pCtx, _float fDT)
{
    /* 로프 시작 위치 갱신 */
    if (m_upLeftRope)
    {
        m_upLeftRope->Set_StartPoint(m_tr->vPosition);
        m_upLeftRope->Update(fDT);
    }
}

void CODM_Gear::Late_Update(void* pCtx, _float fDT)
{
}


NS_END;
