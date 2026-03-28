#include "PlayerState.h"
#include "CameraController.h"

HRESULT CPlayerState::Initialize()
{
    return S_OK;
}

void CPlayerState::Priority_Update(_float fDT)
{
    Control_Camera();
}

void CPlayerState::Update(_float fDT)
{
}

void CPlayerState::Late_Update(_float fDT)
{
}

void CPlayerState::Control_Camera()
{
    if (m_tRef.pCameraController)
    {
        m_tRef.pCameraController->Yaw(m_tInputCmd.vMouseDelta.x);
        m_tRef.pCameraController->Pitch(m_tInputCmd.vMouseDelta.y);
    }
}

void CPlayerState::Update_PlayerInput(const PLAYER_INPUT_COMMAND& tInputCmd)
{
    m_tInputCmd = tInputCmd;
}

void CPlayerState::Cache_PlayerInfos(const PLAYER_COMPONENTS& tComponents, const PLAYER_RUNTIME_REF& tRef)
{
    m_tComponents = tComponents;
    m_tRef = tRef;
    m_pFSM = tRef.pFSM;
}
