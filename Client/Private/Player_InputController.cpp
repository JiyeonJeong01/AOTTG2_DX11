#include "Player_InputController.h"
#include "Input_System.h"
#include "Engine_Math.h"

CPlayer_InputController::CPlayer_InputController()
{
}

CPlayer_InputController::~CPlayer_InputController()
{
}

const PLAYER_INPUT_COMMAND& CPlayer_InputController::Update_InputCommand()
{
    /* 사용자의 입력을 받아 구조체를 채운다 */
    m_tInputCommand = {};

    long iMove = 0;
    if (iMove = SYS_INPUT.Get_DIMouseMove(MOUSE_MOVE_AXIS::HORIZONTAL))
        m_tInputCommand.vMouseDelta.x = To<_float>(iMove);
    if (iMove = SYS_INPUT.Get_DIMouseMove(MOUSE_MOVE_AXIS::VERTICAL))
        m_tInputCommand.vMouseDelta.y = To<_float>(iMove);

    _float3 vMove{};
    if (SYS_INPUT.Get_Key('W'))
        vMove.z = 1.f;
    else if (SYS_INPUT.Get_Key('S'))
        vMove.z = -1.f;
    if (SYS_INPUT.Get_Key('A'))
        vMove.x = -1.f;
    if (SYS_INPUT.Get_Key('D'))
        vMove.x = 1.f;

    const _vector vMoveVec = XMLoadFloat3(&vMove);
    if (!XMVector3Equal(vMoveVec, XMVectorZero()))
        XMStoreFloat3(&m_tInputCommand.vMove, XMVector3Normalize(vMoveVec));
    else
        m_tInputCommand.vMove = _float3(0.f, 0.f, 0.f);

    if (SYS_INPUT.Get_KeyDown('Q'))
        m_tInputCommand.bLeftAnchorPressed = true;
    else if (SYS_INPUT.Get_Key('Q'))
        m_tInputCommand.bLeftAnchorHeld = true;
    if (SYS_INPUT.Get_KeyDown('E'))
        m_tInputCommand.bRightAnchorPressed = true;
    else if (SYS_INPUT.Get_Key('E'))
        m_tInputCommand.bRightAnchorHeld = true;

    if (SYS_INPUT.Get_KeyDown(VK_SHIFT))
        m_tInputCommand.bBoostPressed = true;
    if (SYS_INPUT.Get_Key(VK_SHIFT))
        m_tInputCommand.bBoostHeld = true;

    if (SYS_INPUT.Get_KeyDown(VK_LBUTTON))
        m_tInputCommand.bNormalAttackPressed = true;
    if (SYS_INPUT.Get_KeyDown(VK_RBUTTON))
        m_tInputCommand.bStrongAttackPressed = true;

    return m_tInputCommand;
}

std::unique_ptr<CPlayer_InputController> CPlayer_InputController::Create()
{
    return std::make_unique<CPlayer_InputController>();
}
