#include "CrawlerTitan.h"
#include "TitanState.h"
#include "CrawlerTitanStateMachine.h"
#include "AnimationClip_Titan.h"
#include "TargetSensor.h"
#include "TitanBound_Controller.h"
#include "HitBox.h"
#include "HurtBox.h"
#include "NavMesh.h"
#include "Titan_Scriptable_Object.h"

NS_BEGIN(Client)

CCrawlerTitan::CCrawlerTitan()
{
}

CCrawlerTitan::~CCrawlerTitan()
{
}

void CCrawlerTitan::Awake(void* pCtx)
{
    m_goTitan = SYS_GAMEOBJECT.Get_Wrapper(m_hObject);
    m_upStateMachine = CCrawlerTitanStateMachine::Create(m_goTitan, this);
    m_upNav = GAME_INSTANCE.Create_NavMesh(L"../../Client/Bin/Assets/DataFiles/NavMesh.dat", true);

    IF_NULL_RETURN_MSG_BREAK(m_upStateMachine, , "m_upStateMachine is nullptr");
}

void CCrawlerTitan::Start(void* pCtx)
{
    /* 컴포넌트 참조 */
    {
        m_tComponents.transform = m_goTitan->Get_Component<CTransform>();
        IF_TRUE_RETURN_MSG_BREAK(!m_tComponents.transform.Is_Valid(), , "transform is invalid");

        m_tComponents.animator = m_goTitan->Get_Component<CAnimator>();
        IF_TRUE_RETURN_MSG_BREAK(!m_tComponents.animator.Is_Valid(), , "animator is invalid");

        m_tComponents.collider = m_goTitan->Get_Component<CCollider>();
        IF_TRUE_RETURN_MSG_BREAK(!m_tComponents.collider.Is_Valid(), , "collider is invalid");

        m_tComponents.rigidbody = m_goTitan->Get_Component<CRigidbody>();
        IF_TRUE_RETURN_MSG_BREAK(!m_tComponents.rigidbody.Is_Valid(), , "rigidbody is invalid");

        m_tComponents.meshRenderer = m_goTitan->Get_Component<CMeshRenderer>();
        IF_TRUE_RETURN_MSG_BREAK(!m_tComponents.meshRenderer.Is_Valid(), , "meshRenderer is invalid");
    }

    /* 런타임 정보 참조 */
    {
        m_tRef.pFSM = m_upStateMachine.get();
        IF_NULL_RETURN_MSG_BREAK(m_tRef.pFSM, , "m_tRef.pFSM is nullptr");

        m_tRef.pSensor = m_goTitan->Get_Script_InChildren<CTargetSensor>();
        IF_NULL_RETURN_MSG_BREAK(m_tRef.pSensor, , "m_tRef.pSensor is nullptr");

        /* 히트박스 캐싱 */
        auto allHitBoxes = m_goTitan->Get_AllScripts_InChildren<CHitBox>();
        for (auto& hit : allHitBoxes)
        {
            CGameObject* goHitBox = hit->Get_HitBoxObject();
            IF_NULL_RETURN_MSG_BREAK(goHitBox, , "goHitBox is nullptr");

            ///* ERASE_마스크_설정 */
            //{
            //    if (!goHitBox)
            //        __debugbreak();

            //    auto col = goHitBox->Get_Component<CCollider>();
            //    col->iMask = (O_HITBOX | O_ENEMY);
            //    col->iDiscardMask = (O_HITBOX | O_ENEMY | O_WALKABLE);
            //}

            auto [iter, bInserted] = m_AllHitBoxes.emplace(std::string(goHitBox->Get_Label()), hit);
            IF_TRUE_RETURN_MSG_BREAK(!bInserted, , "duplicated hitbox label");
        }

        /* 허트박스에 이벤트 등록 */
        auto allHurtBoxes = m_goTitan->Get_AllScripts_InChildren<CHurtBox>();
        for (auto& hurt : allHurtBoxes)
        {
            if (!hurt)
                continue;

            hurt->Subscribe_OnHurt(&CCrawlerTitan::On_Hurt, this);

            ///* ERASE_마스크_설정 */
            //{
            //    auto* goHurt = hurt->Get_HurtBoxObject();
            //    if (!goHurt)
            //        __debugbreak();

            //    auto col = goHurt->Get_Component<CCollider>();
            //    col->iMask = (O_HURTBOX | O_ENEMY);
            //    col->iDiscardMask = (O_HURTBOX | O_ENEMY | O_WALKABLE);
            //}
        }

        m_tRef.m_pStunnedAcc = &m_iStunnedAcc;
        m_tRef.pAllHitBoxes = &m_AllHitBoxes;
        m_tRef.pPose = &m_ePose;
        m_tRef.pNav = m_upNav.get();
    }

    m_tComponents.animator.Set_NextAnimationClip(ANIM_TITAN::CRAWLER_IDLE);

    auto* scTitanSO = m_goTitan->Get_Script<CTitan_Scriptable_Object>();
    IF_NULL_RETURN_MSG_BREAK(scTitanSO, , "scTitanSO is nullptr");

    TITAN_SCRIPTABLE_OBJECT tSO{};
    tSO = scTitanSO->Get_Data();
    m_tStats.fCurSpeed = tSO.fCurSpeed;
    m_tStats.fMaxSpeed = tSO.fMaxSpeed;

    TITAN_CONTEXT tContext;
    tContext.tComponents = m_tComponents;
    tContext.tRef = m_tRef;
    tContext.pStats = &m_tStats;
    tContext.pPatrol = &m_tPatrol;
    tContext.pSO = &tSO;

    m_goEren = tSO.goEren;
    IF_NULL_RETURN_MSG_BREAK(m_goEren, , "m_goEren is nullptr");

    m_upStateMachine->Cache_TitanInfos(tContext);
    m_spCurState = m_upStateMachine->Sync_StateMachine();

    m_tRef.pSensor->Set_TargetMask(O_EREN | O_SCOUT | O_PLAYER);
    m_tRef.pSensor->Subscribe_OnDetectedTarget(&CCrawlerTitan::On_DetectedHumanSide, this);
    m_upStateMachine->Subscribe_OnChangedCurState(&CCrawlerTitan::OnChange_CurState, this);

    /* 히트박스 전부 끄기 */
    for (auto& hit : m_AllHitBoxes)
        hit.second->Set_Active(false);
}

