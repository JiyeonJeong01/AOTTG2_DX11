#pragma region HEADER
#include "Player.h"

#include "Player_InputController.h"
#include "Player_SkillController.h"

#include "PlayerStateMachine.h"
#include "PlayerState.h"

#include "CameraController.h"
#include "ODM_Gear.h"
#include "GroundChecker.h"
#include "HitBox.h"
#include "Trail.h"
#include "VFX_Manager.h"

#include "TargetSensor.h"
#include "AnimationClip_Player.h"

#include "UI_HitController.h"
#include "Attacher.h"
#pragma endregion

NS_BEGIN(Client)

CPlayer::CPlayer() {}
CPlayer::~CPlayer() {}

void CPlayer::Awake(void* pCtx)
{
    m_goPlayer = SYS_GAMEOBJECT.Get_Wrapper(m_hObject);

    m_upStateMachine = CPlayerStateMachine::Create(m_goPlayer, this);
    m_upInputController = CPlayer_InputController::Create();
    m_upSkillController = CPlayer_SkillController::Create(&m_tSkillSet);
    m_upTrail = CTrail::Create();

    m_upLeftBladeTrail = CTrail::Create();
    m_upRightBladeTrail = CTrail::Create();

    IF_NULL_RETURN_MSG_BREAK(m_upStateMachine, , "m_upStateMachine is nullptr");
    IF_NULL_RETURN_MSG_BREAK(m_upInputController, , "m_upInputController is nullptr");
    IF_NULL_RETURN_MSG_BREAK(m_upSkillController, , "m_upSkillController is nullptr");

    m_upStateMachine->Subscribe_OnChangedCurState(&CPlayer::OnChange_CurState, this);
}

void CPlayer::Start(void* pCtx)
{
    /* 오브젝트 참조 */
    Set_ReferenceObject();

    /* 컴포넌트 참조 */
    Set_ReferenceComponent();

    /* script 정보 참조 */
    Set_ReferenceScript();

    m_tRef.pSensor->Set_TargetMask(O_ENEMY);
    m_tRef.pSensor->Subscribe_OnDetectedTarget(&CPlayer::On_DetectedTitan, this);

    m_tRef.pCameraController->Bind_PlayerSensor(m_tRef.pSensor);

    /* 플레이어 스킬 정보 */
    m_upSkillController->SetUp_SkillSet();

    const auto& children = m_goPlayer->Get_Children();
    for (const auto& child : children)
    {
        if (!child) continue;
        const auto& label = child->Get_Label();
        if (label == "Blade_Left")
            m_tBlade.pLeftBlade = child;
        else if (label == "Blade_Right")
            m_tBlade.pRightBlade = child;
        else if (label == "Gas_Resupply")
            m_goGasResupply = child;

    }
    IF_NULL_RETURN_MSG_BREAK(m_tBlade.pLeftBlade, , "pLeftBlade is nullptr");
    IF_NULL_RETURN_MSG_BREAK(m_tBlade.pRightBlade, , "pRightBlade is nullptr");
    IF_NULL_RETURN_MSG_BREAK(m_goGasResupply, , "Gas_Resupply is nullptr");

    /* PlayerContext 구조체 값 넣기 */
    Build_Context();

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
    m_pGrabTitan = pTitan;
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

void CPlayer::On_BladeHit(CGameObject* goCounter, const HIT_INFO& tHitInfo)
{
    if (!goCounter)
        return;

    if (!goCounter->Has_Mask(O_ENEMY))
        return;

    if (!m_tBlade.Can_ConsumeBladeAtk())
        return;

    /* TODO : m_pUIHitControlelr 여기에 UI 연결하기 */

    m_tBlade.Consume_Blade();
    m_pUIHitController->On_PlayerHitTitan();

    SYS_SOUND.PlaySFX(L"Blade_Attack_Success", CHANNEL_21, 0.5f);
}

void CPlayer::On_DetectedTitan(CGameObject* goTitan)
{
    if (!m_tRef.pSensor)
        return;

    if (!goTitan)
        return;

    if (goTitan->Get_Mask() != O_ENEMY)
        return;

    auto Set_DetectedTitan = [this](CGameObject* pTitan) -> void
        {
            if (!pTitan)
                return;

            m_tRef.pSensor->Set_Target(pTitan);

            CTitan* scTitan = pTitan->Get_Script_InChildren<CTitan>();
            m_pCameraController->On_Change_DetectedTitan(pTitan, scTitan);
        };

    CGameObject* goPrevTarget = m_tRef.pSensor->Get_Target();
    if (!goPrevTarget)
    {
        /* 새 타겟 설정 */
        Set_DetectedTitan(goTitan);
        return;
    }

    if (goPrevTarget == goTitan)
        return;

    /* 더 가까운 녀석을 타겟으로 설정 */
    auto trPrev = goPrevTarget->Get_Component<CTransform>();
    auto trNew = goTitan->Get_Component<CTransform>();

    _vector vPrevPos = trPrev.Get_StateXM(STATE::POSITION);
    _vector vNewPos = trNew.Get_StateXM(STATE::POSITION);
    _vector vPlayerPos = m_tComponents.transform.Get_StateXM(STATE::POSITION);

    const _float fPrevDistSq = XMVectorGetX(XMVector3LengthSq(vPrevPos - vPlayerPos));
    const _float fNewDistSq = XMVectorGetX(XMVector3LengthSq(vNewPos - vPlayerPos));

    const _bool bNewIsCloser = (fNewDistSq < fPrevDistSq);
    if (!bNewIsCloser)
        return;

    Set_DetectedTitan(goTitan);
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

void CPlayer::Ready_Deliver_Supplies()
{
    Display_GasResupply(true);
}

void CPlayer::Complete_Deliver_Supplies()
{
    Display_GasResupply(false);
}

CTitan* CPlayer::Get_GrabbTitan() const
{
    return m_pGrabTitan;
}

CGameObject* CPlayer::Get_FadeUI() const
{
    CGameObject* goFadeUI = GAME_INSTANCE.Find_GameObject(m_refFadeUI.hObject);
    if (goFadeUI)
        return goFadeUI;

    return GAME_INSTANCE.Find_GameObject("FadeUI");
}

void CPlayer::Display_GasResupply(_bool bDisplay)
{
    if (!m_goGasResupply)
        return;

    auto mr = m_goGasResupply->Get_Component<CMeshRenderer>();
    if (!mr.Is_Valid())
        return;

    mr.Set_Enable(bDisplay);
}

void CPlayer::Set_ReferenceObject()
{
    CGameObject* goVFX = SYS_GAMEOBJECT.Get_Wrapper(m_refVFXManager.hObject);
    IF_NULL_RETURN_MSG_BREAK(goVFX, , "goVFX is nullptr");
    m_pVFX_Manager = goVFX->Get_Script<CVFX_Manager>();
    IF_NULL_RETURN_MSG_BREAK(m_pVFX_Manager, , "m_pVFX_Manager is nullptr");

    CGameObject* goUI = GAME_INSTANCE.Find_GameObject(m_refUIHit.hObject);
    IF_NULL_RETURN_MSG_BREAK(goUI, , "goUI is nullptr");
    m_pUIHitController = goUI->Get_Script<CUI_HitController>();
    IF_NULL_RETURN_MSG_BREAK(m_pUIHitController, , "m_pUIHitController is nullptr");
}

void CPlayer::Set_ReferenceComponent()
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

    m_tComponents.transform.Set_Position(XMLoadFloat3(&m_fStartPosition));
    m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::IDLE_CASUAL_M);
}

