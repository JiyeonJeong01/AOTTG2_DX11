#include "Player.h"

#include "Player_InputController.h"
#include "Player_SkillController.h"

#include "PlayerStateMachine.h"
#include "PlayerState.h"

#include "CameraController.h"
#include "ODM_Gear.h"
#include "GroundChecker.h"
#include "HitBox.h"

#include "AnimationClip_Player.h"

NS_BEGIN(Client)

CPlayer::CPlayer() {}

CPlayer::~CPlayer() {}

void CPlayer::Awake(void* pCtx)
{
    m_goPlayer = SYS_GAMEOBJECT.Get_Wrapper(m_hObject);

    m_upStateMachine = CPlayerStateMachine::Create(m_goPlayer, this);
    m_upInputController = CPlayer_InputController::Create();
    m_upSkillController = CPlayer_SkillController::Create(&m_tSkillSet);

    IF_NULL_RETURN_MSG_BREAK(m_upStateMachine, , "m_upStateMachine is nullptr");
    IF_NULL_RETURN_MSG_BREAK(m_upInputController, , "m_upInputController is nullptr");
    IF_NULL_RETURN_MSG_BREAK(m_upSkillController, , "m_upSkillController is nullptr");

    m_upStateMachine->Subscribe_OnChangedCurState(&CPlayer::OnChange_CurState, this);
}

void CPlayer::Start(void* pCtx)
{
    /* 컴포넌트 참조 */
    {
        m_tComponents.transform = m_goPlayer->Get_Component<CTransform>();
        IF_TRUE_RETURN_MSG_BREAK(!m_tComponents.transform.Is_Valid(), , "transform is invalid");

        m_tComponents.animator = m_goPlayer->Get_Component<CAnimator>();
        IF_TRUE_RETURN_MSG_BREAK(!m_tComponents.animator.Is_Valid(), , "animator is invalid");

        m_tComponents.collider = m_goPlayer->Get_Component<CCollider>();
        IF_TRUE_RETURN_MSG_BREAK(!m_tComponents.collider.Is_Valid(), , "collider is invalid");

        m_tComponents.rigidbody = m_goPlayer->Get_Component<CRigidbody>();
        IF_TRUE_RETURN_MSG_BREAK(!m_tComponents.rigidbody.Is_Valid(), , "rigidbody is invalid");

        m_tComponents.rigidbody->bDebugLog = true;

        m_tComponents.springJoint = m_goPlayer->Get_Component<CSpringJoint>();
        IF_TRUE_RETURN_MSG_BREAK(!m_tComponents.springJoint.Is_Valid(), , "springJoint is invalid");

        m_tComponents.meshRenderer = m_goPlayer->Get_Component<CMeshRenderer>();
        IF_TRUE_RETURN_MSG_BREAK(!m_tComponents.meshRenderer.Is_Valid(), , "meshRenderer is invalid");
    }

    /* 런타임 정보 참조 */
    {
        m_pGear = m_tRef.pGear = m_goPlayer->Get_Script_InChildren<CODM_Gear>();
        m_tRef.pGroundChecker = m_goPlayer->Get_Script_InChildren<CGroundChecker>();
        m_tRef.pFSM = m_upStateMachine.get();
        m_tRef.pCameraController = m_goPlayer->Get_Script<CCameraController>();
        m_tRef.pAllHitBoxes = &m_AllHitBoxes;
    }

    /* 플레이어 스킬 정보 */
    {
        m_upSkillController->SetUp_SkillSet();
    }

    const auto& children = m_goPlayer->Get_Children();
    for (const auto& child : children)
    {
        if (!child) continue;
        if (child->Get_Label() == "Blade_Left")
            m_tBlade.pLeftBlade = child;
        else if (child->Get_Label() == "Blade_Right")
            m_tBlade.pRightBlade = child;
    }
    IF_NULL_RETURN_MSG_BREAK(m_tBlade.pLeftBlade, , "pLeftBlade is nullptr");
    IF_NULL_RETURN_MSG_BREAK(m_tBlade.pRightBlade, , "pRightBlade is nullptr");

    /* 플레이어 상태에게 전달 */
    m_tContext.tComponents = m_tComponents;
    m_tContext.tRef = m_tRef;
    m_tContext.pStats = &m_tStats;
    m_tContext.pBlade = &m_tBlade;
    m_tContext.pHitBox = m_goPlayer->Get_Script_InChildren<CHitBox>();

    /* 컨트롤러 */
    m_tContext.pSkillController = m_upSkillController.get();

    m_upStateMachine->Cache_PlayerInfos(m_tContext);
    m_tRef.pGear->Bind_PlayerContext(m_tContext);

    auto allHitBoxes = m_goPlayer->Get_AllScripts_InChildren<CHitBox>();

    for (auto& hit : allHitBoxes)
    {
        CGameObject* goHitBox = hit->Get_HitBoxObject();
        IF_NULL_RETURN_MSG_BREAK(goHitBox, , "goHitBox is nullptr");

        auto [iter, bInserted] = m_AllHitBoxes.emplace(string(goHitBox->Get_Label()), hit);
        IF_TRUE_RETURN_MSG_BREAK(!bInserted, , "duplicated hitbox label");

        hit->Subscribe_OnSuccessHit(&CPlayer::On_BladeHit, this);
    }

    /* 히트박스 전부 끄기 */
    for (auto& hit : m_AllHitBoxes)
        hit.second->Set_Active(false);
}

