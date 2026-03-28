#include "Player.h"

#include "GameObject_System.h"
#include "GameObject.h"
#include "Input_System.h"

#include "PlayerState.h"
#include "PlayerStateMachine.h"
#include "Player_InputController.h"

#include "CameraController.h"
#include "ODM_Gear.h"


NS_BEGIN(Client)

CPlayer::CPlayer() {}

CPlayer::~CPlayer() {}

void CPlayer::Awake(void* pCtx)
{
    m_goPlayer = SYS_GAMEOBJECT.Get_Wrapper(m_hObject);

    m_upStateMachine = CPlayerStateMachine::Create(m_goPlayer, this);
    m_upInputController = CPlayer_InputController::Create();
}

void CPlayer::Start(void* pCtx)
{
    /* 컴포넌트 참조 */
    {
        m_tComponents.transform = m_goPlayer->Get_Component<CTransform>();
        IF_TRUE_RETURN_MSG_BREAK(!m_tComponents.transform.Is_Valid(), , "transform is invalid");

        m_tComponents.animator = m_goPlayer->Get_Component<CAnimator>();
        IF_TRUE_RETURN_MSG_BREAK(!m_tComponents.animator.Is_Valid(), , "animator is invalid");

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
        m_tRef.pGroundChecker = nullptr; /* TODO */
        m_tRef.pFSM = m_upStateMachine.get();
    }

    /* 플레이어 상태에게 전달 */
    m_upStateMachine->Cache_PlayerInfos(m_tComponents, m_tRef);
}

void CPlayer::Priority_Update(void* pCtx, _float fDT)
{
    const auto& tInput = m_upInputController->Update_InputCommand();
    m_upStateMachine->Update_PlayerInput(tInput);
    m_upStateMachine->Priority_Update(fDT);
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
}

NS_END;
