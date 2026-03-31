#include "Player.h"
#include "Input_System.h"

#include "Player_InputController.h"
#include "Player_SkillController.h"

#include "PlayerStateMachine.h"
#include "PlayerState.h"

#include "CameraController.h"
#include "ODM_Gear.h"
#include "GroundChecker.h"


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

        m_tComponents.springJoint = m_goPlayer->Get_Component<CSpringJoint>();
        IF_TRUE_RETURN_MSG_BREAK(!m_tComponents.springJoint.Is_Valid(), , "springJoint is invalid");

        m_tComponents.meshRenderer = m_goPlayer->Get_Component<CMeshRenderer>();
        IF_TRUE_RETURN_MSG_BREAK(!m_tComponents.meshRenderer.Is_Valid(), , "meshRenderer is invalid");
    }

    /* 런타임 정보 참조 */
    {
        m_tRef.pGear = m_goPlayer->Get_Script_InChildren<CODM_Gear>();
        m_tRef.pGroundChecker = m_goPlayer->Get_Script_InChildren<CGroundChecker>();
        m_tRef.pFSM = m_upStateMachine.get();
        m_tRef.pCameraController = m_goPlayer->Get_Script<CCameraController>();
    }

    /* 플레이어 스킬 정보 */
    {
        m_upSkillController->SetUp_SkillSet();
    }

    /* 플레이어 상태에게 전달 */
    PLAYER_CONTEXT tContext;
    tContext.tComponents = m_tComponents;
    tContext.tRef = m_tRef;
    tContext.pStats = &m_tStats;

    /* 컨트롤러 */
    tContext.pSkillController = m_upSkillController.get();

    m_upStateMachine->Cache_PlayerInfos(tContext);

    /* 이벤트 등록 */
    //m_tComponents.collider->OnCollisionEnter.Add_Listener(&CPlayer::On_CollisionEnter, this);
    //m_tComponents.collider->OnCollisionStay.Add_Listener(&CPlayer::On_CollisionStay, this);
    //m_tComponents.collider->OnCollisionExit.Add_Listener(&CPlayer::On_CollisionExit, this);
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

void CPlayer::OnChange_CurState(std::shared_ptr<CPlayerState> spNewState)
{
    IF_NULL_RETURN_MSG_BREAK(spNewState, , "spNewState is nullptr");

    m_spCurState = spNewState;
    strncpy_s(m_szState, sizeof(m_szState), spNewState->Get_StateName(), _TRUNCATE);
}

void CPlayer::On_CollisionEnter(const COLLISION_DESC& tDesc)
{
    CGameObject* pObj = GAME_INSTANCE.Find_GameObject(tDesc.hObject);
    if (pObj)
        LOG_INFO("================================================== COLLISION_ENTER ====================================== ");
}

void CPlayer::On_CollisionStay(const COLLISION_DESC& tDesc)
{
    CGameObject* pObj = GAME_INSTANCE.Find_GameObject(tDesc.hObject);
    if (pObj)
        LOG_INFO("================================================== COLLISION_STAY ====================================== ");
}

void CPlayer::On_CollisionExit(const COLLISION_DESC& tDesc)
{
    CGameObject* pObj = GAME_INSTANCE.Find_GameObject(tDesc.hObject);
    if (pObj)
        LOG_INFO("================================================== COLLISION_EXIT ====================================== ");
}

PLAYER_CONTEXT CPlayer::Get_PlayerContext()
{
    /* 플레이어 상태에게 전달 */
    PLAYER_CONTEXT tContext;
    tContext.tComponents = m_tComponents;
    tContext.tRef = m_tRef;
    tContext.pStats = &m_tStats;

    /* 컨트롤러 */
    tContext.pSkillController = m_upSkillController.get();

    return tContext;
}

NS_END;
