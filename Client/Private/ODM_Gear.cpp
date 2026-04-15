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
        LOG_INFO("-------------------------------------");
        LOG_INFO("ROPE Anchored : %.2f, %.2f, %.2f", m_vAnchor.x, m_vAnchor.y, m_vAnchor.z);

        _vector vDiff = XMLoadFloat3(&m_vAnchor) - m_tr.Get_StateXM(STATE::POSITION);
        _float fDist = XMVectorGetX(XMVector3Length(vDiff));
        _float3 vDir;
        XMStoreFloat3(&vDir, XMVector3Normalize(vDiff));

        LOG_INFO("Anchor <- Player Dir : %.2f, %.2f, %.2f", vDir.x, vDir.y, vDir.z);
        LOG_INFO("-------------------------------------");

        /* attach 순간 1회만 outward 성분 절반 제거 */
        {
            _float3 vLinearVel = m_rb->vLinearVel; // 여기만 본인 rigidbody 접근 변수로 바꾸세요
            _vector vLinearVelXM = XMLoadFloat3(&vLinearVel);

            const _float fToward = XMVectorGetX(XMVector3Dot(vLinearVelXM, XMVector3Normalize(vDiff)));

            if (fToward < 0.f)
            {
                /* 앵커 반대 방향 성분(outward) 절반만 제거 */
                vLinearVelXM = vLinearVelXM - XMVector3Normalize(vDiff) * (fToward * 0.5f);
                XMStoreFloat3(&m_rb->vLinearVel, vLinearVelXM); // 여기만 본인 rigidbody 접근 변수로 바꾸세요
            }
        }

        m_sj.Set_Enable(true);
        m_sj.Set_Anchor(m_vAnchor);
        m_sj.Set_UseSpring(true);

        if (m_bReelBoost)
        {
            m_sj.Set_Spring(m_fForceSpring);
            m_sj.Set_Damper(m_fForceDamper);
        }
        else
        {
            m_sj.Set_Spring(m_fForceSpring);
            m_sj.Set_Damper(m_fForceDamper);
        }

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
        m_rb = m_pOwner->Get_Component<CRigidbody>();
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

    TRY_GRAPPLING_INFO tInfo{};

    _vector vLook =  XMVector3Normalize(m_tr.Get_StateXM(STATE::LOOK));
    _vector vUp = XMVector3Normalize(m_tr.Get_StateXM(STATE::UP));
    _vector vRopeStart = XMLoadFloat3(&m_tr->vPosition) + vLook * m_vRopeOffset.z + vUp * m_vRopeOffset.y;

    if (Detect_GrapplingPoint(tInfo))
    {
        m_vAnchor = tInfo.vPoint;

        if (m_upLeftRope && eSide == SIDE::LEFT)
            m_upLeftRope->Start_Extending_Success(vRopeStart, XMLoadFloat3(&m_vAnchor));
        else if (m_upRightRope && eSide == SIDE::RIGHT)
            m_upRightRope->Start_Extending_Success(vRopeStart, XMLoadFloat3(&m_vAnchor));

        m_flagUsingSide |= To<_uint>(eSide);
    }
    /* 실패한 경우 */
    else
    {
        /* 실패하여 앵커 위치가 없는 경우 발사 방향을 찾아 넘기기 */
        _vector vTryPos = XMLoadFloat3(&tInfo.vCamOrigin) + XMLoadFloat3(&tInfo.vRayDir) * m_fRopeMaxDist;
        _vector vTryDir = XMVector4Normalize(vTryPos - vRopeStart);

        if (eSide == SIDE::LEFT)
            m_upLeftRope->Start_Extending_Fail(vRopeStart, vTryDir);
        else if (eSide == SIDE::RIGHT)
            m_upRightRope->Start_Extending_Fail(vRopeStart, vTryDir);
    }
}

void CODM_Gear::Finish_Grappling(SIDE eSide)
{
    const _uint iSideFlag = To<_uint>(eSide);

    if ((m_flagUsingSide & iSideFlag) == 0)
        return;

    if (eSide == SIDE::LEFT)
    {
        if (m_upLeftRope)
            m_upLeftRope->Stop();
    }
    else if (eSide == SIDE::RIGHT)
    {
        if (m_upRightRope)
            m_upRightRope->Stop();
    }

    m_flagUsingSide &= ~iSideFlag;

    if (m_flagUsingSide == 0)
        m_sj.Set_UseSpring(false);
}