void CPlayer::Set_ReferenceScript()
{
    m_pGear = m_tRef.pGear = m_goPlayer->Get_Script_InChildren<CODM_Gear>();
    m_tRef.pGroundChecker = m_goPlayer->Get_Script_InChildren<CGroundChecker>();
    m_tRef.pFSM = m_upStateMachine.get();
    m_pCameraController = m_tRef.pCameraController = m_goPlayer->Get_Script<CCameraController>();
    m_tRef.pSensor = m_goPlayer->Get_Script_InChildren<CTargetSensor>();
    m_tRef.pTrail = m_upTrail.get();

    CGameObject* pLeftBladeAttacher = nullptr;
    CGameObject* pRightBladeAttacher = nullptr;
    auto attachers = m_goPlayer->Get_AllScripts<CAttacher>();

    for (auto attacher : attachers)
    {
        auto* goObj = attacher->Get_AttachObject();
        if (!goObj) continue;
        if (goObj->Get_Label() == "LeftTrail")
            pLeftBladeAttacher = goObj;
        else if (goObj->Get_Label() == "RightTrail")
            pRightBladeAttacher = goObj;
    }

    if (pLeftBladeAttacher)
    {
        m_tRef.tLeftBladeTrail = { pLeftBladeAttacher->Get_Component<CTransform>(), m_upLeftBladeTrail.get() };
        m_upLeftBladeTrail->Set_Color({ 1.f, 0.f, 0.f, 1.f });
    }
    if (pRightBladeAttacher)
    {
        m_tRef.tRightBladeTrail = { pRightBladeAttacher->Get_Component<CTransform>(), m_upRightBladeTrail.get() };
        m_upRightBladeTrail->Set_Color({ 1.f, 0.f, 0.f, 1.f });
    }
    m_tRef.pAllHitBoxes = &m_AllHitBoxes;
}

void CPlayer::Build_Context()
{
    /* 플레이어 상태에게 전달 */
    m_tContext.tComponents = m_tComponents;
    m_tContext.tRef = m_tRef;
    m_tContext.pStats = &m_tStats;
    m_tContext.pBlade = &m_tBlade;
    m_tContext.pHitBox = m_goPlayer->Get_Script_InChildren<CHitBox>();
    m_tContext.pVFX_Manager = m_pVFX_Manager;

    /* 컨트롤러 */
    m_tContext.pSkillController = m_upSkillController.get();
    m_tContext.pOriginDrag = &m_fForceDrag;
}

NS_END;