void CCrawlerTitan::Priority_Update(void* pCtx, _float fDT)
{
    Validate_Target();
    m_upStateMachine->Priority_Update(fDT);
}

void CCrawlerTitan::Update(void* pCtx, _float fDT)
{
    m_upStateMachine->Update(fDT);
}

void CCrawlerTitan::Late_Update(void* pCtx, _float fDT)
{
    m_upStateMachine->Late_Update(fDT);
}

TITAN_CONTEXT CCrawlerTitan::Get_TitanContext()
{
    TITAN_CONTEXT tContext;
    tContext.tComponents = m_tComponents;
    tContext.tRef = m_tRef;
    tContext.pStats = &m_tStats;
    tContext.pPatrol = &m_tPatrol;

    return tContext;
}

void CCrawlerTitan::Set_Target(Engine::CGameObject* pTarget)
{
    if (m_goTarget == pTarget)
        return;

    m_goTarget = pTarget;

    if (m_tRef.pSensor)
    {
        if (pTarget)
            m_tRef.pSensor->Set_Target(pTarget);
        else
            m_tRef.pSensor->Clear_Target();
    }

    m_OnChanged_Target.Invoke(m_goTarget);
}

void CCrawlerTitan::Clear_Target()
{
    Set_Target(nullptr);
}

_bool CCrawlerTitan::Has_Target() const
{
    return m_goTarget != nullptr;
}

_bool CCrawlerTitan::Is_ValidTarget(Engine::CGameObject* pTarget)
{
    if (!pTarget)
        return false;

    CTransform trTarget = pTarget->Get_Component<CTransform>();
    if (!trTarget.Is_Valid())
        return false;

    return true;
}

CGameObject* CCrawlerTitan::Get_CurTarget() const
{
    return m_goTarget;
}

_bool CCrawlerTitan::Is_Moving()
{
    if (!m_spCurState)
        return false;
    const auto eState = m_spCurState->Get_State();
    return eState == TITAN_STATE::MOVE || eState == TITAN_STATE::CHASE;
}

void CCrawlerTitan::On_Grab(SIDE eSide, CHuman* pHuman)
{
    UNREFERENCED_PARAMETER(eSide);
    UNREFERENCED_PARAMETER(pHuman);

    /* crawler는 일단 grab 패턴 미사용 */
}