_bool CODM_Gear::Detect_GrapplingPoint(TRY_GRAPPLING_INFO& tInfo)
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
        sort(allHits.allHits.begin(), allHits.allHits.end(), [](const RAYCAST_HIT& a, const RAYCAST_HIT& b)
            {
                return a.fDist < b.fDist;
            });


        for (const auto& hit : allHits.allHits)
        {
            _float3 vCamPos3 = GAME_INSTANCE.Cam_Position();
            _vector vCamPos = XMLoadFloat3(&vCamPos3);
            _float3 vTargetPos3 = hit.vHitPos;
            _vector vTargetPos = XMLoadFloat3(&vTargetPos3);

            _float fCamToPlayerDistSq = XMVectorGetX(XMVector3LengthSq(XMLoadFloat3(&m_tr->vPosition) - vCamPos));
            _float fCamToTargetDistSq = XMVectorGetX(XMVector3LengthSq(vTargetPos - vCamPos));

            if (fCamToPlayerDistSq < fCamToTargetDistSq)
            {
                tInfo.vPoint = hit.vHitPos;
                tInfo.fDist = hit.fDist;
                return true;
            }
        }
    }
    return false;
}

_bool CODM_Gear::Detect_GrapplingDist(_float* fDist)
{
    POINT pt = SYS_INPUT.Get_GameCenterPos();
    RAY tRay{ };
    tRay.fMaxDist = m_fRopeMaxDist;
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

_bool CODM_Gear::Has_Anchor() const
{
    return m_flagUsingSide != 0;
}

_float3 CODM_Gear::Get_AnchoredPos() const
{
    if (m_flagUsingSide != 0)
        return m_vAnchor;
    return {};
}

void CODM_Gear::Set_ReelBoost(_bool bEnable)
{
    m_bReelBoost = bEnable;

    if (m_flagUsingSide == 0)
        return;

    m_sj.Set_UseSpring(true);

    if (m_bReelBoost)
    {
        m_sj.Set_Spring(m_fSpringReel);
        m_sj.Set_Damper(m_fDamperReel);
    }
    else
    {
        m_sj.Set_Spring(m_fSpringNormal);
        m_sj.Set_Damper(m_fDamperNormal);
    }
}

_bool CODM_Gear::Can_UseGas() const
{
    return m_tGas.fCurrent > 0.f;
}

void CODM_Gear::Fill_Max()
{
    m_tGas.fCurrent = m_tGas.fMax;
}

GAS_STATE CODM_Gear::Get_GasState() const
{
    return m_tGas;
}

void CODM_Gear::Bind_PlayerContext(const PLAYER_CONTEXT& tContext)
{
    m_fSpringNormal = tContext.pStats->fSpringNormal;
    m_fDamperNormal = tContext.pStats->fDamperNormal;
    m_fSpringNormal = tContext.pStats->fSpringNormal;
    m_fDamperNormal = tContext.pStats->fDamperNormal;
}

CODM_Gear::CODM_Gear()
{
}

CODM_Gear::~CODM_Gear()
{
}

void CODM_Gear::Awake(void* pCtx)
{
}

void CODM_Gear::Start(void* pCtx)
{
    Find_Owner();

    m_upLeftRope = CRope::Create(SIDE::LEFT);
    m_upLeftRope->Subscribe_On_RopeState_Changed(&CODM_Gear::Handle_RopeState, this);
    m_upRightRope = CRope::Create(SIDE::RIGHT);
    m_upRightRope->Subscribe_On_RopeState_Changed(&CODM_Gear::Handle_RopeState, this);

    m_upLeftRope->Set_MaxLength(m_fRopeMaxDist);
    m_upRightRope->Set_MaxLength(m_fRopeMaxDist);
}

void CODM_Gear::Priority_Update(void* pCtx, _float fDT)
{
}

void CODM_Gear::Update(void* pCtx, _float fDT)
{
    const _uint flagUsingSide = m_flagUsingSide;

    if (!Can_UseGas() && flagUsingSide != 0)
    {
        if (flagUsingSide & To<_uint>(SIDE::LEFT))
            Finish_Grappling(SIDE::LEFT);

        if (flagUsingSide & To<_uint>(SIDE::RIGHT))
            Finish_Grappling(SIDE::RIGHT);
    }

    /* 로프 시작 위치 갱신 */
    _vector vLook = XMVector3Normalize(m_tr.Get_StateXM(STATE::LOOK));
    _vector vUp = XMVector3Normalize(m_tr.Get_StateXM(STATE::UP));
    _float3 vRopeStart;
     XMStoreFloat3(&vRopeStart, XMLoadFloat3(&m_tr->vPosition) + vLook * m_vRopeOffset.z + vUp * m_vRopeOffset.y);
     
    if (m_upLeftRope)
    {
        m_upLeftRope->Set_StartPoint(vRopeStart);
        m_upLeftRope->Update(fDT);
    }
    if (m_upRightRope)
    {
        m_upRightRope->Set_StartPoint(vRopeStart);
        m_upRightRope->Update(fDT);
    }

    if (m_flagUsingSide != 0)
    {
        m_tGas.fCurrent -= fDT;
        m_tGas.fCurrent = fmaxf(m_tGas.fCurrent, 0.f);
    }

}

void CODM_Gear::Late_Update(void* pCtx, _float fDT)
{
}


NS_END;
