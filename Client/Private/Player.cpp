#include "Player.h"

#include "GameObject_System.h"
#include "GameObject.h"

#include "PlayerState.h"
#include "PlayerStateMachine.h"
#include "Player_InputController.h"

#include "CameraController.h"


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


}

void CPlayer::Priority_Update(void* pCtx, _float fDT)
{
    const auto& tInput = m_upInputController->Update_InputCommand();
    m_upStateMachine->Update_PlayerInput(tInput);
    m_upStateMachine->Priority_Update(fDT);

    if (!m_pCameraController)
    {
        m_pCameraController = m_goPlayer->Get_Script<CCameraController>();
    }

    if (m_pCameraController)
    {
        m_pCameraController->Yaw(tInput.vMouseDelta.x);
        m_pCameraController->Pitch(tInput.vMouseDelta.y);
    }
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