void CPlayer::Priority_Update(void* pCtx, _float fDT)
{
    const auto& tInput = m_upInputController->Update_InputCommand();
    m_upStateMachine->Update_PlayerInput(tInput);
    m_upStateMachine->Priority_Update(fDT);

    m_upSkillController->Update_SkillSet(fDT, tInput);
}

void CPlayer::Update(void* pCtx, _float fDT)
{
    m_upStateMachine->Update(fDT);
}

void CPlayer::Late_Update(void* pCtx, _float fDT)
{
    m_upStateMachine->Late_Update(fDT);
}

void CPlayer::On_Grabbed(SIDE eSide, CTitan* pTitan)
{
    PLAYER_STATE eState = m_spCurState->Get_State();
    if (eState == PLAYER_STATE::GRABBED)
        return;

    GRABBED eGrabbed = GRABBED::END;
    if (eSide == SIDE::LEFT)
        eGrabbed = GRABBED::LEFT;
    else if (eSide == SIDE::RIGHT)
        eGrabbed = GRABBED::RIGHT;

    m_upStateMachine->Change_State(To<_uint>(PLAYER_STATE::GRABBED), To<_uint>(eGrabbed));
}

void CPlayer::On_Dead()
{
    CHuman::On_Dead();
}

void CPlayer::OnChange_CurState(std::shared_ptr<CPlayerState> spNewState)
{
    IF_NULL_RETURN_MSG_BREAK(spNewState, , "spNewState is nullptr");

    m_spCurState = spNewState;
    strncpy_s(m_szState, sizeof(m_szState), spNewState->Get_StateName(), _TRUNCATE);
}

void CPlayer::On_BladeHit(CGameObject* goCounter)
{
    if (!goCounter)
        return;

    if (!goCounter->Has_Mask(O_ENEMY))
        return;

    if (!m_tBlade.Can_ConsumeBladeAtk())
        return;

    m_tBlade.Consume_Blade();
}

PLAYER_CONTEXT CPlayer::Get_PlayerContext()
{
    /* 플레이어 상태에게 전달 */
    return m_tContext;
}

void CPlayer::Resupply()
{
    if (!m_upStateMachine)
        return;

    m_upStateMachine->Change_State(To<_uint>(PLAYER_STATE::RESUPPLY));
}

void CPlayer::Deliver_Supplies()
{
    m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::RESUPPLY);
}

NS_END;
