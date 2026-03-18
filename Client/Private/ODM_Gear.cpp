#include "ODM_Gear.h"
#include "GameInstance.h"
#include "Input_System.h"
#include "Raycast.h"
#include "Rope.h"

NS_BEGIN(Client)

void CODM_Gear::Handle_RopeState(CRope::ROPE_STATE eState)
{
    if (eState == CRope::ROPE_STATE::ANCHORED)
    {
        m_sj.Set_Enable(true);
        m_sj.Set_Anchor(m_vAnchor);
        m_sj.Set_UseSpring(true);
        m_sj.Set_Spring(m_fSpring);
        m_sj.Set_Spring(m_fDamper);
    }
}

void CODM_Gear::Is_Anchorable()
{
}

Engine::CGameObject* CODM_Gear::Find_Owner()
{
    m_pOwner = GAME_INSTANCE.Find_GameObject(m_hObject)->Get_Parent();

    if (m_pOwner)
    {
        LOG_INFO("owner exist!");
        m_tr = m_pOwner->Get_Component<CTransform>();
        m_sj = m_pOwner->Get_Component<CSpringJoint>();
    }

    return m_pOwner;
}

void CODM_Gear::Try_Grappling()
{
    if (!m_pOwner)
    {
        Find_Owner();
        return;
    }

    POINT pt = SYS_INPUT.Get_GameCenterPos();
    RAY tRay{ };
    tRay.fMaxDist = 1000.f;
    tRay.fMinDist = 0.f;
    RAYCAST_HITS allHits;

    GAME_INSTANCE.RaycastAll(pt, tRay, allHits);

    if (allHits.iNumHits > 0)
    {
        m_vAnchor = allHits.primaryHit.vHitPos;
        m_upRope->Start_Extending(m_tr->vPosition, m_vAnchor);
    }
}

void CODM_Gear::Finish_Grappling()
{
    m_upRope->Stop();
    m_sj.Set_UseSpring(false);
}

void CODM_Gear::Awake(void* pCtx)
{
}

void CODM_Gear::Start(void* pCtx)
{
    Find_Owner();

    m_upRope = CRope::Create();

    m_upRope->Subscribe_On_RopeState_Changed(&CODM_Gear::Handle_RopeState, this);
}

void CODM_Gear::Priority_Update(void* pCtx, _float fDT)
{
}

void CODM_Gear::Update(void* pCtx, _float fDT)
{
    if (m_upRope)
    {
        m_upRope->Set_StartPoint(m_tr->vPosition);
        m_upRope->Update(fDT);
    }
}

void CODM_Gear::Late_Update(void* pCtx, _float fDT)
{
}


NS_END;