void CCrawlerTitan::On_Dead(const _float fAccuracy)
{
    UNREFERENCED_PARAMETER(fAccuracy);

    TITAN_STATE eState = m_spCurState->Get_State();
    if (eState == TITAN_STATE::DEAD)
        return;

    m_upStateMachine->Change_State(To<_uint>(TITAN_STATE::DEAD), 0);
}

void CCrawlerTitan::On_Stunned(const HIT_INFO& tHitInfo)
{
    TITAN_STATE eState = m_spCurState->Get_State();
    if (eState == TITAN_STATE::STUNNED || eState == TITAN_STATE::DEAD)
        return;

    if (m_goEren)
        Set_Target(m_goEren);

    m_upStateMachine->Change_State(To<_uint>(TITAN_STATE::STUNNED), To<_uint>(eState));
}

void CCrawlerTitan::On_Hurt(const HIT_INFO& tHitInfo, const std::string& strHurtBox)
{
    const _int iPlayerAtkMask = O_PLAYER | O_HITBOX;
    const _int iCropsAtkMask = O_SCOUT | O_HITBOX;
    const _int iAttackerMask = tHitInfo.goAttacker->Get_Mask();

    if (!((iPlayerAtkMask == iAttackerMask) || (iCropsAtkMask == iAttackerMask)))
        return;

    TITAN_HURT eHurt = TITAN_HURT::END;

    eHurt = TITAN_HURT::CRAWL_EYE;

    //else if (strHurtBox == TITAN_WEAK_POINT)
    //{
    //    if (tHitInfo.goAttacker->Is_ExactMask(O_PLAYER | O_HITBOX))
    //    {
    //        CTransform tr = m_goWeakPoint->Get_Component<CTransform>();
    //        const _vector vPoint = XMLoadFloat3(&tr->vPosition);
    //        _vector vDiff = vPoint - XMLoadFloat3(&tHitInfo.vHitPoint);
    //        _float fDiff = XMVectorGetX(XMVector3Length(vDiff));
    //
    //        On_Dead(fDiff);
    //    }
    //
    //    return;
    //}

    if (eHurt == TITAN_HURT::END)
        return;

    m_upStateMachine->Change_State(To<_uint>(TITAN_STATE::HURT), To<_uint>(eHurt));
}

void CCrawlerTitan::Validate_Target()
{
    if (!m_goTarget)
        return;

    if (!Is_ValidTarget(m_goTarget))
    {
        Clear_Target();
    }
}

void CCrawlerTitan::On_DetectedHumanSide(CGameObject* goHuman)
{
    if (!Is_ValidTarget(goHuman))
        return;

    const uint32_t iNewMask = goHuman->Get_Mask();
    const uint32_t iPrevMask = m_goTarget ? m_goTarget->Get_Mask() : 0;

    if (!(iNewMask == O_PLAYER || iNewMask == O_EREN || iNewMask == O_SCOUT))
        return;

    if (Has_Target())
    {
        if (iNewMask >= iPrevMask)
            return;
    }

    Set_Target(goHuman);

    TITAN_STATE eCur = m_spCurState ? m_spCurState->Get_State() : TITAN_STATE::IDLE;

    if (iNewMask == O_EREN)
    {
        _bool CantAtkEren = eCur == TITAN_STATE::ATTACK_EREN || eCur == TITAN_STATE::DEAD;
        if (!CantAtkEren)
        {
            m_tRef.pFSM->Change_State(To<_uint>(TITAN_STATE::ATTACK_EREN));
            return;
        }
    }

    _bool bToChase = eCur == TITAN_STATE::IDLE || eCur == TITAN_STATE::MOVE || eCur == TITAN_STATE::ATTACK_EREN;
    if (bToChase)
    {
        m_tRef.pFSM->Change_State(To<_uint>(TITAN_STATE::CHASE));
    }
}

void CCrawlerTitan::OnChange_CurState(std::shared_ptr<CTitanState> spNewState)
{
    IF_NULL_RETURN_MSG_BREAK(spNewState, , "spNewState is nullptr");

    m_spCurState = spNewState;
    strncpy_s(m_szState, sizeof(m_szState), spNewState->Get_StateName(), _TRUNCATE);
}

NS_END;
