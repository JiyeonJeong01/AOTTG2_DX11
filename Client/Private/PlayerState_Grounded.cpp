#include "PlayerState_Grounded.h"

#include "Logger.h"
#include "GameObject.h"

CPlayerState_Grounded::CPlayerState_Grounded(Engine::CGameObject* goPlayer, CPlayer* scPlayer)
    : CPlayerState(goPlayer, scPlayer)
{
}

CPlayerState_Grounded::~CPlayerState_Grounded()
{
}

HRESULT CPlayerState_Grounded::Initialize()
{
    IF_NULL_RETURN_MSG_BREAK(m_goPlayer, E_FAIL, "m_goPlayer is nullptr");

    m_rbPlayer = m_goPlayer->Get_Component<CRigidbody>();

    return S_OK;
}

void CPlayerState_Grounded::Priority_Update(_float fDT)
{

}

void CPlayerState_Grounded::Update(_float fDT)
{
    Walk(fDT);
}

void CPlayerState_Grounded::Late_Update(_float fDT)
{
    CPlayerState::Late_Update(fDT);
}

void CPlayerState_Grounded::Decide_NextState()
{
    if (m_tInputCmd.bBoostPressed)
    {
        /* 점프 */
    }

    if (m_tInputCmd.bLeftAnchorPressed || m_tInputCmd.bLeftAnchorPressed)
    {
        /* 앵커 고정 가능한지 판단 */
        /* TODO : 이 판단은 꼭 Utils로 만들자! */
    }
}

void CPlayerState_Grounded::Enter()
{
    LOG_INFO("Enter CPlayerState_Grounded");
}

void CPlayerState_Grounded::Walk(_float fDT)
{
    _float3 vLinearVel = m_rbPlayer.Get_LinearVel();

    vLinearVel.x = m_tInputCmd.vMove.x * 10.f;
    vLinearVel.z = m_tInputCmd.vMove.z * 10.f;

    m_rbPlayer.Set_LinearVel(vLinearVel);
}

std::shared_ptr<CPlayerState_Grounded> CPlayerState_Grounded::Create(Engine::CGameObject* goPlayer, CPlayer* scPlayer)
{
    auto pInstance = std::make_shared<CPlayerState_Grounded>(goPlayer, scPlayer);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "pInstance create faild");
    return pInstance;
}
